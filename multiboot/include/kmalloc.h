#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdint-gcc.h>
#include <pageframe.h>

#define NUM_POOLS       7
#define MAX_POOL_SIZE   2048

typedef struct s_free_chunk_header_t {
    struct s_free_chunk_header_t* next;
} kfree_chunk_header_t;

typedef struct {
    uint64_t size;
    uint64_t num_free;
    kfree_chunk_header_t* next;
} kmalloc_pool_header_t;

typedef struct {
   kmalloc_pool_header_t* pool;
   uint64_t size;
} kmalloc_chunk_header_t;

void    kmalloc_init();
void*   kmalloc(uint64_t size);
void    kfree(void* addr);

extern kmalloc_pool_header_t pool_list[NUM_POOLS];


#endif //KMALLOC_H