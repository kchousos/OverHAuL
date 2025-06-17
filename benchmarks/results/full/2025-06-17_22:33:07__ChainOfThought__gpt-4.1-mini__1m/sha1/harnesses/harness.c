#include <stdint.h>
#include <stddef.h>
#include "sha1.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    SHA1_CTX ctx;
    unsigned char digest[20];

    SHA1Init(&ctx);
    SHA1Update(&ctx, data, (uint32_t)size);
    SHA1Final(digest, &ctx);

    // Optionally consume digest to avoid compiler warnings
    volatile unsigned char sink = 0;
    for (int i = 0; i < 20; i++) {
        sink ^= digest[i];
    }

    return 0;
}