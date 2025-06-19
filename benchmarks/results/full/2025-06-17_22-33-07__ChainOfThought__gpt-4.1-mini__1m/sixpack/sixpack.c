// SPDX-License-Identifier: MIT

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE < 2)
#undef _POSIX_C_SOURCE
#endif

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 2  // Needed for getopt().
#endif

#include "sixpack.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)
#  define FALLTHROUGH [[fallthrough]]
#else
#  define FALLTHROUGH
#endif

#define NOOP ((void) 0)

#ifdef __GNUC__
#  define likely(x)   __builtin_expect(!!(x), 1)
#  define unlikely(x) __builtin_expect(!!(x), 0)
#else
#  define likely(x)   (x)
#  define unlikely(x) (x)
#endif

#ifdef SIXPACK_STATIC
#  ifndef SIXPACK_STATIC_BUFFER_SIZE
#    define SIXPACK_STATIC_BUFFER_SIZE 255
#  endif
#  define CBUF_ALLOC(cbuf) SIXPACK_STATIC_BUFFER_SIZE
#else
#  define CBUF_ALLOC(cbuf) ((cbuf)->alloc)
#endif

struct cbuf {
	size_t size;
#ifdef SIXPACK_STATIC
	char data[SIXPACK_STATIC_BUFFER_SIZE + 1];
#else
	char  *data;
	size_t alloc;
#endif
};

static inline bool
cbuf_push(struct cbuf *b, int ch)
{
	if (b->size >= CBUF_ALLOC(b)) {
#if SIXPACK_STATIC
		return false;
#else
		b->alloc = b->alloc ? (b->alloc << 1) : 32;
		void *data = b->data ? realloc(b->data, b->alloc + 1) : malloc(b->alloc + 1);
		if (!data)
			return false;
		b->data = data;
#endif
	}

	b->data[b->size++] = ch;
	b->data[b->size] = '\0';
	return true;
}

static inline void
cbuf_free(struct cbuf *b)
{
	b->size = 0;

#ifdef SIXPACK_STATIC
	b->data[0] = '\0';
#else
	free(b->data);
	b->data = NULL;
	b->alloc = 0;
#endif
}

enum status {
	STATUS_OK = 0,
	STATUS_EOF,
	STATUS_ERROR,
	STATUS_IO_ERROR,
	STATUS_OOM_ERROR,
	STATUS_STOPPED,
};

struct parser {
	struct sixpack *sp;
	int look;
	unsigned line, column;
	struct cbuf buffer;
};

#define P struct parser *p
#define S enum status *status
#define R SIXPACK_NODISCARD static bool

#define CHECK_OK status); \
	if (*status != STATUS_OK) goto error; \
	((void) 0
#define DUMMY
#undef DUMMY

#define ERROR(st, msg) do {           \
	if (*status == STATUS_OK)         \
		*status = (st);               \
	if (!p->sp->error_message)        \
		p->sp->error_message = (msg); \
	} while (false)

static inline void
buffer_push(struct parser *p, S)
{
	if (unlikely(!cbuf_push(&p->buffer, p->look)))
		ERROR(STATUS_OOM_ERROR, "out of memory");
}

static inline void
buffer_clear(struct parser *p)
{
	p->buffer.size = 0;
}

static inline void
buffer_free(struct parser *p)
{
	cbuf_free(&p->buffer);
}

static inline bool
buffer_is_empty(const struct parser *p)
{
	return p->buffer.size == 0;
}

const char *SIXPACK_BEGIN = "<";
const char *SIXPACK_END = ">";

SIXPACK_NODISCARD static inline bool
is_whitespace_char(int ch)
{
	switch (ch) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
		return true;
	default:
		return false;
	}
}

SIXPACK_NODISCARD static inline bool
is_key_char(int ch)
{
	switch (ch) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':
	case '[': case ']':
	case '{': case '}':
	case ':':
	case ',':
		return false;
	default:
		return true;
	}
}

