#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "libchron.h"

// Dummy callback for timers: does nothing.
void dummy_timer_callback(chron_timer_t* timer, void* arg) {
    (void)timer;
    (void)arg;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(unsigned long)*4 + sizeof(uint32_t) + 1) {
        // Not enough data for parameters, skip.
        return 0;
    }

    size_t offset = 0;

    // Helper to read unsigned long safely
    if (offset + sizeof(unsigned long) > size) return 0;
    unsigned long expiry_ms;
    memcpy(&expiry_ms, data + offset, sizeof(unsigned long));
    offset += sizeof(unsigned long);

    if (offset + sizeof(unsigned long) > size) return 0;
    unsigned long interval_ms;
    memcpy(&interval_ms, data + offset, sizeof(unsigned long));
    offset += sizeof(unsigned long);

    if (offset + sizeof(uint32_t) > size) return 0;
    uint32_t max_expirations;
    memcpy(&max_expirations, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (offset + 1 > size) return 0;
    bool is_exponential = data[offset] & 1;
    offset += 1;

    // Create a timer
    chron_timer_t* timer = chron_timer_init(dummy_timer_callback, NULL, expiry_ms, interval_ms, max_expirations, is_exponential);
    if (!timer) {
        // malloc or timer_create failed; not a crash, so just exit.
        return 0;
    }

    // Now fuzz various API calls with remaining data if any
    size_t remaining = size - offset;

    for (size_t i = 0; i + sizeof(unsigned long)*2 <= remaining; i += sizeof(unsigned long)*2) {
        unsigned long new_expiry, new_interval;
        memcpy(&new_expiry, data + offset + i, sizeof(unsigned long));
        memcpy(&new_interval, data + offset + i + sizeof(unsigned long), sizeof(unsigned long));

        // Try reschedule
        (void)chron_timer_reschedule(timer, new_expiry, new_interval);
        // Try restart
        (void)chron_timer_restart(timer);
        // Try pause and resume in alternation
        chron_timer_pause(timer);
        chron_timer_resume(timer);
    }

    // Cancel and delete timer safely
    chron_timer_cancel(timer);
    chron_timer_delete(timer);

    return 0;
}