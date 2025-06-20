#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <beaufort.h>

// Include implementation sources to resolve linker errors
#include "tableau.c"
#include "encrypt.c"
#include "decrypt.c"

static size_t
min_size(size_t a, size_t b) {
  return a < b ? a : b;
}

/**
 * Extract a null-terminated string from fuzz input data safely.
 * Copies up to max_len chars or until input length exhausted.
 */
static char *
extract_string(const uint8_t *data, size_t size, size_t max_len) {
  if (size == 0) {
    // Allocate empty string
    char *str = malloc(1);
    if (str) str[0] = '\0';
    return str;
  }
  size_t len = min_size(size, max_len);
  char *str = malloc(len + 1);
  if (!str) return NULL;
  memcpy(str, data, len);
  str[len] = '\0';
  return str;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (!data || size == 0) return 0;

  // Partition input into plaintext, key, alphabet segments
  // Minimum key length = 1 to avoid division/mod by zero
  // Minimum plaintext length = 1
  // Alphabet optional, can be empty string or null

  size_t min_key_len = 1;
  size_t min_plain_len = 1;
  size_t max_alphabet_len = 64; // arbitrary limit for alphabet size

  if (size < min_key_len + min_plain_len) return 0;

  // Simple split:
  // plaintext: first third or half
  // key: next segment min_key_len
  // alphabet: rest or NULL

  size_t plain_len = size / 2;
  if (plain_len < min_plain_len) plain_len = min_plain_len;
  if (plain_len > size - min_key_len) plain_len = size - min_key_len;

  size_t key_len = min_key_len;
  size_t alphabet_len = size - plain_len - key_len;

  const uint8_t *plain_data = data;
  const uint8_t *key_data = data + plain_len;
  const uint8_t *alpha_data = data + plain_len + key_len;

  // Extract null-terminated strings
  char *plaintext = extract_string(plain_data, plain_len, plain_len);
  char *key = extract_string(key_data, key_len, key_len);
  if (!plaintext || !key) {
    free(plaintext);
    free(key);
    return 0;
  }

  char *alphabet = NULL;
  if (alphabet_len > 0 && alphabet_len <= max_alphabet_len) {
    alphabet = extract_string(alpha_data, alphabet_len, alphabet_len);
  }

  // Create tableau if alphabet provided, else NULL for default alpha
  char **tableau = NULL;
  if (alphabet && alphabet[0] != '\0') {
    tableau = beaufort_tableau(alphabet);
    // If allocation failed, fallback to NULL
  }

  // Encrypt plaintext
  char *encrypted = beaufort_encrypt(plaintext, key, tableau);
  if (encrypted) {
    // Decrypt ciphertext
    char *decrypted = beaufort_decrypt(encrypted, key, tableau);
    // Optionally could check decrypted == plaintext for printable keys, but
    // the fuzzer may generate arbitrary data so just free and continue

    free(decrypted);
    free(encrypted);
  }

  // Free resources
  if (tableau) {
    // Free tableau array of strings
    for (size_t i = 0; tableau[i] != NULL; i++) {
      free(tableau[i]);
    }
    free(tableau);
  }

  free(plaintext);
  free(key);
  free(alphabet);

  return 0;
}