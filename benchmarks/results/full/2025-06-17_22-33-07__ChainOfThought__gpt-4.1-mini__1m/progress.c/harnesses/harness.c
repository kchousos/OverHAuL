#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "progress.h"

// Minimal event callback which writes progress output
static void progress_event_callback(progress_data_t *data) {
    if (data && data->holder) {
        progress_write(data->holder);
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Defensive: require size > 0 and not too large for format string
    if (size == 0 || size > 512) return 0;

    // Allocate a progress bar with total 100 and width 60 (arbitrary)
    progress_t *progress = progress_new(100, 60);
    if (!progress) return 0;

    // Copy fuzz input into a buffer as the format string, ensure null-terminated
    char fmtbuf[513];
    if (size > 512) size = 512;
    memcpy(fmtbuf, data, size);
    fmtbuf[size] = '\0';

    // Assign fuzz input as format string
    progress->fmt = fmtbuf;

    // Attach event listener for PROGRESS_EVENT_PROGRESS
    // This avoids potential null derefs in callbacks
    if (!progress_on(progress, PROGRESS_EVENT_PROGRESS, progress_event_callback)) {
        progress_free(progress);
        return 0;
    }

    // Simulate progress ticks incrementing by varying amounts derived from input
    // This exercises progress changing and event dispatch
    for (size_t i = 0; i < size; ++i) {
        int tick_val = (int)(data[i] % 10); 
        if (tick_val == 0) tick_val = 1; // Avoid zero increments for meaningful progress
        progress_tick(progress, tick_val);
        if (progress->finished) break; // stop if finished
    }

    // Cleanup progress data
    progress_free(progress);

    return 0;
}