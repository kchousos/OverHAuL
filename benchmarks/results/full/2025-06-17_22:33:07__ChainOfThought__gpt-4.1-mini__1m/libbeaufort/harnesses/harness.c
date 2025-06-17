#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <beaufort.h>

extern char **beaufort_tableau(const char *);
extern char *beaufort_encrypt(const char *, const char *, char **);
extern char *beaufort_decrypt(const char *, const char *, char **);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < 2) {
    // Need at least one char for key and one for plaintext to exercise.
    return 0;
  }

  // We split data roughly in half for key and plaintext.
  size_t half = size / 2;
  // Allocate buffers for key and plaintext + null terminator
  char *key = (char *)malloc(half + 1);
  char *plaintext = (char *)malloc(size - half + 1);
  if (!key || !plaintext) {
    free(key);
    free(plaintext);
    return 0;
  }

  // Copy and null terminate
  memcpy(key, data, half);
  key[half] = '\0';
  memcpy(plaintext, data + half, size - half);
  plaintext[size - half] = '\0';

  // Generate tableau once with default alphabet
  char **tableau = beaufort_tableau(BEAUFORT_ALPHA);
  if (!tableau) {
    // Clean up and exit gracefully
    free(key);
    free(plaintext);
    return 0;
  }

  // Encrypt
  char *encrypted = beaufort_encrypt(plaintext, key, tableau);
  if (encrypted) {
    // Decrypt
    char *decrypted = beaufort_decrypt(encrypted, key, tableau);
    if (decrypted) {
      // Optionally compare results, but not needed for fuzz harness
      free(decrypted);
    }
    free(encrypted);
  }

  // Free tableau
  for (size_t i = 0; tableau[i] != NULL; i++) {
    free(tableau[i]);
  }
  free(tableau);

  free(key);
  free(plaintext);
  return 0;
}