SIXPACK_NODISCARD static inline bool
is_number_char(int ch)
{
	switch (ch) {
	case '.':
	case '+':
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'a': case 'A':
	case 'b': case 'B':
	case 'c': case 'C':
	case 'd': case 'D':
	case 'e': case 'E':
	case 'f': case 'F':
		return true;
	default:
		return false;
	}
}

SIXPACK_NODISCARD static inline bool
is_octal_nonzero_char(int ch)
{
	return (ch >= '0') && (ch < '8');
}

SIXPACK_NODISCARD static inline bool
is_hex_char(int ch)
{
    return ((ch >= '0' && ch <= '9') ||
            (ch >= 'A' && ch <= 'F') ||
            (ch >= 'a' && ch <= 'f'));
}

SIXPACK_NODISCARD static inline int
hex_char_to_int(int ch)
{
    switch (ch) {
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case 'a': case 'A': return 0xA;
	case 'b': case 'B': return 0xB;
	case 'c': case 'C': return 0xC;
	case 'd': case 'D': return 0xD;
	case 'e': case 'E': return 0xE;
	case 'f': case 'F': return 0xF;
	default: assert(!"not an hexadecimal digit"); abort();
    }
}

SIXPACK_NODISCARD static inline int
nextchar_raw(P, S)
{
	int ch = (*p->sp->getchar)(p->sp);
	switch (ch) {
	case SIXPACK_IO_ERROR:
		ERROR(STATUS_IO_ERROR, NULL);
		FALLTHROUGH;
	case SIXPACK_IO_EOF:
		break;
	case '\n':
		p->column = 0;
		p->line++;
		FALLTHROUGH;
	default:
		p->column++;
	}
	return ch;
}

static inline void
nextchar(P, S)
{
	do {
		p->look = nextchar_raw(p, CHECK_OK);
		if (p->look == '#') {
			while (p->look != '\n' && p->look != SIXPACK_IO_EOF) {
				p->look = nextchar_raw(p, CHECK_OK);
			}
		}
	} while (p->look != SIXPACK_IO_EOF && p->look == '#');

error: NOOP;
}

static inline void
skipwhite(P, S)
{
	while (p->look != SIXPACK_IO_EOF && is_whitespace_char(p->look))
		nextchar(p, status);
}

static inline void
matchchar(P, int ch, const char *errmsg, S)
{
	if (p->look == ch) {
		nextchar(p, CHECK_OK);
	} else {
		ERROR(STATUS_ERROR, errmsg);
	}

error: NOOP;
}

static inline void
process(P, enum sixpack_type type, const char *data, unsigned data_size, S)
{
	if (likely(p->sp->process)) {
		void *result = (*p->sp->process)(p->sp, type, data, data_size);
		if (result != SIXPACK_CONTINUE)
			ERROR(STATUS_STOPPED, result);
	}
}

static void
parse_bool(P, S)
{
	if (p->look == 'T' || p->look == 't') {
		nextchar(p, CHECK_OK);
		matchchar(p, 'r', NULL, CHECK_OK);
		matchchar(p, 'u', NULL, CHECK_OK);
		matchchar(p, 'e', NULL, CHECK_OK);
		process(p, SIXPACK_BOOL, "true", 1, CHECK_OK);
		return;
	} else if (p->look == 'F' || p->look == 'f') {
		nextchar(p, CHECK_OK);
		matchchar(p, 'a', NULL, CHECK_OK);
		matchchar(p, 'l', NULL, CHECK_OK);
		matchchar(p, 's', NULL, CHECK_OK);
		matchchar(p, 'e', NULL, CHECK_OK);
		process(p, SIXPACK_BOOL, "false", 0, CHECK_OK);
		return;
	}

error:
	ERROR(STATUS_ERROR, "invalid boolean value");
}

