#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "kvstore.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    kvstore_init();

    // We will treat the input data as a sequence of commands separated by '\n'.
    // Each command is a null-terminated string passed to kvstore_handle_command.
    // To avoid excessive memory usage, limit the number of commands processed.
    const size_t max_commands = 100;
    size_t commands_processed = 0;

    size_t start = 0;
    for (size_t i = 0; i <= size && commands_processed < max_commands; i++) {
        if (i == size || data[i] == '\n') {
            size_t len = i - start;
            if (len > 0) {
                // Allocate a buffer for the command string plus null terminator
                char *cmd = (char *)malloc(len + 1);
                if (!cmd) break;
                memcpy(cmd, data + start, len);
                cmd[len] = '\0';

                kvstore_handle_command(cmd);

                free(cmd);
                commands_processed++;
            }
            start = i + 1;
        }
    }

    kvstore_cleanup();
    return 0;
}