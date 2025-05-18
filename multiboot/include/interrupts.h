#ifndef INT_H
#define INT_H

#include <stdint-gcc.h>
#include <pic.h>

#define IDT_NUM_ENTRIES     256
#define IDT_TRAP_GATE       0xF
#define IDT_INTR_GATE       0xE
#define IDT_NUM_EXCEPTIONS  0x20

#define EXC_DE          0
#define EXC_DF          8
#define EXC_TS          10
#define EXC_NP          11
#define EXC_SS          12
#define EXC_GP          13
#define EXC_PF          14
#define EXC_AC          17

#define IST_CUR         0
#define IST_DF          1
#define IST_PF          2
#define IST_GP          3

#define STACK_SIZE      4096

#define IRQ_KEYBOARD    (M_PIC_OFFSET + PIC_KEYBOARD_IRQ_NUM)
#define IRQ_SERIAL      (M_PIC_OFFSET + PIC_SERIAL_IRQ_NUM)

#define INT_N(n)        __asm__ volatile ("int %0" : : "i" (n))
#define STI()           __asm__ volatile ("sti")
#define CLI()           __asm__ volatile ("cli")
#define HLT()           __asm__ volatile ("hlt")

typedef struct {
    uint16_t offset0;
    uint16_t sel;
    
    uint16_t ist        : 3;
    uint16_t res0       : 5;
    uint16_t type       : 4;
    uint16_t res1       : 1;
    uint16_t dpl        : 2;
    uint16_t present    : 1;
    uint16_t offset1;

    uint32_t offset2;
    uint32_t res2;
} __attribute__((packed)) idt_entry_t;


typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) descriptor_ptr_t;

typedef struct {
    uint32_t res0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t res1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t res2;
    uint16_t res3;
    uint16_t iobp;
} __attribute__((packed)) tss_t;

// irq num, err code, args
typedef void (*irq_handler_t)(int, int, void*);
typedef struct {
    void *args;
    irq_handler_t handler;
} irq_table_entry_t;

// irq_stubs.asm
extern void (*irq_stub_array[IDT_NUM_ENTRIES])(void);

extern uint16_t kernel_code_selector;
extern uint16_t tss_selector;

extern tss_t tss;
extern intptr_t critical_stacks[4];

void irq_handler(int n, int e);
void setup_idt();
void register_irq(int n, void* args, irq_handler_t irq_handler);
void TSS_setup();
void IRQ_setup();

#endif // INT_H