#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "mpc.h"

static mpc_parser_t *parser = NULL;

int LLVMFuzzerInitialize(int *argc, char ***argv) {
  (void)argc; (void)argv;
  parser = mpc_ident();
  return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (parser == NULL) {
    return 0;
  }

  // Convert fuzz input to string by copying and null-terminating
  char *input_str = (char *)malloc(size + 1);
  if (!input_str) {
    return 0;
  }
  memcpy(input_str, data, size);
  input_str[size] = '\0';

  mpc_result_t r;
  int res = mpc_parse("<fuzz_input>", input_str, parser, &r);

  if (res) {
    // Parse success, free output AST
    mpc_ast_delete(r.output);
  } else {
    // Parse failed, free error
    mpc_err_delete(r.error);
  }

  free(input_str);
  return 0;
}

void LLVMFuzzerShutdown(void) {
  if (parser) {
    mpc_delete(parser);
    parser = NULL;
  }
}