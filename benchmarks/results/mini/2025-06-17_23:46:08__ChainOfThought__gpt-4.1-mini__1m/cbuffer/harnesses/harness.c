#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

// Begin embedded cbuffer.c code

#define fail() assert(0)

/** OSX needs some help here */
#ifndef MAP_ANONYMOUS
#  define MAP_ANONYMOUS MAP_ANON
#endif

typedef struct
{
    unsigned long int size;
    unsigned int head, tail;
    void *data;
} cbuf_t;

static void __create_buffer_mirror(cbuf_t* cb)
{
    char path[] = "/tmp/cb-XXXXXX";
    int fd, status;
    void *address;

    fd = mkstemp(path);
    if (fd < 0)
        fail();

    status = unlink(path);
    if (status)
        fail();

    status = ftruncate(fd, cb->size);
    if (status)
        fail();

    /* create the array of data */
    cb->data = mmap(NULL, cb->size << 1, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE,
                    -1, 0);
    if (cb->data == MAP_FAILED)
        fail();

    address = mmap(cb->data, cb->size, PROT_READ | PROT_WRITE,
                   MAP_FIXED | MAP_SHARED, fd, 0);
    if (address != cb->data)
        fail();

    address = mmap(cb->data + cb->size, cb->size, PROT_READ | PROT_WRITE,
                   MAP_FIXED | MAP_SHARED, fd, 0);
    if (address != cb->data + cb->size)
        fail();

    status = close(fd);
    if (status)
        fail();
}

cbuf_t *cbuf_new(const unsigned int order)
{
    cbuf_t *me = malloc(sizeof(cbuf_t));
    if (!me)
        return NULL;
    me->size = 1UL << order;
    me->head = me->tail = 0;
    __create_buffer_mirror(me);
    return me;
}

void cbuf_free(cbuf_t *me)
{
    if (me) {
        munmap(me->data, me->size << 1);
        free(me);
    }
}

int cbuf_is_empty(const cbuf_t *me)
{
    return me->head == me->tail;
}

int cbuf_usedspace(const cbuf_t *me)
{
    if (me->head <= me->tail)
        return me->tail - me->head;
    else
        return me->size - (me->head - me->tail);
}

int cbuf_unusedspace(const cbuf_t *me)
{
    return me->size - cbuf_usedspace(me);
}

int cbuf_offer(cbuf_t *me, const unsigned char *data, const int size)
{
    /* prevent buffer from getting completely full or over commited */
    if (cbuf_unusedspace(me) <= size)
        return 0;

    int written = cbuf_unusedspace(me);
    written = size < written ? size : written;
    memcpy((unsigned char*)me->data + me->tail, data, written);
    me->tail += written;
    if (me->size < me->tail)
        me->tail %= me->size;
    return written;
}

unsigned char *cbuf_peek(const cbuf_t *me)
{
    if (cbuf_is_empty(me))
        return NULL;

    return (unsigned char*)me->data + me->head;
}

unsigned char *cbuf_poll(cbuf_t *me, const unsigned int size)
{
    if (cbuf_is_empty(me))
        return NULL;

    unsigned char *end = (unsigned char*)me->data + me->head;
    me->head += size;
    if (me->head > me->size)
        me->head %= me->size;
    return end;
}

int cbuf_size(const cbuf_t *me)
{
    return me->size;
}

// End embedded cbuffer.c code


// Fuzz target function required by libFuzzer
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0)
        return 0;

    // Create circular buffer of size 2^12 (4096 bytes)
    cbuf_t *cb = cbuf_new(12);
    if (!cb)
        return 0; // malloc failure

    size_t offset = 0;

    // Loop over input data; interpret as operation commands
    while (offset < size) {
        uint8_t cmd = data[offset++];
        switch (cmd % 2) {
            case 0: // cbuf_offer
            {
                // Offer a chunk of data to the buffer
                if (offset >= size)
                    break;

                // Use next byte as length of chunk to offer (max 256)
                uint8_t chunk_len = data[offset++];
                // Bound chunk_len by remaining input, buffer capacity
                size_t max_len = size - offset;
                if (chunk_len > max_len)
                    chunk_len = (uint8_t)max_len;
                if (chunk_len > (size_t)cbuf_unusedspace(cb))
                    chunk_len = (uint8_t)cbuf_unusedspace(cb);

                if (chunk_len > 0) {
                    // Offer chunk_len bytes to buffer
                    cbuf_offer(cb, data + offset, chunk_len);
                    offset += chunk_len;
                }
                break;
            }
            case 1: // cbuf_poll
            {
                if (offset >= size)
                    break;

                // Use next byte as size of poll to perform
                uint8_t poll_len = data[offset++];
                // Bound poll size by used space in buffer
                int used = cbuf_usedspace(cb);
                if (used == 0)
                    break;
                if (poll_len > (uint8_t)used)
                    poll_len = (uint8_t)used;

                if (poll_len > 0)
                    cbuf_poll(cb, poll_len);
                break;
            }
        }
    }

    cbuf_free(cb);

    return 0;
}