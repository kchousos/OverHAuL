#include "progress.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <progress.h>

// Callback that calls progress_write to print current progress status
static void on_progress(progress_data_t *data) {
    progress_write(data->holder);
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < 5) return 0; // need some data to proceed

    // Parse integers for total and width from first bytes
    // Limit total and width to reasonable max to avoid huge memory use
    int total = (Data[0] << 8) | Data[1];
    size_t width = (Data[2] << 8) | Data[3];

    // clamp total and width to > 0 and some max
    if (total <= 0) total = 100;
    if (width <= 0 || width > 1000) width = 60;

    // Create progress bar
    progress_t *progress = progress_new(total, width);
    if (!progress) return 0;

    // Remaining data as format string (null terminated or truncated)
    size_t fmt_len = Size - 4;
    char *fmt = malloc(fmt_len + 1);
    if (!fmt) {
        progress_free(progress);
        return 0;
    }
    memcpy(fmt, Data + 4, fmt_len);
    fmt[fmt_len] = 0; // Null terminate

    progress->fmt = fmt;

    // Register one event listener for PROGRESS_EVENT_PROGRESS
    progress_on(progress, PROGRESS_EVENT_PROGRESS, on_progress);

    // Now feed tick increments from the fmt data itself to cover different increments
    // Iterate over each byte, interpret as small increments 0-20
    // This stresses progress_tick and progress change
    for (size_t i = 0; i < fmt_len; i++) {
        int increment = Data[4 + i] % 21; // increments 0-20
        progress_tick(progress, increment);
    }

    // Cleanup
    progress_free(progress);
    free(fmt);

    return 0;
}