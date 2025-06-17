#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "chan.h"

// Structure to pass data to sender thread
typedef struct {
    chan_t* chan;
    int32_t value;
    int ops; // number of operations to perform
} sender_arg_t;

// Sender thread func: sends int32 multiple times
static void* sender_thread(void* arg) {
    sender_arg_t* sarg = (sender_arg_t*) arg;
    for (int i = 0; i < sarg->ops; i++) {
        // Try sending; if channel closed or fail, stop
        if (chan_send_int32(sarg->chan, sarg->value) != 0) {
            break;
        }
    }
    return NULL;
}

// Receiver thread func: receive int32 multiple times
static void* receiver_thread(void* arg) {
    sender_arg_t* rarg = (sender_arg_t*) arg;
    int32_t val;
    for (int i = 0; i < rarg->ops; i++) {
        if (chan_recv_int32(rarg->chan, &val) != 0) {
            break;
        }
        // Optional: Check value or do something with val
        (void)val;
    }
    return NULL;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < 5) {
        return 0; // Need minimum data
    }
    // First byte: capacity selector
    size_t capacity = (size_t)(Data[0] % 5); // capacities 0..4 (0 unbuffered)

    // Next 4 bytes: value to send as int32
    int32_t value = 0;
    memcpy(&value, Data + 1, sizeof(int32_t));

    chan_t* chan = chan_init(capacity);
    if (!chan) {
        return 0; // malloc failed, rare
    }

    // Number ops from 1 to 10 at most
    int ops = (Size > 5) ? (Data[5] % 10) + 1 : 1;

    // Create sender/receiver args
    sender_arg_t sarg = {chan, value, ops};
    sender_arg_t rarg = {chan, 0, ops};

    pthread_t snd_thread;
    pthread_t rcv_thread;

    // Start receiver thread first to avoid blocking sender on unbuffered
    if (pthread_create(&rcv_thread, NULL, receiver_thread, &rarg) != 0) {
        chan_dispose(chan);
        return 0;
    }
    if (pthread_create(&snd_thread, NULL, sender_thread, &sarg) != 0) {
        pthread_cancel(rcv_thread);
        pthread_join(rcv_thread, NULL);
        chan_dispose(chan);
        return 0;
    }

    // Wait for both to finish
    pthread_join(snd_thread, NULL);
    pthread_join(rcv_thread, NULL);

    // Try closing the channel and sending after close (should fail)
    chan_close(chan);
    // Sending after closed returns -1
    (void)chan_send_int32(chan, value);

    chan_dispose(chan);
    return 0;
}