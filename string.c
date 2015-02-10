/*
 * Copyright (C) 2004-2015 Oliver Hitz <oliver@net-track.ch>
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "serial.h"
#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int _strlen(const char *c)
{
  int l = 0;
  while ('\0' != *c++) {
    l++;
  }
  return l;
}

static void _strcpy(const char *s, char *d)
{
  while ('\0' != (*d++ = *s++)) ;
}

static int _indexof(const char *s, int c)
{
  int i;
  for (i = 0; '\0' != s[i]; i++) {
    if (c == s[i]) {
      return i;
    }
  }
  return -1;
}

static const char *_skipdelimiters(const char *s, const char *d)
{
  while ('\0' != *s) {
    if (-1 == _indexof(d, *s)) {
      break;
    }
    s++;
  }
  return s;
}

static const char *_finddelimiters(const char *s, const char *d)
{
  while ('\0' != *s) {
    if (-1 != _indexof(d, *s)) {
      break;
    }
    s++;
  }
  return s;
}

static void *_alloc(int size)
{
  void *m;
  if (0 == (m = (void *) malloc(size))) {
    perror("Unable to allocate memory (" __FILE__ ") ");
    exit(1);
  }
  return m;
}


string *str_alloc(string *s, int l)
{
  if (!s) {
    s = _alloc(sizeof(string));
    s->alloc = 1;
  } else {
    s->alloc = 0;
  }
  s->len = 0;
  s->content = _alloc(l+1);
  s->content[0] = 0;
  s->maxlen = l;
  return s;
}

string *str_increase(struct string *s, int l)
{
  char *c = _alloc(l + 1);
  c[0] = '\0';
  if (0 != s->content) {
    _strcpy(s->content, c);
    free(s->content);
  }
  s->content = c;
  s->maxlen = l;
  return s;
}

void str_free(string *s)
{
  if (s->content) {
    free(s->content);
  }
  if (s->alloc) {
    free(s);
  } else {
    s->content = 0;
    s->len = 0;
    s->maxlen = 0;
  }
}

void str_clear(string *s)
{
  s->len = 0;
  s->content[0] = 0;
}

string *str_create(string *s, const char *c)
{
  int l = _strlen(c);
  s = str_alloc(s, l);
  _strcpy(c, s->content);
  s->len = l;
  return s;
}

const char *str_getbuf(string *s)
{
  return s->content;
}

void str_replace(string *s, const char *c)
{
  int l = _strlen(c);
  if (s->maxlen < l) {
    str_increase(s, l);
  }
  _strcpy(c, s->content);
  s->len = l;
}

int str_len(string *s)
{
  return s->len;
}

int str_getc(string *s, int i)
{
  if (i >= s->len) {
    return -1;
  }
  return s->content[i];
}

void str_append(string *s, string *d)
{
  int i;
  for (i = 0; i < s->len; i++) {
    str_appendc(d, str_getc(s, i));
  }
}

void str_appendc(string *s, char c)
{
  if (s->maxlen < s->len + 2) {
    str_increase(s, s->maxlen + 64);
  }
  s->content[s->len] = c;
  s->content[s->len+1] = 0;
  s->len++;
}

void str_sprintf(string *s, int size, const char *format, ...)
{
  va_list va;

  if (s->maxlen < size) {
    str_increase(s, size);
  }
  va_start(va, format);
  s->len = vsnprintf(s->content, size+1, format, va);
}

int str_cmp(string *s1, string *s2)
{
  char *_a = s1->content;
  char *_b = s2->content;

  for (; *_a == *_b; _a++, _b++) {
    if ('\0' == *_a) {
      return 0;
    }
  }
  return *_a - *_b;
}

int str_cmpb(string *a, char *b)
{
  char *_a = a->content;

  for (; *_a == *b; _a++, b++) {
    if ('\0' == *_a) {
      return 0;
    }
  }
  return *_a - *b;
}

int str_tok(string *t, int n, const char *s, const char *d)
{
  const char *e;
  int i;

  for (i = 0; i < n; i++) {
    s = _skipdelimiters(s, d);
    e = _finddelimiters(s, d);
    str_clear(&t[i]);
    while (*s != *e) {
      str_appendc(&t[i], *s++);
    }
    s = e;
  }
  return i;
}

string *str_substring(string *s, int start, int length)
{
  string *d;
  int i;

  if (length > 0) {
    d = str_alloc(NULL, length);
  } else {
    d = str_alloc(NULL, str_len(s) - start);
  }

  for (i = 0; (i < length || length == 0); i++) {
    int c = str_getc(s, start+i);
    if (c == -1) {
      return d;
    }
    str_appendc(d, c);
  }

  return d;
}
