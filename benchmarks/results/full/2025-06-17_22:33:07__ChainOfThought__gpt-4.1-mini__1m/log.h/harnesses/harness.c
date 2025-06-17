#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Limit size to reasonable max to avoid stack overflow
    if (size > 4095) {
        size = 4095;
    }
    // Create a buffer to hold the fuzz input plus null terminator
    char fmt[4096];
    memcpy(fmt, data, size);
    fmt[size] = '\0';  // Null terminate

    // Call the log_info macro with the fuzz data as format string
    // No additional format arguments provided to test robustness of formatting
    log_info(fmt);

    return 0;
}