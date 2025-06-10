#include <pagetable.h>
#include <pageframe.h>
#include <interrupts.h>

#include <dumbio.h>
#include <dumbstring.h>

#include <interrupts.h>

PML4_entry_t* kernel_page_table;
uintptr_t cur_kheap_vaddr;
valloc_state_t state;

void setup_kernel_page_table() {
    cur_kheap_vaddr = KHEAP_START;
    CR3(kernel_page_table);
    kernel_page_table = (PML4_entry_t*)((uintptr_t)kernel_page_table & (~0xFFF));

    setup_identity_mapping();

    state.cur_page_table = kernel_page_table;
    register_irq(EXC_PF, &state, pagefault_handler);
}


void setup_identity_mapping() {
    PDP_entry_t* pdpt = (PDP_entry_t*)(uintptr_t)(kernel_page_table[0].base << 12);
    for (int i = 0; i < PT_NUM_ENTRIES; i++) {
        pdpt[i].p = 1;
        pdpt[i].us = 0;
        pdpt[i].rw = 1;
        pdpt[i].ps = 1;
        pdpt[i].base = ((uint64_t)i << 18);
        // printk("entry %d: %lx\n", i, (uintptr_t)(pdpt[i].base << PT_OFFSET));
    }
}

void* MMU_alloc_page() {
    void* vpage = (void*)cur_kheap_vaddr;
    if (cur_kheap_vaddr >= KHEAP_START + KHEAP_START) {
        printk("RUN OUT OF HEAP SPACE\n");
        HLT();
    }
    vp_alloc_page(kernel_page_table, cur_kheap_vaddr, 1);

    cur_kheap_vaddr += PAGE_SIZE;
    return vpage;
}

void* MMU_alloc_pages(int num) {
    void* vpage = (void*)cur_kheap_vaddr;
    for (int i = 0; i < num; i++) {
        if (cur_kheap_vaddr >= KHEAP_START + KHEAP_START) {
            printk("RUN OUT OF HEAP SPACE\n");
            HLT();
        }
        vp_alloc_page(kernel_page_table, cur_kheap_vaddr, 1);
        
        cur_kheap_vaddr += PAGE_SIZE;
    }
    
    return vpage;
}

void MMU_free_page(void* vaddr) {
    uintptr_t vaddrnum = (uintptr_t)vaddr;
    if (vaddrnum >= KHEAP_START + KHEAP_START) {
        printk("INVALID FREE PTR\n");
        HLT();
    }
    vp_free_page(kernel_page_table, vaddrnum);

    return;
}

void MMU_free_pages(void* vaddr, int num) {
    uintptr_t vaddrnum = (uintptr_t)vaddr;
    for (int i = 0; i < num; i++) {
        if (vaddrnum >= KHEAP_START + KHEAP_START) {
            printk("INVALID FREE PTR\n");
            HLT();
        }
        vp_free_page(kernel_page_table, vaddrnum);
        vaddrnum += PAGE_SIZE;
    }

    return;
}

void vp_free_page(PML4_entry_t* page_table, uintptr_t vaddr) {
    PDP_entry_t*    pdpt_base = get_PDPT_base(page_table, vaddr, 0);
    if (pdpt_base == NULL_PAGE) {
        printk("FREE: INVALID PDPT\n");
        HLT();
    }

    PD_entry_t*     pdt_base = get_PDT_base(pdpt_base, vaddr, 0);
    if (pdt_base == NULL_PAGE) {
        printk("FREE: INVALID PDT\n");
        HLT();
    }
    
    PT_entry_t*     pt_base = get_PT_base(pdt_base, vaddr, 0);
    if (pt_base == NULL_PAGE) {
        printk("FREE: INVALID PT\n");
        HLT();
    }

    uint16_t idx = (vaddr & PT_BIT_MASK) >> PT_BIT_POS;
    if (pt_base[idx].p == 1) {
        pf_free((void*)((uintptr_t)pt_base[idx].base << PT_OFFSET));

        pt_base[idx].p = 0;
        pt_base[idx].avl = 0;
        INVLPG(vaddr);
    }
    else if (pt_base[idx].avl == 1) {
        pt_base[idx].avl = 0;
    }
    else {
        printk("FREE: INVALID FREE PTR\n");
        HLT();
    }
}

void vp_alloc_page(PML4_entry_t* page_table, uintptr_t vaddr, bool demand) {
    PDP_entry_t*    pdpt_base = get_PDPT_base(page_table, vaddr, 1);
    PD_entry_t*     pdt_base = get_PDT_base(pdpt_base, vaddr, 1);
    PT_entry_t*     pt_base = get_PT_base(pdt_base, vaddr, 1);

    uint16_t idx = (vaddr & PT_BIT_MASK) >> PT_BIT_POS;
    if (demand) {
        pt_base[idx].p = 0;
        pt_base[idx].us = 0;
        pt_base[idx].rw = 1;
        pt_base[idx].avl = 1;
    }
    else {
        void* pf = pf_alloc();
        memset(pf, 0, PAGE_SIZE);

        pt_base[idx].p = 1;
        pt_base[idx].avl = 0;
        pt_base[idx].us = 0;
        pt_base[idx].rw = 1;
        pt_base[idx].base = (uintptr_t)pf >> PT_OFFSET;
    }
}


