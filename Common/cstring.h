#ifndef __CSTRING_H
#define __CSTRING_H

#include <stddef.h>

typedef struct cstring
{
    char *str;
    size_t alloced;
    size_t len;

    struct cstring *(*create)(size_t len);
    void (*destory)(struct cstring *);
    void (*appendStr)(struct cstring *cs, const char *str, size_t len);
    void (*appendChar)(struct cstring *cs, char c);
    void (*appendInt)(struct cstring *cs, int val);
    void (*frontStr)(struct cstring *cs, const char *str, size_t len);
    void (*frontChar)(struct cstring *cs, char c);
    void (*frontInt)(struct cstring *cs, int val);
    void (*clear)(struct cstring *cs);
    void (*truncate)(struct cstring *cs, size_t len);
    void (*dropBegin)(struct cstring *cs, size_t len);
    void (*dropEnd)(struct cstring *cs, size_t len);
    size_t (*length)(const struct cstring *cs);
    const char *(*peek)(const struct cstring *cs);
    char *(*dump)(const struct cstring *cs, size_t *len);
} cstring_t;

cstring_t *cstring_create(size_t len);

void cstring_destory(cstring_t *cs);

void cstring_append_str(cstring_t *cs, const char *str, size_t len);

void cstring_append_char(cstring_t *cs, char c);

void cstring_append_int(cstring_t *cs, int val);

void cstring_front_str(cstring_t *cs, const char *str, size_t len);

void cstring_front_char(cstring_t *cs, char c);

void cstring_front_int(cstring_t *cs, int val);

void cstring_clear(cstring_t *cs);

void cstring_truncate(cstring_t *cs, size_t len);

void cstring_drop_begin(cstring_t *cs, size_t len);

void cstring_drop_end(cstring_t *cs, size_t len);

size_t cstring_len(const cstring_t *cs);

const char *cstring_peek(const cstring_t *cs);

char *cstring_dump(const cstring_t *cs, size_t *len);

void cstring_init(cstring_t *cs);

void cstring_uninit(cstring_t *cs);

#define cstring_new(cstr)      \
    cstring_t *cstr = NULL;    \
    cstr = cstring_create(32); \
    cstring_init(cstr);

#define cstring_new_len(cstr, len) \
    cstring_t *cstr = NULL;        \
    cstr = cstring_create(len);    \
    cstring_init(cstr);

#define cstring_del(cstr)      \
    if (cstr)                  \
    {                          \
        cstring_uninit(cstr);  \
        cstring_destory(cstr); \
        cstr = NULL;           \
    }

#endif