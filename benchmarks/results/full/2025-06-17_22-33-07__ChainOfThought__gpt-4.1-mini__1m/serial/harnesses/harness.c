#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "serial.h"

/*
 * We reuse the serialWriteRaw function from serial.c.
 * The harness creates a pipe and fuzzes the write end.
 */

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    int fds[2];
    if (pipe(fds) == -1) {
        // Can't create pipe, skip this input
        return 0;
    }
    int write_fd = fds[1];
    int read_fd = fds[0];

    // Set write_fd non-blocking to not block indefinitely in serialWriteRaw
    int flags = fcntl(write_fd, F_GETFL, 0);
    if (flags != -1) {
        fcntl(write_fd, F_SETFL, flags | O_NONBLOCK);
    }

    // Call serialWriteRaw on the write end of the pipe with fuzz data
    // This will call write() syscall in a loop until all data is sent or timeout
    (void)serialWriteRaw(write_fd, (const char *)data, (int)size);

    // Clean up
    close(write_fd);
    close(read_fd);
    return 0;
}