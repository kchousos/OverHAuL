#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "progress.h"
#include "progress.c"  // Include implementation to avoid linker errors

// We define a helper function to safely assign fmt from fuzz input.
// We allocate a buffer, copy fuzz input and null-terminate.
static char *copy_input_fmt(const uint8_t *data, size_t size) {
    // Allocate size+1 to ensure null termination
    char *fmt = (char *)malloc(size + 1);
    if (!fmt) return NULL;
    memcpy(fmt, data, size);
    fmt[size] = '\0';
    return fmt;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Defensive: skip zero size inputs, nothing to test
    if (size == 0) return 0;

    // Create progress_t object with fixed total and width
    progress_t *progress = progress_new(100, 20);
    if (!progress) return 0;

    // Set the fuzz data as the progress format string safely
    char *fmt = copy_input_fmt(data, size);
    if (!fmt) {
        progress_free(progress);
        return 0;
    }
    progress->fmt = fmt;

    // Set bar chars to default single character strings
    progress->bar_char = "=";
    progress->bg_bar_char = "-";

    // Set some progress value to trigger started to true and progress emitting
    // Use progress_value to set a deterministic value from fuzz data size to avoid overflow
    // Clamp value between 0 and total
    int val = (int)(size % 150); // May go beyond total on purpose
    progress_value(progress, val);

    // Call progress_write to trigger formatting and possible vulnerable replace_str()
    progress_write(progress);

    // Cleanup allocations
    free(fmt);
    progress_free(progress);

    return 0;
}