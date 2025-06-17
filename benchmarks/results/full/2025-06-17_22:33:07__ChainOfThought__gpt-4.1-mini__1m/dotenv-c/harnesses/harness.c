#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdbool.h>

// Declaration of the function under test from dotenv.h
int env_load(const char* path, bool overwrite);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Create a temporary file
    char tmp_filename[] = "/tmp/libfuzzer-env-XXXXXX";
    int fd = mkstemp(tmp_filename);
    if (fd == -1) {
        return 0;
    }

    // Write the fuzz data to the file
    ssize_t written = write(fd, data, size);
    (void)written; // suppress unused warning

    close(fd);

    // Call the dotenv loader with overwrite=true to test replacement path
    env_load(tmp_filename, true);

    // Remove the temporary file
    unlink(tmp_filename);

    return 0;
}