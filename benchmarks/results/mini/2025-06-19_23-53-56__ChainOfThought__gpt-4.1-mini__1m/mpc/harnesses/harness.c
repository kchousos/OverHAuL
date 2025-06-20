#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mpc.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {

  if (Size == 0) { return 0; }

  // Copy data to null-terminated string
  char *input = malloc(Size + 1);
  if (!input) { return 0; }
  memcpy(input, Data, Size);
  input[Size] = '\0';

  // Create a dummy parser to pass to mpca_lang (avoids crash due to missing parsers)
  mpc_parser_t *dummy = mpc_new("dummy");

  // Parse the fuzz input as a language grammar string with dummy parser passed
  mpc_err_t *err = mpca_lang(MPCA_LANG_DEFAULT, input, dummy, NULL);

  if (err) {
    mpc_err_delete(err);
  }

  mpc_delete(dummy);
  free(input);

  return 0;
}

/* 
 * To compile and link properly this harness, you MUST include the mpc.c implementation file:
 * For example:
 *    clang harness.c mpc.c -o harness
 */