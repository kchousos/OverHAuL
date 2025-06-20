#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <beaufort.h>

static void free_tableau(char **tableau) {
  if (!tableau)
    return;
  for (size_t i = 0; tableau[i] != NULL; i++) {
    free(tableau[i]);
  }
  free(tableau);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < 2)
    return 0;

  // Use first byte to decide key length (min 1, max 32)
  size_t key_len = (data[0] % 32) + 1;

  // If remaining size is too small for key + plaintext, skip
  if (size < 1 + key_len + 1)
    return 0;

  // Extract key and source strings from input; both may not be null-terminated
  char *key = malloc(key_len + 1);
  if (!key)
    return 0;
  memcpy(key, data + 1, key_len);
  key[key_len] = '\0';

  size_t src_len = size - 1 - key_len;
  char *src = malloc(src_len + 1);
  if (!src) {
    free(key);
    return 0;
  }
  memcpy(src, data + 1 + key_len, src_len);
  src[src_len] = '\0';

  // Build tableau with default alphabet
  char **tableau = beaufort_tableau(BEAUFORT_ALPHA);
  if (!tableau) {
    free(key);
    free(src);
    return 0;
  }

  // Encrypt
  char *enc = beaufort_encrypt(src, key, tableau);
  if (!enc) {
    free(key);
    free(src);
    free_tableau(tableau);
    return 0;
  }

  // Decrypt
  char *dec = beaufort_decrypt(enc, key, tableau);
  if (dec) {
    // Check if decrypted matches original source
    if (strcmp(src, dec) != 0) {
      // Deliberately crash to catch mismatch
      __builtin_trap();
    }
    free(dec);
  }

  // Cleanup
  free(enc);
  free(key);
  free(src);
  free_tableau(tableau);

  return 0;
}