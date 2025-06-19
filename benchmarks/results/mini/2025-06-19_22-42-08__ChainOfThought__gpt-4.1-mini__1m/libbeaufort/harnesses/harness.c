#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <beaufort.h>

// Include implementation to avoid linker errors
#include "encrypt.c"
#include "decrypt.c"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < 2) {
    // Need at least two bytes to have source and key
    return 0;
  }

  // Find a split point to separate source and key
  size_t split = size / 2;

  // Allocate null-terminated strings for source and key
  char *src = (char *)malloc(split + 1);
  char *key = (char *)malloc((size - split) + 1);
  if (!src || !key) {
    free(src);
    free(key);
    return 0;
  }

  // Copy data to src and key, ensuring null termination
  memcpy(src, data, split);
  src[split] = '\0';

  memcpy(key, data + split, size - split);
  key[size - split] = '\0';

  // Encrypt with default tableau (NULL)
  char *enc = beaufort_encrypt(src, key, NULL);
  if (enc != NULL) {
    // Decrypt with default tableau
    char *dec = beaufort_decrypt(enc, key, NULL);
    if (dec != NULL) {
      free(dec);
    }
    free(enc);
  }
  free(src);
  free(key);

  return 0;
}