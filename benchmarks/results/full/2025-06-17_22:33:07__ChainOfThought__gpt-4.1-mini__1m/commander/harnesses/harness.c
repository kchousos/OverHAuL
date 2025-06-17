#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Forward declare command and functions from the library.
typedef struct command command_t;
typedef void (* command_callback_t)(command_t *self);

typedef struct {
  int optional_arg;
  int required_arg;
  char *argname;
  char *large;
  const char *small;
  const char *large_with_arg;
  const char *description;
  command_callback_t cb;
} command_option_t;

struct command {
  void *data;
  const char *usage;
  const char *arg;
  const char *name;
  const char *version;
  int option_count;
  command_option_t options[32];
  int argc;
  char *argv[32];
  char **nargv;
};

void command_init(command_t *self, const char *name, const char *version);
void command_free(command_t *self);
void command_parse(command_t *self, int argc, char **argv);
void command_option(command_t *self, const char *small, const char *large, const char *desc, command_callback_t cb);
void command_help(command_t *self);

// Override exit to avoid exiting during fuzzing.
void exit(int status) {
  // Instead of exiting, abort to signal error to fuzzer.
  // For normal exit 0, just return.
  if (status == 0) {
    return;
  }
  abort();
}

// libFuzzer entry point:
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // We'll parse the input into an argv array of strings.
  // Let's create up to COMMANDER_MAX_ARGS (32) arguments.
  // We'll split input on null bytes and spaces to separate arguments.

  const int max_args = 32;
  char *argv[max_args];
  int argc = 0;

  // Temporary buffer to hold a current argument.
  char current_arg[256];
  size_t current_len = 0;

  // Helper to finalize current argument and add to argv.
  auto add_arg = [&argc, &argv, &current_arg, &current_len]() {
    if (current_len > 0 && argc < max_args) {
      current_arg[current_len] = '\0';
      // Duplicate string to heap
      argv[argc] = (char *)malloc(current_len + 1);
      if (argv[argc]) {
        memcpy(argv[argc], current_arg, current_len + 1);
        argc++;
      }
      current_len = 0;
    }
  };

  for (size_t i = 0; i < size; i++) {
    char c = (char)data[i];
    // Use null byte or whitespace as separator
    if (c == '\0' || c == ' ' || c == '\t' || c == '\r' || c == '\n') {
      add_arg();
    } else {
      if (current_len + 1 < sizeof(current_arg)) {
        current_arg[current_len++] = c;
      } else {
        // Argument too long, finalize and start new
        add_arg();
        current_arg[current_len++] = c;
      }
    }
  }
  add_arg();

  if (argc == 0) {
    // Provide at least one argument (program name)
    argv[0] = (char *)malloc(2);
    if (argv[0]) {
      argv[0][0] = 'a';
      argv[0][1] = '\0';
      argc = 1;
    }
  }

  command_t cmd;
  // Clear command
  memset(&cmd, 0, sizeof(cmd));
  command_init(&cmd, argv[0], "1.0");

  // Parse the arguments
  // Note: command_parse will call exit if errors.
  // We override exit to abort, so fuzzer sees crashes.

  command_parse(&cmd, argc, argv);

  command_free(&cmd);

  // Free arguments
  for (int i = 0; i < argc; i++) {
    free(argv[i]);
  }

  return 0;
}