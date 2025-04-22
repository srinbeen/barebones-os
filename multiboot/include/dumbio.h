#ifndef DUMBIO_H
#define DUMBIO_H

#include <stdint-gcc.h>

#define MAX_LEN_ULONGLONG_STR 20
#define SIGNED_MASK     1 << 0
#define UNSIGNED_MASK   0 << 0
#define HEX_MASK        1 << 1
#define PTR_MASK        1 << 2

// %% %d %u %x %c %p %h[dux] %l[dux] %q[dux] %s
int printk(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

// static void print_char(char);
// static void print_str(const char *);
// static void print_num(unsigned long long num, uint8_t flags);

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

#endif  // DUMBIO_H