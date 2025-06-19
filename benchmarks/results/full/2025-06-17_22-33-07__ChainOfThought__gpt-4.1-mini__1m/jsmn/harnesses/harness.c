#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "jsmn.h"

// Define a sufficiently large token buffer to allow parsing complex JSON structures.
#define MAX_TOKENS 1024

// libFuzzer entry point.
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  jsmn_parser parser;
  jsmntok_t tokens[MAX_TOKENS];

  jsmn_init(&parser);
  // Cast data to const char* for the JSON string.
  // Pass tokens buffer and its size.
  (void) jsmn_parse(&parser, (const char *)data, size, tokens, MAX_TOKENS);

  return 0; // Non-zero return values are reserved by libFuzzer.
}