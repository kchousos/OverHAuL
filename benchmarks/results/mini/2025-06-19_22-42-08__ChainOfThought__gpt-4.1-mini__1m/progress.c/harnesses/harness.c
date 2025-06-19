#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "progress.h"
#include "progress.c"  // Include implementation to resolve linker errors

// Callback that writes the progress bar upon events
void on_progress(progress_data_t *data) {
    // NOTE: progress_write calls replace_str internally with data->holder->fmt
    progress_write(data->holder);
}

// Helper function to safely copy fuzz input as null-terminated string.
// Copies up to max_len bytes and null terminates.
static char *copy_string(const uint8_t *data, size_t size, size_t max_len) {
    size_t len = (size < max_len) ? size : max_len;
    char *str = malloc(len + 1);
    if (!str) return NULL;
    memcpy(str, data, len);
    str[len] = '\0';
    return str;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 2) return 0; // Need at least bytes for total and width params

    // Extract total and width from first 2 bytes
    // Limit total and width to reasonable max to avoid large allocs
    int total = data[0] + 1;         // total in [1,256]
    size_t width = data[1] % 100 + 1; // width in [1,100]

    // Create new progress object
    progress_t *progress = progress_new(total, width);
    if (!progress) return 0;

    // Use rest of input as format string for progress->fmt with max 200 chars
    char *fmt = NULL;
    bool fmt_allocated = false;
    if (size > 2) {
        fmt = copy_string(data + 2, size - 2, 200);
        if (fmt) {
            progress->fmt = fmt;
            fmt_allocated = true;
        } else {
            // Fallback default format string
            progress->fmt = "progress [:bar] :percent :elapsed";
        }
    } else {
        progress->fmt = "progress [:bar] :percent :elapsed";
    }

    // Register callbacks for all event types
    progress_on(progress, PROGRESS_EVENT_START, on_progress);
    progress_on(progress, PROGRESS_EVENT_PROGRESS, on_progress);
    progress_on(progress, PROGRESS_EVENT_END, on_progress);

    // Drive progress with increments from fuzz data bytes after first 2+fmt_len
    // To not overflow size, start at offset = 2 + length of fmt (if allocated), else 2.
    size_t offset = 2;
    if (fmt_allocated) offset += strlen(fmt);

    // If offset >= size, no increments, just free and exit
    if (offset <= size) {
        size_t i;
        for (i = offset; i < size; i++) {
            uint8_t inc = data[i];
            // Clamp increments to 1..10 to avoid slow progress cycles
            int inc_val = (inc % 10) + 1;
            bool continued = progress_tick(progress, inc_val);
            if (!continued) break; // finished
        }
    }

    if (fmt_allocated) {
        free(fmt);
    }
    progress_free(progress);

    return 0;
}