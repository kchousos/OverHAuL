
/**
 * `bacon.h' - libbacon
 *
 * copyright (c) 2014 joseph werle <joseph.werle@gmail.com>
 */

#ifndef BACON_H
#define BACON_H 1

#if __GNUC__ >= 4
# define BACON_EXTERN __attribute__((visibility("default")))
#else
# define BACON_EXTERN
#endif

/**
 * Bacon version
 */

#ifndef BACON_VERSION
#define BACON_VERSION "1" // default
#endif

/**
 * Bacon alphabet
 */

#ifndef BACON_ALPHA
#define BACON_ALPHA "ABCDEFGHIKLMNOPQRSTUWXYZ"
#endif

/**
 * Bacon 0 binary char
 */

#ifndef BACON_A
#define BACON_A 'A'
#endif

/**
 * Bacon 1 binary char
 */

#ifndef BACON_B
#define BACON_B 'B'
#endif

/**
 * Encodes a string with baconian cipher
 * using an alphabet
 */

BACON_EXTERN char *
bacon_encode (const char *, const char *);

/**
 * Decodes a string with baconian cipher
 * using an alphabet
 */

BACON_EXTERN char *
bacon_decode (const char *, const char *);

#endif

