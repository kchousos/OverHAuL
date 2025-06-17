#include "locale-string.h"

#ifdef _WIN32
# include <windows.h>
#else
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

static char utf8len_tab[256] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /*bogus*/
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /*bogus*/
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1,
};

static int
utf_bytes2char(unsigned char* p) {
  int    len;

  if (p[0] < 0x80)  /* be quick for ASCII */
    return p[0];

  len = utf8len_tab[p[0]];
  if ((p[1] & 0xc0) == 0x80) {
    if (len == 2)
      return ((p[0] & 0x1f) << 6) + (p[1] & 0x3f);
    if ((p[2] & 0xc0) == 0x80) {
      if (len == 3)
        return ((p[0] & 0x0f) << 12) + ((p[1] & 0x3f) << 6)
          + (p[2] & 0x3f);
      if ((p[3] & 0xc0) == 0x80) {
        if (len == 4)
          return ((p[0] & 0x07) << 18) + ((p[1] & 0x3f) << 12)
            + ((p[2] & 0x3f) << 6) + (p[3] & 0x3f);
        if ((p[4] & 0xc0) == 0x80) {
          if (len == 5)
            return ((p[0] & 0x03) << 24) + ((p[1] & 0x3f) << 18)
              + ((p[2] & 0x3f) << 12) + ((p[3] & 0x3f) << 6)
              + (p[4] & 0x3f);
          if ((p[5] & 0xc0) == 0x80 && len == 6)
            return ((p[0] & 0x01) << 30) + ((p[1] & 0x3f) << 24)
              + ((p[2] & 0x3f) << 18) + ((p[3] & 0x3f) << 12)
              + ((p[4] & 0x3f) << 6) + (p[5] & 0x3f);
        }
      }
    }
  }
  /* Illegal value, just return the first byte */
  return p[0];
}

static int
utf_char2bytes(int c, unsigned char* buf) {
  if (c < 0x80) {      /* 7 bits */
    buf[0] = c;
    return 1;
  }
  if (c < 0x800) {    /* 11 bits */
    buf[0] = 0xc0 + ((unsigned)c >> 6);
    buf[1] = 0x80 + (c & 0x3f);
    return 2;
  }
  if (c < 0x10000) {    /* 16 bits */
    buf[0] = 0xe0 + ((unsigned)c >> 12);
    buf[1] = 0x80 + (((unsigned)c >> 6) & 0x3f);
    buf[2] = 0x80 + (c & 0x3f);
    return 3;
  }
  if (c < 0x200000) {    /* 21 bits */
    buf[0] = 0xf0 + ((unsigned)c >> 18);
    buf[1] = 0x80 + (((unsigned)c >> 12) & 0x3f);
    buf[2] = 0x80 + (((unsigned)c >> 6) & 0x3f);
    buf[3] = 0x80 + (c & 0x3f);
    return 4;
  }
  if (c < 0x4000000) {  /* 26 bits */
    buf[0] = 0xf8 + ((unsigned)c >> 24);
    buf[1] = 0x80 + (((unsigned)c >> 18) & 0x3f);
    buf[2] = 0x80 + (((unsigned)c >> 12) & 0x3f);
    buf[3] = 0x80 + (((unsigned)c >> 6) & 0x3f);
    buf[4] = 0x80 + (c & 0x3f);
    return 5;
  }

  /* 31 bits */
  buf[0] = 0xfc + ((unsigned)c >> 30);
  buf[1] = 0x80 + (((unsigned)c >> 24) & 0x3f);
  buf[2] = 0x80 + (((unsigned)c >> 18) & 0x3f);
  buf[3] = 0x80 + (((unsigned)c >> 12) & 0x3f);
  buf[4] = 0x80 + (((unsigned)c >> 6) & 0x3f);
  buf[5] = 0x80 + (c & 0x3f);
  return 6;
}
#endif

