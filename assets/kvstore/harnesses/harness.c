#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "kvstore.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    kvstore_init();

    // We will split the input data into multiple commands separated by '\n'
    // Each command is null-terminated for kvstore_handle_command.
    // To avoid excessive allocations, we copy data into a buffer and replace '\n' with '\0'.

    if (size == 0) {
        kvstore_cleanup();
        return 0;
    }

    // Allocate buffer and copy data
    char *buf = malloc(size + 1);
    if (!buf) {
        kvstore_cleanup();
        return 0;
    }
    memcpy(buf, data, size);
    buf[size] = '\0';

    char *start = buf;
    for (size_t i = 0; i <= size; i++) {
        if (buf[i] == '\n' || buf[i] == '\0') {
            buf[i] = '\0';
            if (start[0] != '\0') {
                kvstore_handle_command(start);
            }
            start = buf + i + 1;
        }
    }

    free(buf);
    kvstore_cleanup();
    return 0;
}