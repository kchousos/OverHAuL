#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <beaufort.h>
#include "tableau.c"
#include "encrypt.c"
#ifdef __cplusplus
}
#endif

// The fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < 2) {
    // Need at least one byte for input and one for key
    return 0;
  }

  // Split input roughly in half to create src and key strings
  size_t src_len = size / 2;
  size_t key_len = size - src_len;

  // Allocate and copy src string, ensure null-terminated
  char *src = (char *)malloc(src_len + 1);
  if (!src) return 0;
  memcpy(src, data, src_len);
  src[src_len] = '\0';

  // Allocate and copy key string, ensure null-terminated
  char *key = (char *)malloc(key_len + 1);
  if (!key) {
    free(src);
    return 0;
  }
  memcpy(key, data + src_len, key_len);
  key[key_len] = '\0';

  // Call beaufort_encrypt with NULL tableau to let it create default tableau
  char *enc = beaufort_encrypt(src, key, NULL);

  // If encryption succeeded, free the encrypted string
  if (enc) free(enc);

  free(src);
  free(key);

  return 0;
}