static void
parse_number(P, S)
{
	assert(buffer_is_empty(p));

	if (p->look == '-' || p->look == '+') {
		buffer_push(p, CHECK_OK);
		nextchar(p, CHECK_OK);
	}

	// Octal/hex numbers.
	bool is_octal = false;
	bool is_hex = false;
	if (p->look == '0') {
		buffer_push(p, CHECK_OK);
		nextchar(p, CHECK_OK);
		if (p->look == 'x' || p->look == 'X') {
			buffer_push(p, CHECK_OK);
			nextchar(p, CHECK_OK);
			is_hex = true;
		} else if (is_octal_nonzero_char(p->look)) {
			is_octal = true;
		}
	}

	// Read the rest of the number.
	bool dot_seen = false;
	bool exp_seen = false;
	while (p->look != SIXPACK_IO_EOF && is_number_char(p->look)) {
		if (!is_hex && (p->look == 'e' || p->look == 'E')) {
			if (exp_seen || is_octal)
				goto error;
			exp_seen = true;
			buffer_push(p, CHECK_OK);
			nextchar(p, CHECK_OK);

			// Optional sign of the exponent.
			if (p->look == '-'  || p->look == '+') {
				buffer_push(p, CHECK_OK);
				nextchar(p, CHECK_OK);
			}
		} else {
			if (p->look == '.') {
				if (dot_seen || is_hex || is_octal)
					goto error;
				dot_seen = true;
			}
			if (p->look == '-' || p->look == '+')
				goto error;
			buffer_push(p, CHECK_OK);
			nextchar(p, CHECK_OK);
		}
	}

	if (!p->buffer.size)
		goto error;

	if (is_hex || is_octal) {
		process(p, SIXPACK_INTEGER, p->buffer.data, p->buffer.size, CHECK_OK);
	} else if (dot_seen || exp_seen) {
		process(p, SIXPACK_FLOAT, p->buffer.data, p->buffer.size, CHECK_OK);
	} else {
		process(p, SIXPACK_INTEGER, p->buffer.data, p->buffer.size, CHECK_OK);
	}
	goto out;

error:
	ERROR(STATUS_ERROR, "invalid numeric value");
out:
	buffer_clear(p);
}

static inline void
parse_key(P, S)
{
	assert(buffer_is_empty(p));

	while (p->look != SIXPACK_IO_EOF && is_key_char(p->look)) {
		buffer_push(p, CHECK_OK);
		nextchar(p, CHECK_OK);
	}
	
	if (buffer_is_empty(p))
		ERROR(STATUS_ERROR, "missing dictionary key");
	else
		return;

error:
	ERROR(STATUS_ERROR, NULL);
}

static void
parse_string(P, S)
{
	assert(buffer_is_empty(p));

	matchchar(p, '"', NULL, CHECK_OK);

	while (p->look != '"' && p->look != SIXPACK_IO_EOF) {
		// Handle escapes.
		if (p->look == '\\') {
			int extra;
			p->look = nextchar_raw(p, CHECK_OK);
			switch (p->look) {
			case '"' : p->look = '"' ; break;
			case 'n' : p->look = '\n'; break;
			case 'r' : p->look = '\r'; break;
			case 't' : p->look = '\t'; break;
			case '\\': p->look = '\\'; break;
			default:
				// Hex number escape sequence.
				extra = nextchar_raw(p, CHECK_OK);
				if (!is_hex_char(extra) || !is_hex_char(p->look)) {
					ERROR(STATUS_ERROR, "invalid escape sequence");
					goto error;
				}
				p->look = (hex_char_to_int(p->look) * 16) + hex_char_to_int(extra);
				break;
			}
		}
		buffer_push(p, CHECK_OK);
		p->look = nextchar_raw(p, CHECK_OK);
	}

	matchchar(p, '"', "unterminated string value", CHECK_OK);
	process(p, SIXPACK_STRING, p->buffer.data, p->buffer.size, CHECK_OK);

error:
	buffer_clear(p);
}

static void parse_value(P, S);

