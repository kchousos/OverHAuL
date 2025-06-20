#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "torrent_reader.h"

// Dummy callbacks that do nothing, return 0 always.
static int dummy_cb_event(void* udata, const char* key) {
    (void)udata; (void)key;
    return 0;
}

static int dummy_cb_event_str(void* udata, const char* key, const char* val, int len) {
    (void)udata; (void)key; (void)val; (void)len;
    return 0;
}

static int dummy_cb_event_int(void* udata, const char* key, int val) {
    (void)udata; (void)key; (void)val;
    return 0;
}

// Include the source file to provide definitions for tfr_new and tfr_read_metainfo
#include "torrent_reader.c"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    void *ctx = tfr_new(dummy_cb_event, dummy_cb_event_str, dummy_cb_event_int, NULL);
    if (ctx) {
        tfr_read_metainfo(ctx, (const char*)data, (int)size);
        free(ctx);
    }
    return 0;
}