#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <cargs.h>

#define MAX_ARGS 64
#define MAX_ARG_LEN 256

// Define options to test
static struct cag_option test_options[] = {
    {.identifier = 'a',
     .access_letters = "a",
     .access_name = "alpha",
     .value_name = NULL,
     .description = "Alpha flag"},
    {.identifier = 'b',
     .access_letters = "bB",
     .access_name = "beta",
     .value_name = "VALUE",
     .description = "Beta option with value"},
    {.identifier = 'c',
     .access_letters = NULL,
     .access_name = "charlie",
     .value_name = NULL,
     .description = "Charlie long flag"},
    {.identifier = 'd',
     .access_letters = "d",
     .access_name = NULL,
     .value_name = NULL,
     .description = "Delta flag"},
    {.identifier = 'h',
     .access_letters = "h",
     .access_name = "help",
     .value_name = NULL,
     .description = "Help flag"},
};

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // We must produce argv array:
    // argv[0] = "program"
    // argv[1..] = strings derived from data input
    // Cap max args and arg length
    char *argv[MAX_ARGS];
    int argc = 1;

    // argv[0] program name
    argv[0] = "program";

    // Temporary buffer to hold one argument string
    char argbuf[MAX_ARG_LEN];
    size_t arglen = 0;

    // We will split input data into arguments separated by zero bytes or spaces
    // The fuzz data may be arbitrary bytes, so we treat 0x00 and 0x20 (space) as delimiters.
    for (size_t i = 0; i < size && argc < MAX_ARGS; ++i) {
        uint8_t c = data[i];
        if (c == 0 || c == 0x20) {
            // End current argument if any
            if (arglen > 0) {
                // Null terminate
                argbuf[arglen] = '\0';
                // Allocate new string and copy
                char *arg = (char *)malloc(arglen + 1);
                if (arg == NULL) {
                    // malloc failure, free allocated arguments and return
                    for (int j = 1; j < argc; ++j) free(argv[j]);
                    return 0;
                }
                memcpy(arg, argbuf, arglen + 1);
                argv[argc++] = arg;
                arglen = 0;
            }
        } else {
            if (arglen + 1 < MAX_ARG_LEN) {
                argbuf[arglen++] = (char)c;
            }
            // else truncate
        }
    }
    // Add last argument if any
    if (arglen > 0 && argc < MAX_ARGS) {
        argbuf[arglen] = '\0';
        char *arg = (char *)malloc(arglen + 1);
        if (arg == NULL) {
            for (int j = 1; j < argc; ++j) free(argv[j]);
            return 0;
        }
        memcpy(arg, argbuf, arglen + 1);
        argv[argc++] = arg;
    }

    // If only program name, no args, just skip fetch loop
    if (argc <= 1) {
        for (int j = 1; j < argc; ++j) free(argv[j]);
        return 0;
    }

    // Setup context and init
    cag_option_context context;
    cag_option_init(&context, test_options, sizeof(test_options) / sizeof(test_options[0]), argc, argv);

    // Fetch all options until no more
    while (cag_option_fetch(&context)) {
        char id = cag_option_get_identifier(&context);
        const char *val = cag_option_get_value(&context);
        // Access these values to ensure they are processed by fuzzer
        // Also handle unknown option case
        if (id == '?') {
            // Option unknown; get error letter and index
            char error_letter = context.error_letter;
            int err_index = context.error_index;
            (void)error_letter;
            (void)err_index;
        } else {
            // Just dummy use of val if present
            if (val) {
                // Access val string bytes to avoid optimizer removing them
                for (size_t i = 0; val[i]; ++i) {
                    volatile char c = val[i];
                    (void)c;
                }
            }
        }
    }

    // Free allocated argv (skip argv[0])
    for (int j = 1; j < argc; ++j) {
        free(argv[j]);
    }

    return 0;
}