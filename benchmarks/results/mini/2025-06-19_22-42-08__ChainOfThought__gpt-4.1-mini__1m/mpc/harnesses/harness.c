#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "mpc.c"
#include "mpc.h"

static mpc_val_t *dummy_fold(int n, mpc_val_t **xs) {
  (void)n; (void)xs; return NULL;
}

// Create dummy parser that matches a single character 'a'
static mpc_parser_t *create_dummy_a(void) {
  mpc_parser_t *p = mpc_new("dummy_a");
  mpc_define(p, mpc_char('a'));
  return p;
}

// Create dummy parser that matches the string "b"
static mpc_parser_t *create_dummy_b(void) {
  mpc_parser_t *p = mpc_new("dummy_b");
  mpc_define(p, mpc_string("b"));
  return p;
}

// Create dummy parser that matches a digit
static mpc_parser_t *create_dummy_digit(void) {
  mpc_parser_t *p = mpc_new("dummy_digit");
  mpc_define(p, mpc_digit());
  return p;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  char *input = (char*)malloc(size + 1);
  if (!input) return 0;
  memcpy(input, data, size);
  input[size] = '\0';

  mpc_parser_t *dummy_a = create_dummy_a();
  mpc_parser_t *dummy_b = create_dummy_b();
  mpc_parser_t *dummy_digit = create_dummy_digit();

  mpc_err_t *err = mpca_lang(MPCA_LANG_PREDICTIVE, input,
                            dummy_a,
                            dummy_b,
                            dummy_digit,
                            NULL);

  if (err) {
    // Optionally print error (commented out to save fuzzer time)
    // mpc_err_print(err);
    mpc_err_delete(err);
  }

  mpc_cleanup(3, dummy_a, dummy_b, dummy_digit);
  free(input);
  return 0;
}