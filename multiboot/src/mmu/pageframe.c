#include <pageframe.h>
#include <dumbio.h>
#include <interrupts.h>

static multiboot_mmap_tag_t* mmap_header;
static multiboot_elf_symbols_tag_t* elf_header;
static char* string_table_addr;

static new_pf_t new_pf;

static range_t mb_tags;

node_t* free_list;

static int mmap_num_entries;

inline uintptr_t page_align_up(uintptr_t addr) {
    return (addr + (PAGE_SIZE - 1)) & ~((uintptr_t)PAGE_SIZE - 1);
}
inline uintptr_t page_align_down(uintptr_t addr) {
    return addr & ~((uintptr_t)PAGE_SIZE - 1);
}

void pf_alloc_init(multiboot_fixed_header_t* mb_header) {
    mb_tags.start = page_align_down((uintptr_t)mb_header);
    mb_tags.end = page_align_up((uintptr_t)mb_header + mb_header->size);

    multiboot_tag_header_t* var_tag_header = mb_header->tags;
    for (int i = 0; var_tag_header->type != MB_NULL_TAG && var_tag_header->size != MB_NULL_TAG_SIZE; i++) {
        switch (var_tag_header->type)
        {
            case MB_MMAP_TAG:
                mmap_header = (multiboot_mmap_tag_t*)var_tag_header;
                break;
            case MB_ELF_SYM_TAG:
            elf_header = (multiboot_elf_symbols_tag_t*)var_tag_header;
            string_table_addr = (char*)(elf_header->section_headers[elf_header->string_table_index].section_address);
                break;
            default:
                break;
        }

        var_tag_header = (multiboot_tag_header_t*)(
            ((uintptr_t)var_tag_header + var_tag_header->size + (MB_TAG_ALIGN-1)) & ~((uintptr_t)MB_TAG_ALIGN-1)
        );
    }

    free_list = NULL_PAGE;

    mmap_num_entries = ((uintptr_t)mmap_header + mmap_header->header.size - (uintptr_t)(mmap_header->entries)) / mmap_header->entry_size;
    new_pf.entry = mmap_header->entries;
    
    while (new_pf.entry->type != MB_MMAP_RAM) {
        if (new_pf.entry == mmap_header->entries + (mmap_num_entries-1)) {
            printk("RUN OUT OF MEMORY\n");
            HLT();
        }
        new_pf.entry++;
    }

    new_pf.addr = page_align_up(new_pf.entry->start);
}

void* pf_alloc() {
    if (free_list != NULL_PAGE) {
        node_t* req = free_list;
        free_list = free_list->next;
        free_list->prev = NULL_PAGE;
        return req;
    }

    if (update_running_pf()) {
        return NULL_PAGE;
    }

    if ((mb_tags.start >= new_pf.addr && mb_tags.start < new_pf.addr + PAGE_SIZE) || 
        (mb_tags.start <= new_pf.addr && mb_tags.end > new_pf.addr)) {

        // printk("hit conflict with %s: [%p-%p) --> %p\n", 
        //     "multiboot tags", 
        //     (void*)mb_tags.start, 
        //     (void*)mb_tags.end, 
        //     (void*)new_pf.addr
        // );

        new_pf.addr = mb_tags.end;
        if (update_running_pf()) {
            return NULL_PAGE;
        }
    }

    for (int64_t j = 0; j < (int64_t)(elf_header->num_sections); j++) {
        multiboot_section_header_t* section_header = elf_header->section_headers + j;

        if (section_header->flags & SHF_ALLOC) {
            range_t section_pf;

            section_pf.start = page_align_down(section_header->section_address);
            section_pf.end = page_align_up(section_header->section_address + section_header->section_size);
            
            if ((section_pf.start >= new_pf.addr && section_pf.start < new_pf.addr + PAGE_SIZE) || 
                (section_pf.start <= new_pf.addr && section_pf.end > new_pf.addr)) {

                // printk("hit conflict with %s: [%p-%p) --> %p\n", 
                //     string_table_addr + section_header->string_table_index, 
                //     (void*)section_header->section_address, 
                //     (void*)(section_header->section_address + section_header->section_size), 
                //     (void*)new_pf.addr
                // );

                new_pf.addr = section_pf.end;
                if (update_running_pf()) {
                    return NULL_PAGE;
                }

                j = -1;
            }
        }
    }

    void* req = (void*)new_pf.addr;
    new_pf.addr += PAGE_SIZE;
    return req;
}

void pf_free(void* pf) {
    node_t* free_node = (node_t*)pf;
    free_node->prev = NULL_PAGE;
    if (free_list != NULL_PAGE) {
        free_list->prev = free_node;
    }
    free_node->next = free_list;
    free_list = free_node;
}

int update_running_pf() {
    if (new_pf.addr >= new_pf.entry->start + new_pf.entry->length) {
        do {
            if (new_pf.entry == mmap_header->entries + (mmap_num_entries-1)) {
                printk("RUN OUT OF MEMORY\n");
                return 1;
            }
            new_pf.entry++;
        } while (new_pf.entry->type != MB_MMAP_RAM);

        new_pf.addr = page_align_up(new_pf.entry->start);
    }

    return 0;
}


int is_pf_valid(void* addr) {
    uintptr_t addr_num = (uintptr_t)addr;

    if (addr_num % PAGE_SIZE != 0) {
        return 0;
    }

    if ((mb_tags.start >= addr_num && mb_tags.start < addr_num + PAGE_SIZE) || 
        (mb_tags.start <= addr_num && mb_tags.end > addr_num)) {

        return 0;
    }

    for (int i = 0; i < mmap_num_entries; i++) {
        if (mmap_header->entries[i].type != MB_MMAP_RAM) {
            continue;
        }

        if (addr_num < mmap_header->entries[i].start ||
            addr_num >= mmap_header->entries[i].start + mmap_header->entries[i].length) {
            continue;
        }

        for (int64_t j = 0; j < (int64_t)(elf_header->num_sections); j++) {
            multiboot_section_header_t* section_header = elf_header->section_headers + j;
            if (!(section_header->flags & SHF_ALLOC)) {
                continue;
            }

            uintptr_t section_pf_start = page_align_down(section_header->section_address);
            uintptr_t section_pf_end = page_align_up(section_header->section_address + section_header->section_size);
            
            if ((section_pf_start >= addr_num && section_pf_start < addr_num + PAGE_SIZE) || 
                (section_pf_start <= addr_num && section_pf_end > addr_num)) {

                return 0;
            }
        }

        return 1;
    }

    return 0;
} 