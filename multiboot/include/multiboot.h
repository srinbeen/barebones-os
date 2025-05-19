#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint-gcc.h>

#define MB_NULL_TAG         0
#define MB_MEM_TAG          4
#define MB_BIOS_DEV_TAG     5
#define MB_BOOT_CMD_TAG     1
#define MB_BOOTLOADER_TAG   2
#define MB_MMAP_TAG         6
#define MB_ELF_SYM_TAG      9

#define MB_MMAP_RAM         1   

#define MB_TAG_ALIGN        8
#define MB_NULL_TAG_SIZE    8

#define SHT_STRTAB          0x3


typedef struct {
    uint32_t size;
    uint32_t res;
} __attribute__((packed)) multiboot_fixed_header_t;

typedef struct {
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) multiboot_tag_header_t;

typedef struct {
    multiboot_tag_header_t header;
    uint32_t mem_lower;
    uint32_t mem_higher;
} __attribute__((packed)) multiboot_basic_mem_tag_t;

typedef struct {
    multiboot_tag_header_t header;
    uint32_t biosdev;
    uint32_t part;
    uint32_t subpart;
} __attribute__((packed)) multiboot_BIOS_device_tag_t;

typedef struct {
    multiboot_tag_header_t header;
    char cmd[];
} __attribute__((packed)) multiboot_boot_cmd_tag_t;

typedef struct {
    multiboot_tag_header_t header;
    char name[];
} __attribute__((packed)) multiboot_bootloader_tag_t;

typedef struct {
    uint64_t start;
    uint64_t length;
    uint32_t type;
    uint32_t res;
} __attribute__((packed)) multiboot_mem_info_entry_t;

typedef struct {
    multiboot_tag_header_t header;
    uint32_t entry_size;
    uint32_t entry_version;
    multiboot_mem_info_entry_t entries[];
} __attribute__((packed)) multiboot_mmap_tag_t;

typedef struct {
    uint32_t string_table_index;
    uint32_t type;
    uint64_t flags;
    uint64_t section_address;
    uint64_t section_disk_offset;
    uint64_t section_size;
    uint32_t table_index_link;
    uint32_t extra;
    uint64_t alignment;
    uint64_t fixed_entry_size;
} __attribute__((packed)) multiboot_section_header_t;

typedef struct {
    multiboot_tag_header_t header;
    uint32_t num_sections;
    uint32_t section_header_size;
    uint32_t string_table_index;
    multiboot_section_header_t section_headers[];
} __attribute__((packed)) multiboot_elf_symbols_tag_t;

void multiboot_parse_tags(multiboot_fixed_header_t* mb_header);
void mulitboot_parse_basic_mem_tag(multiboot_basic_mem_tag_t* tag_header);
void mulitboot_parse_mmap_tag(multiboot_mmap_tag_t* tag_header);
void mulitboot_parse_elf_sym_tag(multiboot_elf_symbols_tag_t* tag_header);


#endif // MULTIBOOT_H