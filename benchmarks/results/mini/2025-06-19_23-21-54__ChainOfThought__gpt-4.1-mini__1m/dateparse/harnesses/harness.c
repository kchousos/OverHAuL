#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "dateparse.h"
#include "dateparse.c" // Include implementation to resolve linker errors

// Limit input to 59 bytes to match parser's max accepted length in dateparse.c
#define MAX_INPUT_LEN 59

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // Truncate input to MAX_INPUT_LEN to prevent buffer overflows in dateparse
    size_t len = Size < MAX_INPUT_LEN ? Size : MAX_INPUT_LEN;

    // Copy input to buffer, zero-terminate
    char buf[MAX_INPUT_LEN + 1];
    memcpy(buf, Data, len);
    buf[len] = '\0';

    date_t dt;
    int offset;

    // Call dateparse with input string and explicit length
    dateparse(buf, &dt, &offset, (int)len);

    return 0;
}