static void
parse_list(P, S)
{
	assert(buffer_is_empty(p));

	matchchar(p, '[', NULL, CHECK_OK);
	skipwhite(p, CHECK_OK);

	process(p, SIXPACK_LIST, SIXPACK_BEGIN, 0, CHECK_OK);

	while (p->look != ']') {
		parse_value(p, CHECK_OK);
		buffer_clear(p);

		bool got_whitespace = is_whitespace_char(p->look);
		skipwhite(p, CHECK_OK);

		// There must be either a comma or whitespace after the value.
		if (p->look == ',') {
			nextchar(p, CHECK_OK);	
		} else if (!got_whitespace && !is_whitespace_char(p->look)) {
			break;
		}
		skipwhite(p, CHECK_OK);
	}

	matchchar(p, ']', "unterminated list", CHECK_OK);

	process(p, SIXPACK_LIST, SIXPACK_END, 0, CHECK_OK);

error: NOOP;
}

static void parse_keyval_items(P, int eos, S);

static void
parse_dict(P, S)
{
	matchchar(p, '{', NULL, CHECK_OK);
	skipwhite(p, CHECK_OK);
	parse_keyval_items(p, '}', CHECK_OK);
	matchchar(p, '}', "unterminated dictionary value", CHECK_OK);
error: NOOP;
}

static void
parse_value(P, S)
{
	// TODO: Handle annotations.

	switch (p->look) {
	case '"':
		parse_string(p, CHECK_OK);
		break;
	case '[':
		parse_list(p, CHECK_OK);
		break;
	case '{':
		parse_dict(p, CHECK_OK);
		break;
	case 'T': case 't':
	case 'F': case 'f':
		parse_bool(p, CHECK_OK);
		break;
	default:
		parse_number(p, CHECK_OK);
		break;
	}

error:
	buffer_clear(p);
}

static void
parse_keyval_items(P, int eos, S)
{
	process(p, SIXPACK_DICT, SIXPACK_BEGIN, 0, CHECK_OK);

	while (p->look != eos) {
		parse_key(p, CHECK_OK);
		process(p, SIXPACK_STRING, p->buffer.data, p->buffer.size, CHECK_OK);
		buffer_clear(p);

		bool got_separator = false;
		if (is_whitespace_char(p->look)) {
			got_separator = true;
			skipwhite(p, CHECK_OK);
		} else switch (p->look) {
			case ':':
				nextchar(p, CHECK_OK);
				skipwhite(p, CHECK_OK);
				FALLTHROUGH;
			case '{':
			case '[':
				got_separator = true;
				break;
		}

		if (!got_separator) {
			ERROR(STATUS_ERROR, "missing separator");
			goto error;
		}

		parse_value(p, CHECK_OK);

		// There must be either a comma or a whitespace after the value,
		// or the end-of-sequence character.
		if (p->look ==  ',') {
			nextchar(p, CHECK_OK);
		} else if (p->look != eos && !is_whitespace_char(p->look)) {
			break;
		}
		skipwhite(p, CHECK_OK);
	}

	process(p, SIXPACK_DICT, SIXPACK_END, 0, CHECK_OK);

error:
	buffer_clear(p);
}

static void
parse_message(P, S)
{
	nextchar(p, CHECK_OK);
	skipwhite(p, CHECK_OK);

	if (p->look == SIXPACK_IO_ERROR) {
		ERROR(STATUS_IO_ERROR, NULL);
	} else if (p->look == '{') {
		// Input starts with a dict marker.
		nextchar(p, CHECK_OK);
		skipwhite(p, CHECK_OK);
		parse_keyval_items(p, '}', CHECK_OK);
		matchchar(p, '}', "unterminated message", CHECK_OK);
	} else {
		parse_keyval_items(p, SIXPACK_IO_EOF, CHECK_OK);
	}

error: NOOP;
}

