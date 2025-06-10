#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include <stdint-gcc.h>
#include <stdbool.h>

#define CR3(x)              __asm__ volatile("mov %%cr3, %0" : "=r"(x) : : "memory")
#define CR2(x)              __asm__ volatile("mov %%cr2, %0" : "=r"(x) : : "memory")
#define INVLPG(x)           __asm__ volatile ("invlpg (%0)" : : "r" ((void*)x))


#define PT_NUM_ENTRIES      512
#define PML4_IDENTITY_ENTRY 0
#define PML4_KHEAP_ENTRY    1
#define PML4_KSTACKS_ENTRY  15

#define PML4_BIT_MASK       0x0000FF8000000000
#define PDP_BIT_MASK        0x0000007FC0000000
#define PD_BIT_MASK         0x000000003FE00000
#define PT_BIT_MASK         0x00000000001FF000

#define PML4_BIT_POS        39
#define PDP_BIT_POS         30
#define PD_BIT_POS          21
#define PT_BIT_POS          12
#define PT_OFFSET           12

#define KHEAP_START         0x8000000000

typedef struct {
    uint64_t res0 : 12;
    uint64_t base : 40;
    uint64_t res1 : 12;
} CR3_t;

typedef struct {
    uint64_t p : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t ign : 1;
    uint64_t z : 2;
    uint64_t avl : 3;
    uint64_t base : 40;
    uint64_t avl1 : 11;
    uint64_t nx : 1;
} PML4_entry_t;

typedef struct {
    uint64_t p : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t d : 1;
    uint64_t ps : 1;
    uint64_t z1 : 1;
    uint64_t avl : 3;
    uint64_t base : 40;
    uint64_t avl1 : 11;
    uint64_t nx : 1;
} PDP_entry_t;

typedef struct {
    uint64_t p : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t ign0 : 1;
    uint64_t z : 1;
    uint64_t ign1 : 1;
    uint64_t avl : 3;
    uint64_t base : 40;
    uint64_t avl1 : 11;
    uint64_t nx : 1;
} PD_entry_t;

typedef struct {
    uint64_t p : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t d : 1;
    uint64_t pat : 1;
    uint64_t g : 1;
    uint64_t avl : 3;
    uint64_t base : 40;
    uint64_t avl1 : 11;
    uint64_t nx : 1;
} PT_entry_t;

typedef struct {
    PML4_entry_t* cur_page_table;
} valloc_state_t;

void setup_kernel_page_table();
void setup_identity_mapping();

void vp_alloc_page(PML4_entry_t* page_table, uintptr_t vaddr, bool demand);
void vp_free_page(PML4_entry_t* page_table, uintptr_t vaddr);

void* MMU_alloc_page();
void* MMU_alloc_pages(int num);
void MMU_free_page(void*);
void MMU_free_pages(void*, int num);

PDP_entry_t* get_PDPT_base(PML4_entry_t* page_table, uintptr_t vaddr, bool create);
PD_entry_t* get_PDT_base(PDP_entry_t* pdpt_base, uintptr_t vaddr, bool create);
PT_entry_t* get_PT_base(PD_entry_t* pdt_base, uintptr_t vaddr, bool create);

void pagefault_handler(int n, int e, void* args);

#endif // PAGE_TABLE_H