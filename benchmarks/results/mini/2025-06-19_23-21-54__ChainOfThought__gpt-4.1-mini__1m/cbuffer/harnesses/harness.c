#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>

/** Begin embedded cbuffer.c and cbuffer.h implementation **/

#ifndef CBUFFER_H
#define CBUFFER_H

typedef struct
{
    unsigned long int size;
    unsigned int head, tail;
    void *data;
} cbuf_t;

cbuf_t *cbuf_new(const unsigned int order);
void cbuf_free(cbuf_t* cb);
int cbuf_offer(cbuf_t* cb, const unsigned char *data, const int size);
unsigned char *cbuf_peek(const cbuf_t* cb);
unsigned char *cbuf_poll(cbuf_t *cb, const unsigned int size);
int cbuf_size(const cbuf_t* cb);
int cbuf_usedspace(const cbuf_t* cb);
int cbuf_unusedspace(const cbuf_t* cb);
int cbuf_is_empty(const cbuf_t* cb);

#endif /* CBUFFER_H */

#define fail() assert(0)

#ifndef MAP_ANONYMOUS
#  define MAP_ANONYMOUS MAP_ANON
#endif

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
    if (!me) return NULL;
    me->size = 1UL << order;
    me->head = me->tail = 0;
    __create_buffer_mirror(me);
    return me;
}

void cbuf_free(cbuf_t *me)
{
    munmap(me->data, me->size << 1);
    free(me);
}

int cbuf_is_empty(const cbuf_t *me)
{
    return me->head == me->tail;
}

int cbuf_offer(cbuf_t *me, const unsigned char *data, const int size)
{
    if (cbuf_unusedspace(me) < size)
        return 0;

    int written = cbuf_unusedspace(me);
    written = size < written ? size : written;
    memcpy(me->data + me->tail, data, written);
    me->tail += written;
    if (me->tail >= me->size)
        me->tail %= me->size;
    return written;
}

unsigned char *cbuf_peek(const cbuf_t *me)
{
    if (cbuf_is_empty(me))
        return NULL;
    return me->data + me->head;
}

unsigned char *cbuf_poll(cbuf_t *me, const unsigned int size)
{
    if (cbuf_is_empty(me))
        return NULL;

    if(size > (unsigned int)cbuf_usedspace(me))
        return NULL;

    void *end = me->data + me->head;
    me->head += size;
    if (me->head >= me->size)
        me->head %= me->size;
    return end;
}

int cbuf_size(const cbuf_t *me)
{
    return me->size;
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

/** End embedded cbuffer code **/

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Use smaller buffer for faster wrap and bug finding
    cbuf_t *cb = cbuf_new(4); // 16 bytes buffer
    if (!cb) return 0;

    size_t pos = 0;
    size_t iterations = 0;

    // Limit iterations to smaller fixed number for quicker cycling
    size_t max_iter = size > 32 ? 32 : size / 2;

    while (pos + 2 <= size && iterations < max_iter) {
        ++iterations;

        // Each cycle uses two control bytes: offer_len, poll_len
        uint8_t offer_len = data[pos];
        uint8_t poll_len = data[pos + 1];
        pos += 2;

        // Clamp offer_len loosely to test boundaries; occasionally allow over-offer attempt
        if (offer_len > (uint8_t)(cb->size))
            offer_len = cb->size + (offer_len % 4); 

        if (offer_len > size - pos)
            offer_len = (uint8_t)(size - pos); // allow zero if no data

        const unsigned char *offer_data = data + pos;

        int offered = cbuf_offer(cb, offer_data, offer_len);
        // Advance position by actual bytes offered only
        pos += offered;

        // For poll_len, allow attempts to poll more than used space, adjust down accordingly
        int used = cbuf_usedspace(cb);
        if (poll_len > (uint8_t)used)
            poll_len = used;

        unsigned char *polled = cbuf_poll(cb, poll_len);

        // Access polled data if any to prevent optimizing away
        if (polled && poll_len > 0) {
            volatile unsigned char dummy = polled[0];
            (void)dummy;
        }

        // Also peek and touch data
        unsigned char *peeked = cbuf_peek(cb);
        if (peeked) {
            volatile unsigned char dummy = peeked[0];
            (void)dummy;
        }

        // Check buffer invariants after every op
        int used2 = cbuf_usedspace(cb);
        int unused = cbuf_unusedspace(cb);
        int size_buf = cbuf_size(cb);
        if(used2 < 0 || unused < 0 || (used2 + unused) != size_buf) {
            __builtin_trap();
        }
    }

    cbuf_free(cb);
    return 0;
}