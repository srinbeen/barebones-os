#include <idt.h>
#include <vga.h>

static idt_entry_t idt[IDT_NUM_ENTRIES];
static idt_ptr_t idt_ptr;

void irq_handler(int n, int e, void* a) {
    switch (n) {
        case EXC_DE:
            VGA_display_str("please for the love of god work\n");
            break;
        default:
            VGA_display_str("ts is NOT tuff\n");
            break;
    }

    __asm__ volatile ("hlt");
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