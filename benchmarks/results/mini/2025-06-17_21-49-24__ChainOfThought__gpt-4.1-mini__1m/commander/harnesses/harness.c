#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "commander.h"

// Maximum number of arguments to simulate
#define MAX_FUZZ_ARGS 64
// Maximum length per argument string
#define MAX_ARG_LEN 128

// Helper to tokenize input data into argv, splitting on non-printable/spaces
static int fuzz_data_to_argv(const uint8_t *data, size_t size, char **argv_out, int max_args) {
    int argc = 0;
    size_t pos = 0;

    while (pos < size && argc < max_args) {
        // Skip initial non-printable or whitespace
        while (pos < size && (data[pos] == 0 || data[pos] == ' ' || data[pos] == '\n' || data[pos] == '\r' || data[pos] == '\t'))
            pos++;
        if (pos >= size)
            break;

        // Copy up to MAX_ARG_LEN or until next whitespace/non-printable
        size_t arg_start = pos;
        size_t arg_len = 0;
        while (pos < size && data[pos] > 32 && data[pos] < 127 && arg_len < MAX_ARG_LEN) {
            pos++;
            arg_len++;
        }

        if (arg_len == 0) {
            // If no valid arg characters, break out
            break;
        }

        char *arg = (char *)malloc(arg_len + 1);
        if (!arg) {
            // Allocation failure: free prior and return
            for (int i = 0; i < argc; i++)
                free(argv_out[i]);
            return 0;
        }
        memcpy(arg, data + arg_start, arg_len);
        arg[arg_len] = '\0';

        argv_out[argc++] = arg;
    }

    return argc;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0)
        return 0;

    // Allocate argv array
    // +1 for command name (argv[0]) + max args from data
    char *argv[MAX_FUZZ_ARGS + 1];
    int argc = 0;

    // set argv[0] as "fuzzcmd"
    argv[0] = strdup("fuzzcmd");
    if (!argv[0])
        return 0;
    argc = 1;

    // parse fuzz input into argv starting at argv[1]
    int parsed_args = fuzz_data_to_argv(data, size, &argv[1], MAX_FUZZ_ARGS);
    if (parsed_args < 0) {
        free(argv[0]);
        return 0;
    }
    argc += parsed_args;
    if (argc > MAX_FUZZ_ARGS + 1)
        argc = MAX_FUZZ_ARGS + 1;

    // null terminate argv array for commander (not strictly needed but safe)
    if (argc < MAX_FUZZ_ARGS + 1)
        argv[argc] = NULL;

    // Initialize command object
    command_t cmd;
    memset(&cmd, 0, sizeof(cmd));

    command_init(&cmd, argv[0], "1.0-fuzz");

    // Do not add extra options to avoid exits in callbacks except defaults

    // Call parse
    // commander expects argv array of argc pointers
    // It modifies internally so safe to pass
    command_parse(&cmd, argc, argv);

    // Cleanup after parsing
    command_free(&cmd);

    // Free allocated argv strings
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }

    return 0;
}