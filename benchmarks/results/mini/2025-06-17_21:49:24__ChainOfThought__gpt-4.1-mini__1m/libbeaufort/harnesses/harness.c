#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <beaufort.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) {
        // Nothing to do on empty input
        return 0;
    }

    // Split input into two parts: plaintext and key
    size_t half = size / 2;

    // If half is zero, assign 1 to key length if possible
    if (half == 0) {
        half = 1;
    }

    size_t plen = half;
    size_t klen = size - plen;

    // Extract plaintext string (not null-terminated)
    char *plaintext = (char *)malloc(plen + 1);
    if (!plaintext) {
        return 0;
    }
    memcpy(plaintext, data, plen);
    plaintext[plen] = '\0';

    // Extract key string (not null-terminated)
    char *key = (char *)malloc(klen + 1);
    if (!key) {
        free(plaintext);
        return 0;
    }
    memcpy(key, data + plen, klen);
    key[klen] = '\0';

    // Skip if key length is zero to avoid division by zero crashes
    if (klen == 0) {
        free(plaintext);
        free(key);
        return 0;
    }

    // Encrypt
    char *enc = beaufort_encrypt(plaintext, key, NULL);
    if (enc) {
        // Decrypt
        char *dec = beaufort_decrypt(enc, key, NULL);
        if (dec) {
            // We can optionally check if decrypted equals plaintext here,
            // but don't abort on mismatch since not all inputs are valid.
            free(dec);
        }
        free(enc);
    }

    free(plaintext);
    free(key);

    return 0;
}