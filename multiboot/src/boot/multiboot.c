#include <multiboot.h>
#include <dumbio.h>

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
    int num_entries = (tag_header->header.size - sizeof(multiboot_mmap_tag_t)) / tag_header->entry_size;

    printk("mmap_tag:\n\tmem info entry size: %d\n\tnum_entries: %d\n", tag_header->entry_size, num_entries);
    multiboot_mem_info_entry_t* entry = (multiboot_mem_info_entry_t*)((intptr_t)tag_header + sizeof(multiboot_mmap_tag_t));

    for (int i = 0; i < num_entries; i++) {
        if (entry->type == 1) {
            printk("ENTRY USABLE:\n\tstarting: 0x%lx\n\tlength: %ld\n", entry->start, entry->length);
        }
        else {
            printk("ENTRY NOT USABLE\n");
        }

        entry++;
    }
}

void mulitboot_parse_elf_sym_tag(multiboot_elf_symbols_tag_t* tag_header) {
    printk("ELF!!\n");
}