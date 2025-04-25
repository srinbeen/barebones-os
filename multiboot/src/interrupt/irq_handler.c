#include <idt.h>
#include <dumbio.h>
#include <keyboard.h>

static idt_entry_t idt[IDT_NUM_ENTRIES];
static idt_ptr_t idt_ptr;

void irq_handler(int n, int e, void* a) {
    // printk("triggered IRQ: %d\n", n);
    switch (n) {
        case EXC_DE:
            printk("divide by zero\n");
            break;
        case EXC_TS:
            printk("invalid tss\n");
            break;
        case IRQ_KEYBOARD:
            PS2_process_keyboard();
            PIC_sendEOI(PIC_KEYBOARD_IRQ_NUM);
            break;
        case 100:
            printk("testing IRQ100\n");
            break;
        default:
            printk("ts is NOT tuff\n");
            __asm__ volatile ("hlt");
            break;
    }
}

void setup_idt() {
    for (int i = 0; i < IDT_NUM_ENTRIES; i++) {
        intptr_t stub_addr = (intptr_t)irq_stub_array[i];
        idt[i].offset0 = (uint16_t)(stub_addr & 0xFFFF);
        idt[i].sel = kernel_code_selector;
        idt[i].ist = 0;
        idt[i].type = i < IDT_NUM_EXCEPTIONS ? 0xF : 0xE;
        idt[i].dpl = 0;
        idt[i].present = 1;
        idt[i].offset1 = (uint16_t)((stub_addr >> 16) & 0xFFFF);
        idt[i].offset2 = (uint16_t)((stub_addr >> 32) & 0xFFFF);
    }

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (intptr_t)idt;

    __asm__ volatile ("lidt %0" :: "m"(idt_ptr));
}