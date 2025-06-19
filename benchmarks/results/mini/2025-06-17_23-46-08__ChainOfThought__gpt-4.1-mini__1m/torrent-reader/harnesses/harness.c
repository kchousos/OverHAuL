#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "torrent_reader.h"

// Callback stubs that safely do nothing and return 0.
static int cb_event(void* udata, const char* key) {
    (void)udata;
    (void)key;
    return 0;
}

static int cb_event_str(void* udata, const char* key, const char* val, int len) {
    (void)udata;
    (void)key;
    (void)val;
    (void)len;
    return 0;
}

static int cb_event_int(void* udata, const char* key, int val) {
    (void)udata;
    (void)key;
    (void)val;
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // Create the torrent reader context with no meaningful user data.
    void* ctx = tfr_new(cb_event, cb_event_str, cb_event_int, NULL);
    if (!ctx) {
        // Allocation failed, just return.
        return 0;
    }
    // Pass the fuzzing input to the metainfo reader.
    tfr_read_metainfo(ctx, (const char*)Data, (int)Size);

    // No freeing context because tfr_new does not have a corresponding free in the provided source.
    // Assuming the scope is small and fuzz run ephemeral.

    return 0;
}

/*
 * NOTE: This harness requires linking with torrent_reader.c and bencode.c during compilation, for example:
 * clang -fsanitize=fuzzer -o harness harness.c torrent_reader.c bencode.c
 */