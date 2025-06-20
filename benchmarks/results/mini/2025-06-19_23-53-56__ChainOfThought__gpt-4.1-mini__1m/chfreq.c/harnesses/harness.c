#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// BEGIN embedded chfreq implementation

#include <stdio.h>

static size_t
ssize (const char *str) {
  size_t size = 0;
  while ('\0' != str[size]) ++size;
  return size;
}

static int
find (uint32_t **mat, const char ch) {
  int idx = 0;
  uint32_t *cur = NULL;
  while ((cur = mat[idx])) {
    if (ch == (char) cur[0]) {
      return idx;
    } else { idx++; }
  }
  return -1;
}

uint32_t **
chfreq (const char *src) {
  uint32_t **mat = NULL;
  char ch = 0;
  size_t size = 1;
  int pos = 0;
  int i = 0;
  int idx = -1;

  // alloc
  mat = (uint32_t **) calloc(size, sizeof(uint32_t *));
  if (NULL == mat) { return NULL; }

  // build
  while ('\0' != (ch = src[i++])) {
    idx = find(mat, ch);
    if (-1 == idx) {
      idx = pos++;
      mat = (uint32_t **) realloc(mat, sizeof(uint32_t *) * (ssize(src) + 1));
      mat[idx] = (uint32_t *) calloc(2, sizeof(uint32_t));
      mat[idx][0] = ch;
      mat[idx][1] = 1;
      size++;
    } else {
      mat[idx][1]++;
    }
  }

  mat[pos] = NULL;

  return mat;
}

// END embedded chfreq implementation


int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Defensive: Early exit on empty input (no chars to count)
    if (size == 0) return 0;

    // Allocate buffer with space for terminating null byte
    char *buf = (char *)malloc(size + 1);
    if (!buf) return 0;

    // Copy input and null-terminate
    memcpy(buf, data, size);
    buf[size] = '\0';

    // Call chfreq with null-terminated string
    uint32_t **freq = chfreq(buf);

    if (freq) {
        uint64_t total_count = 0;

        // Iterate through freq until NULL pointer is found
        for (size_t i = 0; freq[i] != NULL; i++) {
            uint32_t ch = freq[i][0];
            uint32_t count = freq[i][1];

            // The frequency count should never be zero
            if (count == 0) {
                // Abort early: invalid frequency count
                free(buf);
                // Free freq matrix
                size_t idx = 0;
                while (freq[idx] != NULL) {
                    free(freq[idx]);
                    idx++;
                }
                free(freq);
                assert(0 && "Frequency count zero detected");
            }

            // Validate that character appears in input buffer
            // This check helps detect corruption or invalid memory
            int found_char = 0;
            for (size_t j = 0; j < size; j++) {
                if ((char)ch == buf[j]) {
                    found_char = 1;
                    break;
                }
            }
            if (!found_char) {
                free(buf);
                size_t idx = 0;
                while (freq[idx] != NULL) {
                    free(freq[idx]);
                    idx++;
                }
                free(freq);
                assert(0 && "Frequency character not found in input");
            }

            total_count += count;
        }

        // The sum of all frequency counts must equal input size
        if (total_count != size) {
            free(buf);
            size_t idx = 0;
            while (freq[idx] != NULL) {
                free(freq[idx]);
                idx++;
            }
            free(freq);
            assert(0 && "Sum of all frequencies does not equal input size");
        }

        // Free allocated freq matrix
        size_t idx = 0;
        while (freq[idx] != NULL) {
            free(freq[idx]);
            idx++;
        }
        free(freq);
    }

    free(buf);

    return 0;
}