PDP_entry_t* get_PDPT_base(PML4_entry_t* page_table, uintptr_t vaddr, bool create) {
    uint16_t idx = (vaddr & PML4_BIT_MASK) >> PML4_BIT_POS;
    
    PDP_entry_t* pdpt_base;
    if (page_table[idx].p == 0) {
        if (create) {
            pdpt_base = pf_alloc();
            memset(pdpt_base, 0, PAGE_SIZE);
            if (pdpt_base == NULL_PAGE) {
                printk("get_PDPT_base: PF_ALLOC FAILED --> %p\n", (void*)vaddr);
                HLT();
            }
            
            
            page_table[idx].p = 1;
            page_table[idx].us = 0;
            page_table[idx].rw = 1;
            page_table[idx].base = (uintptr_t)pdpt_base >> PT_OFFSET;
        }
        else {
            pdpt_base = NULL_PAGE;
        }
    }
    else {
        pdpt_base = (PDP_entry_t*)((uintptr_t)page_table[idx].base << PT_OFFSET);
    }
    
    return pdpt_base;
}

PD_entry_t* get_PDT_base(PDP_entry_t* pdpt_base, uintptr_t vaddr, bool create) {
    uint16_t idx = (vaddr & PDP_BIT_MASK) >> PDP_BIT_POS;

    PD_entry_t* pdt_base;
    if (pdpt_base[idx].p == 0) {
        if (create) {
            pdt_base = pf_alloc();
            memset(pdt_base, 0, PAGE_SIZE);
            if (pdt_base == NULL_PAGE) {
                printk("get_PDT_base: PF_ALLOC FAILED --> %p\n", (void*)vaddr);
                HLT();
            }
            
            pdpt_base[idx].p = 1;
            pdpt_base[idx].us = 0;
            pdpt_base[idx].rw = 1;
            pdpt_base[idx].base = (uintptr_t)pdt_base >> PT_OFFSET;
        }
        else {
            pdt_base = NULL_PAGE;
        }
    }
    else {
        pdt_base = (PD_entry_t*)((uintptr_t)pdpt_base[idx].base << PT_OFFSET);
    }

    return pdt_base;
}

PT_entry_t* get_PT_base(PD_entry_t* pdt_base, uintptr_t vaddr, bool create) {
    uint16_t idx = (vaddr & PD_BIT_MASK) >> PD_BIT_POS;

    PT_entry_t* pt_base;
    if (pdt_base[idx].p == 0) {
        if (create) {
            pt_base = pf_alloc();
            memset(pt_base, 0, PAGE_SIZE);
            if (pt_base == NULL_PAGE) {
                printk("get_PT_base: PF_ALLOC FAILED --> %p\n", (void*)vaddr);
                HLT();
            }
            
            pdt_base[idx].p = 1;
            pdt_base[idx].us = 0;
            pdt_base[idx].rw = 1;
            pdt_base[idx].base = (uintptr_t)pt_base >> PT_OFFSET;
        }
        else {
            pt_base = NULL_PAGE;
        }
    }
    else {
        pt_base = (PT_entry_t*)((uintptr_t)pdt_base[idx].base << PT_OFFSET);
    }

    return pt_base;
}

void pagefault_handler(int n, int e, void* args) {
    valloc_state_t* state = (valloc_state_t*)args;
    uintptr_t vaddr;
    CR2(vaddr);
    vaddr = page_align_down(vaddr);

    PDP_entry_t*    pdpt_base = get_PDPT_base(state->cur_page_table, vaddr, 0);
    if (pdpt_base == NULL_PAGE) {
        printk("PF_HANDLER: INVALID PDPT --> %p\n", (void*)vaddr);
        HLT();
    }

    PD_entry_t*     pdt_base = get_PDT_base(pdpt_base, vaddr, 0);
    if (pdt_base == NULL_PAGE) {
        printk("PF_HANDLER: INVALID PDT --> %p\n", (void*)vaddr);
        HLT();
    }
    
    PT_entry_t*     pt_base = get_PT_base(pdt_base, vaddr, 0);
    if (pt_base == NULL_PAGE) {
        printk("PF_HANDLER: INVALID PT --> %p\n", (void*)vaddr);
        HLT();
    }

    uint16_t idx = (vaddr & PT_BIT_MASK) >> PT_BIT_POS;
    if (pt_base[idx].avl == 1) {
        void* pf = pf_alloc();
        if (pf == NULL_PAGE) {
            printk("PF_HANDLER: PF_ALLOC FAILED --> %p\n", (void*)vaddr);
            HLT();
        }
        memset(pf, 0, PAGE_SIZE);

        pt_base[idx].p = 1;
        pt_base[idx].avl = 0;
        pt_base[idx].base = (uintptr_t)pf >> PT_OFFSET;
    }
    else {
        printk("PF_HANDLER: INVALID PTR --> %p\n", (void*)vaddr);
        HLT();
    }
}