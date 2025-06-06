#ifndef PAGEFRAME_H
#define PAGEFRAME_H

#include <multiboot.h>

#define PAGE_SIZE       4096
#define NULL_PAGE       (void*)1

typedef struct {
    multiboot_mem_info_entry_t* entry;
    uintptr_t addr;
} new_pf_t;

typedef struct {
    uintptr_t start;
    uintptr_t end;
} range_t;

typedef struct s_node_t {
    struct s_node_t* next;
    struct s_node_t* prev;
} node_t;

uintptr_t page_align_up(uintptr_t addr);
uintptr_t page_align_down(uintptr_t addr);

void  pf_alloc_init(multiboot_fixed_header_t* mb_header);
void* pf_alloc();
void  pf_free(void* pf);

int update_running_pf();
int is_pf_valid(void* addr);


#endif // PAGEFRAME_H