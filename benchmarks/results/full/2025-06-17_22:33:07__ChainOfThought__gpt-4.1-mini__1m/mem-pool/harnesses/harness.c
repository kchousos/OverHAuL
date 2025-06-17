#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "internals.h"
#include "../include/mem_pool.h"

// To avoid too large memory consumption and unreasonable allocations
#define MAX_ALLOC_SIZE 1024
#define MAX_BLOCKS 1024

// Provide a dummy FixedPoolForeach callback to satisfy the API signature if needed
static MemPoolForeachStatus dummy_foreach_cb(void *block) {
    (void)block;
    return MEM_POOL_FOREACH_CONTINUE;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // We fuzz only the variable pool here
    VariableMemPool *pool = NULL;
    size_t grow_size = 4096; // reasonable grow size
    uint16_t tolerance_percent = 20;

    // Initialize pool
    if (pool_variable_init(&pool, grow_size, tolerance_percent) != MEM_POOL_ERR_OK) {
        return 0; // Initialization failed, just bail out
    }

    // Prepare array to hold allocated pointers and their assigned sizes
    void *ptrs[MAX_BLOCKS];
    size_t ptr_sizes[MAX_BLOCKS];
    size_t allocated_count = 0;

    // Iterate over input data: interpret as a sequence of 2-byte little-endian allocation sizes
    size_t pos = 0;
    while (pos + 2 <= size && allocated_count < MAX_BLOCKS) {
        uint16_t requested_size = data[pos] | (data[pos+1] << 8);
        pos += 2;

        // Limit requested size to maximum
        if (requested_size == 0) requested_size = 1;
        if (requested_size > MAX_ALLOC_SIZE) requested_size = requested_size % MAX_ALLOC_SIZE + 1;

        void *ptr = NULL;
        MemPoolError err = pool_variable_alloc(pool, requested_size, &ptr);
        if (err == MEM_POOL_ERR_OK && ptr != NULL) {
            // Store pointer and requested size
            ptrs[allocated_count] = ptr;
            ptr_sizes[allocated_count] = requested_size;
            allocated_count++;
        }
        // else allocation failed, ignore and continue
    }

    // Now free all allocated blocks in order
    for (size_t i = 0; i < allocated_count; ++i) {
        pool_variable_free(pool, ptrs[i]);
    }

    // Destroy pool and free all buffers
    pool_variable_destroy(pool);

    return 0;
}