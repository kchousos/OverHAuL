// SPDX-License-Identifier: MIT

#pragma once

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)
#  define SIXPACK_NODISCARD [[nodiscard]]
#elif defined(__GNUC__)
#  define SIXPACK_NODISCARD __attribute__((warn_unused_result))
#else
#  define SIXPACK_NODISCARD
#endif

#if !(defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L))
#  include <stdbool.h>
#endif

#ifndef SIXPACK_API
#  ifdef __GNUC__
#    define SIXPACK_API __attribute__((visibility("default"))) extern
#  else
#    define SIXPACK_API extern
#  endif
#endif // !SIXPACK_API

enum {
	SIXPACK_IO_EOF = -1,
	SIXPACK_IO_ERROR = -2,
};

enum sixpack_type {
	SIXPACK_INTEGER,
	SIXPACK_FLOAT,
	SIXPACK_BOOL,
	SIXPACK_STRING,
	SIXPACK_LIST,
	SIXPACK_DICT,
};

#define SIXPACK_CONTINUE NULL

struct sixpack {
	void *userdata;

	int   (*getchar)(struct sixpack*);
	void* (*process)(struct sixpack*, enum sixpack_type, const char *data, unsigned data_size);

	const char *error_message;
	unsigned    error_line;
	unsigned    error_column;
};

SIXPACK_API const char* SIXPACK_BEGIN;
SIXPACK_API const char* SIXPACK_END;

SIXPACK_API
int sixpack_stdio_getchar(struct sixpack*);

SIXPACK_NODISCARD SIXPACK_API
bool sixpack_parse(struct sixpack*);
