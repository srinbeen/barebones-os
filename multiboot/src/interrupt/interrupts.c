#include <dumbio.h>

#include <interrupts.h>
#include <keyboard.h>
#include <serial.h>

#include <gdt.h>
#include <stddef.h>

static idt_entry_t idt[IDT_NUM_ENTRIES] = {0};
static irq_table_entry_t irq_table[IDT_NUM_ENTRIES] = {0};

void irq_handler(int n, int e) {
    if (n < 0 && n >= IDT_NUM_ENTRIES) {
        printk("irq number out of bounds: %d\n", n);
        HLT();
    }
    else if (irq_table[n].handler == NULL) {
        printk("unhandled interrupt: %d\n", n);
        HLT();
    }
    
    irq_table[n].handler(irq_table[n].args);
}

void setup_idt() {
    descriptor_ptr_t idt_ptr;

    for (int i = 0; i < IDT_NUM_ENTRIES; i++) {
        intptr_t stub_addr = (intptr_t)irq_stub_array[i];
        idt[i].offset0 = (uint16_t)(stub_addr & 0xFFFF);
        idt[i].sel = kernel_code_selector;
        switch (i) {
            case EXC_DF:
                idt[i].ist = IST_DF;
                break;
            case EXC_PF:
                idt[i].ist = IST_PF;
                break;
            case EXC_GP:
                idt[i].ist = IST_GP;
                break;
            default:
                idt[i].ist = IST_CUR;
                break;

        }
        idt[i].type = IDT_INTR_GATE;
        idt[i].dpl = 0;
        idt[i].present = 1;
        idt[i].offset1 = (uint16_t)((stub_addr >> 16) & 0xFFFF);
        idt[i].offset2 = (uint16_t)((stub_addr >> 32) & 0xFFFFFFFF);
    }

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (intptr_t)idt;

    __asm__ volatile ("lidt %0" :: "m"(idt_ptr));
}

void register_irq(int n, void* args, irq_handler_t irq_handler) {
    irq_table[n].args = args;
    irq_table[n].handler = irq_handler;
}

void IRQ_setup() {
    CLI();
    TSS_setup();
    PIC_remap(M_PIC_OFFSET, S_PIC_OFFSET);
    PS2_setup();
    SER_init();
    setup_idt();
    STI();
}

void TSS_setup() {
    gdt64.tss_descriptor.limit0 = sizeof(tss_t) & 0xFFFF;
    gdt64.tss_descriptor.base0 = (intptr_t)&tss & 0xFFFF;
    gdt64.tss_descriptor.base1 = ((intptr_t)&tss >> 16) & 0xFF;
    gdt64.tss_descriptor.type = 0b1001;
    gdt64.tss_descriptor.dpl = 0;
    gdt64.tss_descriptor.p = 1;
    gdt64.tss_descriptor.limit1 = (sizeof(tss_t) >> 16) & 0xF;
    gdt64.tss_descriptor.base2 = ((intptr_t)&tss >> 24) & 0xFF;
    gdt64.tss_descriptor.base3 = ((intptr_t)&tss >> 32) & 0xFFFFFFFF;

    tss.rsp0 = critical_stacks[IST_CUR];
    tss.ist1 = critical_stacks[IST_DF];
    tss.ist2 = critical_stacks[IST_PF];
    tss.ist3 = critical_stacks[IST_GP];

    __asm__ volatile ("ltr %0" : : "m" (tss_selector));
}