#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sixpack.h"

struct fuzz_data {
    const uint8_t *data;
    size_t size;
    size_t pos;
};

static int fuzz_getchar(struct sixpack *sp) {
    struct fuzz_data *fd = (struct fuzz_data *)sp->userdata;
    if (fd->pos >= fd->size)
        return SIXPACK_IO_EOF;
    return fd->data[fd->pos++];
}

static void* fuzz_process(struct sixpack *sp, enum sixpack_type type, const char *data, unsigned data_size) {
    (void)sp; (void)type; (void)data; (void)data_size;
    return SIXPACK_CONTINUE;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    struct sixpack sp = {0};
    struct fuzz_data fd = {
        .data = data,
        .size = size,
        .pos = 0
    };

    sp.userdata = &fd;
    sp.getchar = fuzz_getchar;
    sp.process = fuzz_process;

    sixpack_parse(&sp);

    return 0;
}