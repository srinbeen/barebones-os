#include <dumbio.h>
#include <vga.h>
#include <serial.h>
#include <stdarg.h>

static void print_char(char);
static void print_str(const char *);
static void print_num(unsigned long long num, uint8_t flags);

inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0"
        : "=a"(ret)
        : "Nd"(port)
    );
    return ret;
}

// %% %d %u %x %c %p %h[dux] %l[dux] %q[dux] %s
int printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    const char *p = fmt;
    while (*p != '\0') {
        if (*p == '%' && *(++p) != '\0') {
            switch (*p) {
                // %
                case '%': {
                    print_char('%');
                    break;
                }
                // signed int
                case 'd': {
                    int num = va_arg(args, int);
                    print_num((long long)num, SIGNED_MASK);
                    break;
                }
                // unsigned int
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    print_num((unsigned long long)num, UNSIGNED_MASK);
                    break;
                }
                // hex int
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    print_num((unsigned long long)num, HEX_MASK);
                    break;
                }
                // char
                case 'c': {
                    // type promotion from shorter types than int to int itself
                    char c = va_arg(args, int);
                    print_char(c);
                    break;
                }
                // pointer
                case 'p': {
                    // type promotion from shorter types than int to int itself
                    intptr_t p = va_arg(args, intptr_t);
                    print_num(p, PTR_MASK | HEX_MASK);
                    break;
                }
                // short
                case 'h': {
                    // type promotion from shorter types than int to int itself
                    unsigned short num = va_arg(args, int);

                    switch (*(++p)) {
                        case 'd': {
                            print_num((long long)(short)num, SIGNED_MASK);
                            break;
                        }
                        case 'u': {
                            print_num((unsigned long long)num, UNSIGNED_MASK);
                            break;
                        }
                        case 'x': {
                            print_num((unsigned long long)num, HEX_MASK);
                            break;
                        }
                    }
                    break;
                }
                // long
                case 'l': {
                    unsigned long num = va_arg(args, long);

                    switch (*(++p)) {
                        case 'd': {
                            print_num((long long)(long)num, SIGNED_MASK);
                            break;
                        }
                        case 'u': {
                            print_num((unsigned long long)num, UNSIGNED_MASK);
                            break;
                        }
                        case 'x': {
                            print_num((unsigned long long)num, HEX_MASK);
                            break;
                        }
                    }
                    break;
                }
                // long long
                case 'q': {
                    long long num = va_arg(args, long long);

                    switch (*(++p)) {
                        case 'd': {
                            print_num(num, SIGNED_MASK);
                            break;
                        }
                        case 'u': {
                            print_num(num, UNSIGNED_MASK);
                            break;
                        }
                        case 'x': {
                            print_num(num, HEX_MASK);
                            break;
                        }
                    }
                    break;
                }
                // string
                case 's': {
                    char *s = va_arg(args, char *);
                    print_str(s);
                    break;
                }
                default: {
                    print_char('%');
                    print_char(*p);
                    break;
                }
            }
        } else {
            print_char(*p);
        }
        ++p;
    }

    va_end(args);

    return 0;
}

static void print_char(char ch) {
    VGA_display_char(ch);
    SER_write(ch);
}
static void print_str(const char * str) {
    uint32_t idx = 0;
    char ch = str[idx];
    while (ch != '\0') {
        print_char(ch);
        idx++;
        ch = str[idx];
    }
}
static void print_num(unsigned long long num, uint8_t flags) {
    char buf[MAX_LEN_ULONGLONG_STR+1];
    buf[MAX_LEN_ULONGLONG_STR] = '\0';

    uint8_t isHex = (flags & HEX_MASK) > 0;
    uint8_t isSigned = (flags & SIGNED_MASK) > 0;
    uint8_t isPtr = (flags & PTR_MASK) > 0;

    unsigned long long base = isHex ? 16 : 10;
    char hexCase = isPtr ? 'A' : 'a';
    int i;

    if (isPtr) {
        print_str("0x");
    }
    if (num == 0) {
        print_char('0');
        return;
    }
    else if (!isHex && isSigned && (long long)num < 0) {
        print_char('-');
        num = (long long)num * -1;
    }
    for (i = sizeof(buf)-2; i > -1; i--) {
        unsigned long long rem = num % base;
        buf[i] = isHex && rem > 9 ? hexCase + (rem-10) : '0' + rem;
        
        num /= base;
        if (num == 0) {
            break;
        }
    }
    print_str(buf + i);
}