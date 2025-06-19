#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "torrent_reader.h"

// No-op implementations for the callbacks to satisfy tfr_new interface:

static int cb_event(void* udata, const char* key)
{
    (void)udata;
    (void)key;
    return 0;
}

static int cb_event_str(void* udata, const char* key, const char* val, int len)
{
    (void)udata;
    (void)key;
    (void)val;
    (void)len;
    return 0;
}

static int cb_event_int(void* udata, const char* key, int val)
{
    (void)udata;
    (void)key;
    (void)val;
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    void *reader = tfr_new(cb_event, cb_event_str, cb_event_int, NULL);
    if (!reader)
        return 0;

    tfr_read_metainfo(reader, (const char*)Data, (int)Size);

    // Do NOT free reader because no tfr_free is provided.
    // free(reader);

    return 0;
}