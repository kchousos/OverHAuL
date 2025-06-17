#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "snappy.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // snappy environment
    struct snappy_env env;
    int ret = snappy_init_env(&env);
    if (ret != 0) {
        // Allocation failure, skip this input
        return 0;
    }

    // For empty input, compress and decompress anyway
    size_t max_compressed_len = snappy_max_compressed_length(Size);
    char *compressed = (char*)malloc(max_compressed_len);
    if (!compressed) {
        snappy_free_env(&env);
        return 0;
    }

    size_t compressed_length = 0;
    ret = snappy_compress(&env, (const char*)Data, Size, compressed, &compressed_length);
    if (ret != 0) {
        // Compression error, free and exit
        free(compressed);
        snappy_free_env(&env);
        return 0;
    }

    // Allocate buffer for decompression
    char *decompressed = (char*)malloc(Size ? Size : 1);
    if (!decompressed) {
        free(compressed);
        snappy_free_env(&env);
        return 0;
    }

    ret = snappy_uncompress(compressed, compressed_length, decompressed);
    if (ret != 0) {
        // Decompression error - crash the program deliberately
        // by writing to null or assert to help fuzzing find bugs
        *((volatile int*)0) = 0;
    }

    // Verify decompressed output matches original input for Size > 0
    if (Size > 0 && memcmp(decompressed, Data, Size) != 0) {
        // Mismatch - crash deliberately
        *((volatile int*)0) = 0;
    }

    free(decompressed);
    free(compressed);
    snappy_free_env(&env);

    return 0;
}