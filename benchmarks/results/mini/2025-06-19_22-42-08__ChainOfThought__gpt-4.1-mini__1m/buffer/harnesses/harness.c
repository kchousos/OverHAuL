#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

// The buffer_t struct and functions declarations
typedef struct {
  size_t len;
  char *alloc;
  char *data;
} buffer_t;

buffer_t *buffer_new();
buffer_t *buffer_new_with_size(size_t n);
buffer_t *buffer_new_with_string(char *str);
buffer_t *buffer_new_with_string_length(char *str, size_t len);
buffer_t *buffer_new_with_copy(char *str);
size_t buffer_size(buffer_t *self);
size_t buffer_length(buffer_t *self);
void buffer_free(buffer_t *self);
int buffer_prepend(buffer_t *self, char *str);
int buffer_append(buffer_t *self, const char *str);
int buffer_appendf(buffer_t *self, const char *format, ...);
int buffer_append_n(buffer_t *self, const char *str, size_t len);
int buffer_equals(buffer_t *self, buffer_t *other);
ssize_t buffer_indexof(buffer_t *self, char *str);
buffer_t *buffer_slice(buffer_t *self, size_t from, ssize_t to);
ssize_t buffer_compact(buffer_t *self);
void buffer_fill(buffer_t *self, int c);
void buffer_clear(buffer_t *self);
void buffer_trim_left(buffer_t *self);
void buffer_trim_right(buffer_t *self);
void buffer_trim(buffer_t *self);
void buffer_print(buffer_t *self);

/*
 * Compute the nearest multiple of `a` from `b`.
 */

#define nearest_multiple_of(a, b) \
  (((b) + ((a) - 1)) & ~((a) - 1))

buffer_t *
buffer_new() {
  return buffer_new_with_size(64);
}

buffer_t *
buffer_new_with_size(size_t n) {
  buffer_t *self = malloc(sizeof(buffer_t));
  if (!self) return NULL;
  self->len = n;
  self->data = self->alloc = calloc(n + 1, 1);
  return self;
}

buffer_t *
buffer_new_with_string(char *str) {
  return buffer_new_with_string_length(str, strlen(str));
}

buffer_t *
buffer_new_with_string_length(char *str, size_t len) {
  buffer_t *self = malloc(sizeof(buffer_t));
  if (!self) return NULL;
  self->len = len;
  self->data = self->alloc = str;
  return self;
}

buffer_t *
buffer_new_with_copy(char *str) {
  size_t len = strlen(str);
  buffer_t *self = buffer_new_with_size(len);
  if (!self) return NULL;
  memcpy(self->alloc, str, len);
  self->data = self->alloc;
  return self;
}

ssize_t
buffer_compact(buffer_t *self) {
  size_t len = buffer_length(self);
  size_t rem = self->len - len;
  char *buf = calloc(len + 1, 1);
  if (!buf) return -1;
  memcpy(buf, self->data, len);
  free(self->alloc);
  self->len = len;
  self->data = self->alloc = buf;
  return rem;
}

void
buffer_free(buffer_t *self) {
  free(self->alloc);
  free(self);
}

size_t
buffer_size(buffer_t *self) {
  return self->len;
}

size_t
buffer_length(buffer_t *self) {
  return strlen(self->data);
}

int
buffer_resize(buffer_t *self, size_t n) {
  n = nearest_multiple_of(1024, n);
  char *new_alloc = realloc(self->alloc, n + 1);
  if (!new_alloc) return -1;
  self->alloc = new_alloc;
  self->len = n;
  self->data = self->alloc;
  self->alloc[n] = '\0';
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

  // First, we compute how many bytes are needed
  // for the formatted string and allocate that
  // much more space in the buffer.
  va_copy(tmpa, ap);
  required = vsnprintf(NULL, 0, format, tmpa);
  va_end(tmpa);
  if (-1 == buffer_resize(self, length + required)) {
    va_end(ap);
    return -1;
  }

  // Next format the string into the space that we
  // have made room for.
  dst = self->data + length;
  bytes = vsnprintf(dst, 1 + required, format, ap);
  va_end(ap);

  return bytes < 0
    ? -1
    : 0;
}

int
buffer_append(buffer_t *self, const char *str) {
  return buffer_append_n(self, str, strlen(str));
}

int
buffer_append_n(buffer_t *self, const char *str, size_t len) {
  size_t prev = strlen(self->data);
  size_t needed = len + prev;

  // enough space
  if (self->len > needed) {
    strncat(self->data, str, len);
    return 0;
  }

  // resize
  int ret = buffer_resize(self, needed);
  if (-1 == ret) return -1;
  strncat(self->data, str, len);

  return 0;
}

