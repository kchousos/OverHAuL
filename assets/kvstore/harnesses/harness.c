#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "kvstore.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    kvstore_init();

    // We will split the input data into multiple commands separated by '\n'
    // Each command is null-terminated and passed to kvstore_handle_command.
    // To avoid excessive memory usage, limit number of commands.
    const size_t max_commands = 20;
    size_t commands_count = 0;

    // Make a writable copy of data to insert null terminators
    char *buf = malloc(size + 1);
    if (!buf) {
        kvstore_cleanup();
        return 0;
    }
    memcpy(buf, data, size);
    buf[size] = '\0';

    char *start = buf;
    for (size_t i = 0; i <= size && commands_count < max_commands; i++) {
        if (buf[i] == '\n' || buf[i] == '\0') {
            buf[i] = '\0';
            if (start[0] != '\0') {
                kvstore_handle_command(start);
                commands_count++;
            }
            start = buf + i + 1;
        }
    }

    free(buf);
    kvstore_cleanup();
    return 0;
}