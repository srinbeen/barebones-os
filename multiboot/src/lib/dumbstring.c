#include <dumbstring.h>

void *memset(void *dst, int c, size_t n) {
    char* dst_chp = (char*)dst;

    for (int i = 0; i < n; i++) {
        dst_chp[i] = c;
    }

    return dst;
}
void *memcpy(void *dst, const void *src, size_t n) {
    char* dst_chp = (char*)dst;
    char* src_chp = (char*)src;

    for (int i = 0; i < n; i++) {
        dst_chp[i] = src_chp[i];
    }

    return dst;
}