int
buffer_prepend(buffer_t *self, char *str) {
  size_t len = strlen(str);
  size_t prev = strlen(self->data);
  size_t needed = len + prev;

  // enough space
  if (self->len > needed) goto move;

  // resize
  int ret = buffer_resize(self, needed);
  if (-1 == ret) return -1;

  // move
  move:
  memmove(self->data + len, self->data, prev + 1);
  memcpy(self->data, str, len);

  return 0;
}

buffer_t *
buffer_slice(buffer_t *buf, size_t from, ssize_t to) {
  size_t len = strlen(buf->data);

  // bad range
  if (to < from) return NULL;

  // relative to end
  if (to < 0) to = len - ~to;

  // cap end
  if (to > len) to = len;

  size_t n = to - from;
  buffer_t *self = buffer_new_with_size(n);
  if (!self) return NULL;
  memcpy(self->data, buf->data + from, n);
  self->data[n] = '\0';
  return self;
}

int
buffer_equals(buffer_t *self, buffer_t *other) {
  return 0 == strcmp(self->data, other->data);
}

ssize_t
buffer_indexof(buffer_t *self, char *str) {
  char *sub = strstr(self->data, str);
  if (!sub) return -1;
  return sub - self->data;
}

void
buffer_trim_left(buffer_t *self) {
  int c;
  while ((c = *self->data) && isspace(c)) {
    ++self->data;
  }
}

void
buffer_trim_right(buffer_t *self) {
  int c;
  size_t len = buffer_length(self);
  if (len == 0) return;
  size_t i = len - 1;
  while (i != (size_t)(-1) && (c = self->data[i]) && isspace(c)) {
    self->data[i--] = 0;
  }
}

void
buffer_trim(buffer_t *self) {
  buffer_trim_left(self);
  buffer_trim_right(self);
}

void
buffer_fill(buffer_t *self, int c) {
  memset(self->data, c, self->len);
}

void
buffer_clear(buffer_t *self) {
  buffer_fill(self, 0);
}

void
buffer_print(buffer_t *self) {
  int i;
  size_t len = self->len;

  printf("\n ");

  // hex
  for (i = 0; i < (int)len; ++i) {
    printf(" %02x", (unsigned char)self->alloc[i]);
    if ((i + 1) % 8 == 0) printf("\n ");
  }

  printf("\n");
}

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size == 0) return 0;

    // Create a buffer with some initial size, at least 64 or input size capped
    size_t initial_size = Size < 64 ? 64 : Size;
    buffer_t *buf = buffer_new_with_size(initial_size);
    if (!buf) return 0;

    // We will split the input into multiple parts for different calls
    // To avoid out-of-bounds, we compute offsets carefully

    // Use up to first half of data for append_n
    size_t half = Size / 2;
    if (half > 0) {
      // use half or fewer bytes to append_n
      size_t len = half;
      buffer_append_n(buf, (const char*)Data, len);
    }

    // Prepend some bytes, for fuzz let's use the last quarter of input if any
    if (Size >= 4) {
      size_t quarter = Size / 4;
      if (quarter > 0) {
        const char *prepend_str = (const char*)(Data + Size - quarter);
        buffer_prepend(buf, (char*)prepend_str);
      }
    }

    // Appendf - use a fixed format, inserting a controlled string from part of input interpreted as a string
    char fmt_buf[128];
    size_t fmt_len = Size < sizeof(fmt_buf)-2 ? Size : sizeof(fmt_buf)-2;
    if (fmt_len > 0) {
      memcpy(fmt_buf, Data, fmt_len);
      fmt_buf[fmt_len] = '\0';
      buffer_appendf(buf, "%s", fmt_buf);
    }

    // Call slice with random from/to inside buffer length
    size_t buf_len = buffer_length(buf);
    if (buf_len > 0) {
      size_t from = Data[0] % buf_len;
      ssize_t to = (buf_len > 1) ? (size_t)(Data[1] % buf_len) : (ssize_t)buf_len;
      if (to < 0) to = buf_len;
      if (to < from) to = from;

      buffer_t *slice = buffer_slice(buf, from, to);
      if (slice) {
        int eq = buffer_equals(buf, slice);
        (void)eq;
        buffer_free(slice);
      }
    }

    // Call trimming functions
    buffer_trim_left(buf);
    buffer_trim_right(buf);
    buffer_trim(buf);

    // Call compact
    buffer_compact(buf);

    // Fill buffer with some byte from input or 0
    int fill_char = Size > 0 ? Data[0] : 0;
    buffer_fill(buf, fill_char);

    // Clear buffer
    buffer_clear(buf);

    // Print the buffer
    buffer_print(buf);

    buffer_free(buf);

    return 0;
}