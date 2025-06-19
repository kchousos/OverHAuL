#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "torrent_reader.h"

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

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) {
        return 0;
    }

    void* ctx = tfr_new(cb_event, cb_event_str, cb_event_int, NULL);
    if (!ctx) {
        return 0;
    }

    tfr_read_metainfo(ctx, (const char*)data, (int)size);

    free(ctx);
    return 0;
}