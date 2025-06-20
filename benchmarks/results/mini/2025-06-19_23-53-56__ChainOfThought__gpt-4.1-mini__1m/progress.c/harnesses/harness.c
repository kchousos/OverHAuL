#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "progress.h"
#include "progress.c"  // Include implementation to avoid linker errors

// Callback triggered on PROGRESS_EVENT_PROGRESS
static void on_progress(progress_data_t *data) {
    if (data && data->holder) {
        progress_write(data->holder);
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) return 0;

    // Allocate format string, ensuring null termination
    char *fmt = (char *)malloc(size + 1);
    if (!fmt) return 0;
    memcpy(fmt, data, size);
    fmt[size] = '\0';

    const int total = 100;
    const size_t width = 40;

    // Create progress instance
    progress_t *progress = progress_new(total, width);
    if (!progress) {
        free(fmt);
        return 0;
    }

    // Set progress format string to fuzzed input
    progress->fmt = fmt;

    // Register progress event callback
    bool registered = progress_on(progress, PROGRESS_EVENT_PROGRESS, on_progress);
    if (!registered) {
        progress_free(progress);
        free(fmt);
        return 0;
    }

    // Set step to at least 1 and less than total
    int step = (size > 0) ? (data[0] % total) : 1;
    if (step == 0) step = 1;

    // Advance progress
    progress_tick(progress, step);

    // Free resources
    progress_free(progress);
    free(fmt);

    return 0;
}