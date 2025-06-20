#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mpc.h"
#include "mpc.c"  // Include implementation to fix linker errors

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  static mpc_parser_t *parser = NULL;
  if (!parser) {
    // Create a regex parser that matches one or more characters (non-empty input)
    parser = mpc_re(".+");
  }

  // Skip empty inputs to save parsing trivial matches
  if (size == 0) {
    return 0;
  }

  // Allocate null-terminated buffer and copy data
  char *buffer = malloc(size + 1);
  if (!buffer) return 0;
  memcpy(buffer, data, size);
  buffer[size] = '\0';

  mpc_result_t r;
  int res = mpc_parse("<fuzzer>", buffer, parser, &r);

  if (res) {
    // Successful parse: output is AST, delete it
    mpc_ast_t *ast = (mpc_ast_t *)r.output;
    mpc_ast_delete(ast);
  } else {
    // Failed parse: delete error
    mpc_err_delete(r.error);
  }

  free(buffer);
  return 0;
}