int
sixpack_stdio_getchar(struct sixpack *sp)
{
	int ch = fgetc(sp->userdata);
	if (unlikely(ch == EOF)) {
		if (ferror(sp->userdata))
			return SIXPACK_IO_ERROR;
		assert(feof(sp->userdata));
		return SIXPACK_IO_EOF;
	}
	return ch;
}

bool
sixpack_parse(struct sixpack *sp)
{
	assert(sp);

	struct parser p = { .sp = sp, .line = 1 };
	enum status status = STATUS_OK;
	parse_message(&p, &status);
	buffer_free(&p);

	if (status == STATUS_OK)
		return true;

	sp->error_column = p.column;
	sp->error_line = p.line;
	return false;
}

#ifdef SIXPACK_MAIN
#include <errno.h>
#include <string.h>
#include <unistd.h>

struct sixpack_ext {
	struct sixpack base;
	unsigned indent;
};

static void*
process_dump(struct sixpack *sp, enum sixpack_type type, const char *data, unsigned data_size)
{
	struct sixpack_ext *spx = (struct sixpack_ext*) sp;

	if (data != SIXPACK_END)
		for (unsigned i = 0; i < spx->indent; i++)
			fputs("  ", stdout);

	switch (type) {
	case SIXPACK_INTEGER:
		printf("integer <%s>\n", data);
		break;
	case SIXPACK_FLOAT:
		printf("float <%s>\n", data);
		break;
	case SIXPACK_BOOL:
		printf("bool <%s>\n", data);
		break;
	case SIXPACK_STRING:
		printf("string <%*s%s>\n", (data_size >= 25) ? 25 : data_size, data, (data_size > 25) ? "..." : "");
		break;
	case SIXPACK_LIST:
		if (data == SIXPACK_BEGIN) {
			puts("list");
			spx->indent++;
		} else {
			assert(data == SIXPACK_END);
			assert(spx->indent > 0);
			spx->indent--;
		}
		break;
	case SIXPACK_DICT:
		if (data == SIXPACK_BEGIN) {
			puts("dictionary");
			spx->indent++;
		} else {
			assert(data == SIXPACK_END);
			assert(spx->indent > 0);
			spx->indent--;
		}
		break;
	}

	return SIXPACK_CONTINUE;
}

static void
parse_one(const char *argv0, const char *filename, struct sixpack_ext *spx, bool *ok)
{
	if (!(spx->base.userdata = filename ? fopen(filename, "rb") : stdin)) {
		fprintf(stderr, "%s: cannot open '%s': %s.\n", argv0, filename, strerror(errno));
		*ok = false;
		return;
	}

	spx->indent = 0;
	if (!sixpack_parse(&spx->base)) {
		fprintf(stderr, "line %u, column %u: %s\n",
				spx->base.error_line, spx->base.error_column,
				spx->base.error_message ? spx->base.error_message : "no errror message");
		*ok = false;
	}

	if (filename)
		fclose(spx->base.userdata);
}

int
main(int argc, char *argv[])
{
	struct sixpack_ext spx = { .base = { .getchar = sixpack_stdio_getchar } };
	bool verbose = false;
	int optch;
	while ((optch = getopt(argc, argv, "htv")) != -1) {
		switch (optch) {
		case 't':
			spx.base.process = process_dump;
			break;
		case 'v':
			verbose = true;
			break;
		default:
			fprintf(stderr,
					"Usage: %s [-vt] [files...]\n"
					"Without files, parse standard input.\n"
					"Options:\n"
					"  -v  Show file paths as they are being processed.\n"
					"  -t  Dump tree structure of parsed files.\n",
					argv[0]);
			return (optch == 'h') ? EXIT_SUCCESS : EXIT_FAILURE;
		}
	}

	bool ok = true;
	if (optind >= argc) {
		parse_one(argv[0], NULL, &spx, &ok);
	} else {
		for (int i = optind; i < argc; i++) {
			if (verbose) printf("=== %s ===\n", argv[i]);
			parse_one(argv[0], argv[i], &spx, &ok);
		}
	}
	return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif // SIXPACK_MAIN
