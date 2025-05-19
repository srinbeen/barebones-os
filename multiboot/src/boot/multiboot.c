#include <multiboot.h>
#include <dumbio.h>

static char* string_table_addr = (char*)0;

void multiboot_parse_tags(multiboot_fixed_header_t* mb_header) {

    printk("header_ptr: %p\n",mb_header);
    if ((intptr_t)mb_header % MB_TAG_ALIGN != 0) {
        printk("shit aint aligned chief\n");
        return;
    }
    printk("fixed header:\n\tsize: %d\n\tres: %d\n", mb_header->size, mb_header->res);
    
    multiboot_tag_header_t* var_tag_header = (multiboot_tag_header_t*)((intptr_t)mb_header + sizeof(multiboot_fixed_header_t));
    for (int i = 0; var_tag_header->type != MB_NULL_TAG && var_tag_header->size != MB_NULL_TAG_SIZE; i++) {
        switch (var_tag_header->type)
        {
            case MB_NULL_TAG:
                printk("NULL TAG: shouldn't be here!\n");
                return;
            case MB_MEM_TAG:
                mulitboot_parse_basic_mem_tag((multiboot_basic_mem_tag_t*)var_tag_header);
                break;
            case MB_BIOS_DEV_TAG:
                printk("BIOSDEV TAG!\n");
                break;
                case MB_BOOT_CMD_TAG:
                printk("BOOTCMD TAG!\n");
                break;
                case MB_BOOTLOADER_TAG:
                printk("BOOTLOADER TAG!\n");
                break;
                case MB_MMAP_TAG:
                mulitboot_parse_mmap_tag((multiboot_mmap_tag_t*)var_tag_header);
                break;
            case MB_ELF_SYM_TAG:
                mulitboot_parse_elf_sym_tag((multiboot_elf_symbols_tag_t*)var_tag_header);
                break;
        
            default:
                printk("tag %d:\n\ttype: %d\n\tsize: %d\n", i, var_tag_header->type, var_tag_header->size);
                break;
        }

        intptr_t new_tag_addr = (intptr_t)var_tag_header + var_tag_header->size;
        int offset = new_tag_addr % MB_TAG_ALIGN;
        if (offset != 0) {
            new_tag_addr += (MB_TAG_ALIGN-offset);
        }
        var_tag_header = (multiboot_tag_header_t*)new_tag_addr;
    }

    printk("size check: expected: %d, actual: %d\n", mb_header->size, (uint32_t)((intptr_t)var_tag_header - (intptr_t)mb_header) + MB_NULL_TAG_SIZE);
    return;
}

void mulitboot_parse_basic_mem_tag(multiboot_basic_mem_tag_t* tag_header) {
    printk("mem_tag:\n\tmem_lower: %d\n\tmem_higher: %d\n", tag_header->mem_lower, tag_header->mem_higher);
}

void mulitboot_parse_mmap_tag(multiboot_mmap_tag_t* tag_header) {
    int num_entries = ((intptr_t)tag_header + tag_header->header.size - (intptr_t)(tag_header->entries)) / tag_header->entry_size;
    multiboot_mem_info_entry_t* entry = tag_header->entries;
    for (int i = 0; i < num_entries; i++) {
        if (entry->type == MB_MMAP_RAM) {
            printk("mmap entry:\n\tstarting: 0x%lx\n\tlength: %ld\n", entry->start, entry->length);
        }

        entry++;
    }
}

void mulitboot_parse_elf_sym_tag(multiboot_elf_symbols_tag_t* tag_header) {
    multiboot_section_header_t* section_header = tag_header->section_headers;
    string_table_addr = (char*)(tag_header->section_headers[tag_header->string_table_index].section_address);

    for (int i = 0; i < tag_header->num_sections; i++) {
        printk("%s:\n\ttype: 0x%x\n\tflag: 0x%lx\n\taddr: 0x%lx\n\tsize: %ld\n\tdisk_offset: %ld\n", 
            string_table_addr + section_header->string_table_index, 
            section_header->type,
            section_header->flags,
            section_header->section_address,
            section_header->section_size,
            section_header->section_disk_offset
        );

        section_header++;
    }
}