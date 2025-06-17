#ifndef LIKELY_H
#define LIKELY_H

#include <stdlib.h>

#ifndef LIKELY
#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define LIKELY(x) (x)
#endif
#endif

#ifndef UNLIKELY
#if defined(__GNUC__) || defined(__clang__)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define UNLIKELY(x) (x)
#endif
#endif

#endif // LIKELY_H