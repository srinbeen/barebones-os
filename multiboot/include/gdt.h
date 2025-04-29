#ifndef GDT_H
#define GDT_H

#include <stdint-gcc.h>

typedef struct {
    uint16_t limit0;
    uint16_t base0;

    uint8_t base1;

    uint8_t type    : 4;
    uint8_t res0    : 1;
    uint8_t dpl     : 2;
    uint8_t p       : 1;

    uint8_t limit1  : 4;
    uint8_t avl     : 1;
    uint8_t res1    : 2;
    uint8_t g       : 1;

    uint8_t base2;
    uint32_t base3;
    uint32_t res2;
} __attribute__((packed)) tss_descriptor_t;

typedef struct {
    uint64_t null;
    uint64_t code_descriptor;
    tss_descriptor_t tss_descriptor;

} __attribute__((packed)) gdt_t;

extern gdt_t gdt64;

void GDT_setup();

#endif // GDT_H