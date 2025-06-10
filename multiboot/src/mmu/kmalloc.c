#include <kmalloc.h>
#include <pagetable.h>
#include <dumbio.h>

kmalloc_pool_header_t pool_list[NUM_POOLS];

void kmalloc_init() {
    for (int i = 0; i < NUM_POOLS; i++) {
        pool_list[i].next = NULL_PAGE;
        pool_list[i].num_free = 0;
        pool_list[i].size = 1 << (5 + i);
    }
}

void* kmalloc(uint64_t size) {
    uint64_t meta_size = size + sizeof(kmalloc_chunk_header_t);
    printk("meta: %ld\n", meta_size);

    if (meta_size > MAX_POOL_SIZE) {
        int num_pages = page_align_up(meta_size) / PAGE_SIZE;
        kmalloc_chunk_header_t* new_chunk = (kmalloc_chunk_header_t*)MMU_alloc_pages(num_pages);
        new_chunk->pool = NULL_PAGE;
        new_chunk->size = meta_size;
        return (void*)((uintptr_t)new_chunk + sizeof(kmalloc_chunk_header_t));
    }

    for (int i = 0; i < NUM_POOLS; i++) {
        if (meta_size <= pool_list[i].size) {
            printk("size: %ld\n", pool_list[i].size);

            if (pool_list[i].num_free == 0) {
                kfree_chunk_header_t* last_header = (kfree_chunk_header_t*)MMU_alloc_page();
                pool_list[i].next = last_header;
                
                int num_free_chunks = PAGE_SIZE / pool_list[i].size;

                for (int j = 1; j < num_free_chunks; j++) {
                    kfree_chunk_header_t* cur_header = (kfree_chunk_header_t*)((uintptr_t)last_header + pool_list[i].size);
                    last_header->next = cur_header;

                    last_header = cur_header;
                }

                pool_list[i].num_free = num_free_chunks;
            }

            kfree_chunk_header_t* free_chunk = pool_list[i].next;
            pool_list[i].next = free_chunk->next;
            pool_list[i].num_free--;

            kmalloc_chunk_header_t* new_chunk = (kmalloc_chunk_header_t*)free_chunk;
            new_chunk->pool = (pool_list + i);
            new_chunk->size = meta_size;
            return (void*)((uintptr_t)new_chunk + sizeof(kmalloc_chunk_header_t));
        }
    }

    return NULL_PAGE;
}

void kfree(void* addr) {
    kmalloc_chunk_header_t* chunk_header = (kmalloc_chunk_header_t*)((uintptr_t)addr - sizeof(kmalloc_chunk_header_t));
    
    if (chunk_header->pool == NULL_PAGE) {
        int num_pages = page_align_up(chunk_header->size) / PAGE_SIZE;
        MMU_free_pages(chunk_header, num_pages);
        return;
    }

    
    kmalloc_pool_header_t* pool = chunk_header->pool;

    kfree_chunk_header_t* free_chunk = (kfree_chunk_header_t*)chunk_header;
    if (pool->num_free != 0) {
        kfree_chunk_header_t* next_free = pool->next;
        free_chunk->next = next_free;
    }
    
    pool->next = free_chunk;
    pool->num_free++;
}