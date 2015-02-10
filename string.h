/*
 * Copyright (C) 2004-2015 Oliver Hitz <oliver@net-track.ch>
 */

#ifndef __STRING_H
#define __STRING_H

struct string
{
  char *content;
  int len;
  int maxlen;
  int alloc;
};

typedef struct string string;

string *str_alloc(string *s, int l);
string *str_increase(string *s, int l);
void str_free(string *s);
void str_clear(string *s);
string *str_create(string *s, const char *c);
const char *str_getbuf(string *s);
int str_len(string *s);
int str_getc(string *c, int i);
void str_append(string *s, string *d);
void str_appendc(string *s, char c);
void str_sprintf(string *s, int size, const char *format, ...);
int str_cmp(string *a, string *b);
int str_cmpb(string *a, char *b);
int str_tok(string *t, int n, const char *s, const char *d);
void str_replace(string *s, const char *c);
string *str_substring(string *s, int start, int length);

#endif /* __STRING_H */
