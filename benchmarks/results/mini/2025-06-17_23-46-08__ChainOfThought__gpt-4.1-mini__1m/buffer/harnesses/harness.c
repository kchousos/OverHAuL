#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>

// Buffer structure and functions based on provided source

#define BUFFER_DEFAULT_SIZE 64

typedef struct {
  size_t len;
  char *alloc;
  char *data;
} buffer_t;

#define nearest_multiple_of(a, b) \
  (((b) + ((a) - 1)) & ~((a) - 1))

buffer_t *
buffer_new_with_size(size_t n) {
  buffer_t *self = malloc(sizeof(buffer_t));
  if (!self) return NULL;
  self->len = n;
  self->data = self->alloc = calloc(n + 1, 1);
  return self;
}

buffer_t *
buffer_new() {
  return buffer_new_with_size(BUFFER_DEFAULT_SIZE);
}

size_t
buffer_length(buffer_t *self) {
  return strlen(self->data);
}

int
buffer_resize(buffer_t *self, size_t n) {
  n = nearest_multiple_of(1024, n);
  self->len = n;
  char *tmp = realloc(self->alloc, n + 1);
  if (!tmp) return -1;
  self->alloc = tmp;
  self->data = tmp;
  self->alloc[n] = '\0';
  return 0;
}

int
buffer_append_n(buffer_t *self, const char *str, size_t len) {
  size_t prev = strlen(self->data);
  size_t needed = len + prev;

  if (self->len > needed) {
    strncat(self->data, str, len);
    return 0;
  }

  int ret = buffer_resize(self, needed);
  if (-1 == ret) return -1;
  strncat(self->data, str, len);

  return 0;
}

int buffer_appendf(buffer_t *self, const char *format, ...) {
  va_list ap;
  va_list tmpa;
  char *dst = NULL;
  int length = 0;
  int required = 0;
  int bytes = 0;

  va_start(ap, format);

  length = buffer_length(self);

  va_copy(tmpa, ap);
  required = vsnprintf(NULL, 0, format, tmpa);
  va_end(tmpa);
  if (-1 == buffer_resize(self, length + required)) {
    va_end(ap);
    return -1;
  }

  dst = self->data + length;
  bytes = vsnprintf(dst, 1 + required, format, ap);
  va_end(ap);

  return bytes < 0 ? -1 : 0;
}

int
buffer_prepend(buffer_t *self, char *str) {
  size_t len = strlen(str);
  size_t prev = strlen(self->data);
  size_t needed = len + prev;

  if (self->len > needed) goto move;

  int ret = buffer_resize(self, needed);
  if (-1 == ret) return -1;

move:
  memmove(self->data + len, self->data, prev + 1);
  memcpy(self->data, str, len);

  return 0;
}

void
buffer_free(buffer_t *self) {
  free(self->alloc);
  free(self);
}

// Fuzzer entry point

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size == 0)
    return 0;

  // Create buffer
  buffer_t *buf = buffer_new();
  if (!buf)
    return 0;

  // Append fuzz data as-is
  buffer_append_n(buf, (const char *)data, size);

  // Additionally, try to prepend if size allows
  if (size > 5) {
    // Use part of data as string to prepend
    // To avoid issues with non-null-terminated data, copy to a temporary buffer
    size_t prepend_len = size / 2;
    char *tmp = (char *)malloc(prepend_len + 1);
    if (tmp) {
      memcpy(tmp, data, prepend_len);
      tmp[prepend_len] = '\0'; // null terminate to avoid crashes in strlen
      buffer_prepend(buf, tmp);
      free(tmp);
    }
  }

  // Attempt a formatted append using fixed format to cover buffer_appendf code
  buffer_appendf(buf, "FuzzTesting:%zu", size);

  buffer_free(buf);

  return 0;
}