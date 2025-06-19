#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "is-email.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Allocate buffer with extra space for null terminator
    char *buf = (char *)malloc(size + 1);
    if (!buf) return 0;

    // Copy input data and append null terminator
    memcpy(buf, data, size);
    buf[size] = '\0';

    // Call the function under test
    (void)is_email(buf);

    free(buf);
    return 0;
}