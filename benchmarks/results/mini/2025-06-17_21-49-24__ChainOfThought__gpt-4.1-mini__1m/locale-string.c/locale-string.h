#ifndef LOCALE_STRING_H
#define LOCALE_STRING_H 1

/**
 * locale-string.h
 *
 * copyright 2014 - mattn <mattn.jp@gmail.com>
 */

/**
 * convert utf-8 string to locale string
 */
char*
utf8_to_locale_alloc(const char*);

/**
 * convert locale string to utf-8 string
 */
char*
utf8_from_locale_alloc(const char*);

#ifdef _WIN32
# define UTF8_ALLOC(p) utf8_from_locale_alloc(p)
# define UTF8_FREE(p) free(p)
# define LOCALE_ALLOC(p) utf8_to_locale_alloc(p)
# define LOCALE_FREE(p) free(p)
#else
# define UTF8_ALLOC(p) (p)
# define UTF8_FREE(p)
# define LOCALE_ALLOC(p) (p)
# define LOCALE_FREE(p)
#endif

#endif