char*
utf8_to_locale_alloc(const char* str) {
#ifdef _WIN32
  UINT cp = CP_UTF8;
  if (str[0] == (char)0xef && str[1] == (char)0xbb && str[2] == (char)0xbf)
    str += 3;
  size_t pwcl = MultiByteToWideChar(cp, 0, str, -1,  NULL, 0);
  wchar_t* pwcs = (wchar_t*) malloc(sizeof(wchar_t) * (pwcl + 1));
  pwcl = MultiByteToWideChar(cp, 0, str, -1, pwcs, pwcl + 1);
  pwcs[pwcl] = '\0';
  cp = GetACP();
  size_t pmbl = WideCharToMultiByte(cp, 0, (LPCWSTR) pwcs,-1,NULL,0,NULL,NULL);
  char* pmbs = (char*) malloc(sizeof(char) * (pmbl + 1));
  pmbl = WideCharToMultiByte(cp, 0, (LPCWSTR) pwcs, -1, pmbs, pmbl, NULL, NULL);
  pmbs[pmbl] = '\0';
  free(pwcs);
  return pmbs;
#else
  if (str[0] == (char)0xef && str[1] == (char)0xbb && str[2] == (char)0xbf)
    str += 3;
  size_t len = strlen(str);
  wchar_t* pwcs = (wchar_t*) malloc(sizeof(wchar_t) * (len + 1));
  if (!pwcs) return NULL;
  size_t pwcl = 0;
  const char* end = str + len;
  while(str < end) {
    int c = utf_bytes2char((unsigned char*) str);
    if (c == 0x301c) c = 0xff5e;
    if (c == 0x2016) c = 0x2225;
    if (c == 0x2212) c = 0xff0d;
    if (c == 0x00a2) c = 0xffe0;
    if (c == 0x00a3) c = 0xffe1;
    if (c == 0x00ac) c = 0xffe2;
    pwcs[pwcl++] = c;
    str += utf8len_tab[(unsigned char) *str];
  }
  pwcs[pwcl] = 0;
  char* pmbs = (char*) malloc(sizeof(char) * (wcslen(pwcs) * 8 + 1));
  if (!pmbs) {
    free(pwcs);
    return NULL;
  }
  char* top = pmbs;
  size_t n;
  for(n = 0; n < pwcl; n++) {
    size_t clen = wctomb(top, pwcs[n]);
    top += clen <= 0 ? 1 : clen;
  }
  *top = '\0';
  free(pwcs);
  return pmbs;
#endif
}

char*
utf8_from_locale_alloc(const char* str) {
#ifdef _WIN32
  UINT cp = GetACP();
  int len = strlen(str);
  size_t pwcl = MultiByteToWideChar(cp, 0, str, len,  NULL, 0);
  wchar_t* pwcs = (wchar_t*) malloc(sizeof(wchar_t) * (pwcl + 1));
  pwcl = MultiByteToWideChar(cp, 0, str, len, pwcs, pwcl + 1);
  pwcs[pwcl] = '\0';
  cp = CP_UTF8;
  size_t pmbl = WideCharToMultiByte(cp, 0, pwcs, -1, NULL, 0, NULL, NULL);
  char* pmbs = (char*) malloc(sizeof(char) * (pmbl + 1));
  pmbl = WideCharToMultiByte(cp, 0, pwcs, pwcl, pmbs, pmbl, NULL, NULL);
  pmbs[pmbl] = '\0';
  free(pwcs);
  return pmbs;
#else
  size_t len = strlen(str);
  wchar_t* pwcs = (wchar_t*) malloc(sizeof(wchar_t) * (len + 1));
  if (!pwcs) return NULL;
  size_t pwcl = 0;
  const char* end = str + len;
  mblen(NULL, 0);
  while(str < end) {
    size_t clen = mblen(str, MB_CUR_MAX);
    if (clen <= 0) {
      mblen(NULL, 0);
      clen = 1;
    }
    clen = mbtowc(pwcs + pwcl++, str,  clen);
    if (clen <= 0) {
      mblen(NULL, 0);
      clen = 1;
    }
    str += clen;
  }
  pwcs[pwcl] = '\0';
  char* pmbs = (char*) malloc(sizeof(char) * (pwcl * 8 + 1));
  if (!pmbs) {
    free(pwcs);
    return NULL;
  }
  char* top = pmbs;
  size_t n;
  for(n = 0; n < pwcl; n++) {
    unsigned char bytes[8] = {0};
    size_t clen = utf_char2bytes(pwcs[n], bytes);
    memcpy(top, bytes, clen);
    top += clen;
  }
  *top = '\0';
  free(pwcs);
  return pmbs;
#endif
}
