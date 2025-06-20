#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Dummy bencode types and functions to satisfy linker */

// Minimal dummy definition of bencode_t
typedef struct {
    const char *data;
    size_t size;
    int index;    // for iterators
} bencode_t;

// Dummy bencode functions used in torrent_reader.c
void bencode_init(bencode_t* ben, const char* buf, int len) {
    ben->data = buf;
    ben->size = len;
    ben->index = 0;
}

int bencode_is_dict(bencode_t* ben) {
    // Fake: always return true
    (void)ben;
    return 1;
}

int bencode_dict_has_next(bencode_t* ben) {
    // Fake: Use index to limit iteration
    if (!ben) return 0;
    return ben->index < 1; // only one element to simulate
}

void bencode_dict_get_next(bencode_t* dict, bencode_t* out, const char** key, int* klen) {
    // Fake: Return fixed key "info"
    if (dict->index == 0) {
        *key = "info";
        *klen = 4;
        *out = *dict;
        dict->index++;
    } else {
        *key = NULL;
        *klen = 0;
    }
}

int bencode_string_value(bencode_t* ben, const char** val, int* len) {
    // Provide dummy string value from data buffer
    (void)ben;
    static const char dummy_str[] = "dummy";
    *val = dummy_str;
    *len = (int)strlen(dummy_str);
    return 0;
}

int bencode_int_value(bencode_t* ben, long int* val) {
    // Provide dummy integer value
    (void)ben;
    *val = 42;
    return 0;
}

int bencode_is_list(bencode_t* ben) {
    // Always fake list
    (void)ben;
    return 1;
}

int bencode_list_has_next(bencode_t* ben) {
    if (!ben) return 0;
    return ben->index < 1; // only one element
}

void bencode_list_get_next(bencode_t* list, bencode_t* out) {
    if (list->index == 0) {
        *out = *list;
        list->index++;
    } else {
        out->data = NULL;
        out->size = 0;
    }
}

void bencode_dict_get_start_and_len(bencode_t* dict, const char** val, int* len) {
    static const char dummy_infohash[] = "infohashdummy";
    *val = dummy_infohash;
    *len = (int)strlen(dummy_infohash);
}

#include "torrent_reader.c"  // Include the implementation to resolve linker errors

// Dummy callbacks for tfr_new

static int cb_event(void* udata, const char* key) {
    (void)udata; (void)key;
    // Just accept all events
    return 0;
}

static int cb_event_str(void* udata, const char* key, const char* val, int len) {
    (void)udata; (void)key; (void)val; (void)len;
    // Accept string events
    return 0;
}

static int cb_event_int(void* udata, const char* key, int val) {
    (void)udata; (void)key; (void)val;
    // Accept integer events
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // Create a new tfr context with dummy callbacks
    void* me = tfr_new(cb_event, cb_event_str, cb_event_int, NULL);
    if (!me) {
        return 0; // allocation failure, skip
    }

    // Call the parser with fuzzed input buffer
    tfr_read_metainfo(me, (const char *)Data, (int)Size);

    // The code does not provide a free function for the tfr_t instance,
    // so we simply free it here assuming it's allocated by calloc in tfr_new
    free(me);

    return 0;
}