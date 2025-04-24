#ifndef IDT_H
#define IDT_H

#include <stdint-gcc.h>

#define IDT_NUM_ENTRIES     256
#define IDT_TRAP_GATE       0xF
#define IDT_INTR_GATE       0xE
#define IDT_NUM_EXCEPTIONS  0x20

#define EXC_DE        0
#define EXC_DF        8
#define EXC_TS        10
#define EXC_NP        11
#define EXC_SS        12
#define EXC_GP        13
#define EXC_PF        14
#define EXC_AC        17

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
} __attribute__((packed)) idt_ptr_t;

// irq num, err code, args
typedef void (*irq_handler_t)(int, int, void*);

// irq_stubs.asm
extern void (*irq_stub_array[IDT_NUM_ENTRIES])(void);
extern uint16_t kernel_code_selector;

void irq_handler(int n, int e, void* a);
void setup_idt();

#endif // IDT_H