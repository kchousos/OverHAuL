#ifndef DATEPARSE_H
#define DATEPARSE_H

#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>



#ifdef __cplusplus
extern "C" {
#endif

#define date_t long long // microseconds

//params: date string, result microseconds, result offset, stringlength if available
int dateparse(const char* datestr, date_t* dt, int* offset, int stringlen);
#define dateparse_2(D, T)    dateparse(D, T, 0, 0)
#define dateparse_3(D, T, F) dateparse(D, T, F, 0)

//returns statically allocated date string
char* datestring(date_t);
char* datestringfmt(date_t t, const char*);

int secs_to_tm(long long t, struct tm *tm);

//convert between struct tm and date_t
struct tm* gmtime64(date_t);
date_t mktimegm(const struct tm *tm);

//get microsecond component (remainder)
#define mcs(A) (A>=0 ? A%1000000 : (A%1000000 ? 1000000+A%1000000 : 0))
//get unix seconds from date_t for secs_to_tm
#define sec(A) (A<0 && A%1000000 ? A/1000000-1 : A/1000000)

extern char* daynames[];
extern char* monthnames[];
extern unsigned int daylens[];
extern unsigned int monthlens[];
date_t nowlocal();
date_t nowgm();

 //set to 1 to skip dates that are formatted like plain numbers, like unix seconds etc
extern int noNumericDates;

#ifdef __cplusplus
}
#endif

#endif
