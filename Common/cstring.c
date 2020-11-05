#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cstring.h"

void cstring_init(cstring_t *cs)
{
    if (!cs) return ;

    cs->create = cstring_create;
    cs->destory = cstring_destory;
    cs->appendStr = cstring_append_str;
    cs->appendChar = cstring_append_char;
    cs->appendInt = cstring_append_int;
    cs->frontStr = cstring_front_str;
    cs->frontChar = cstring_front_char;
    cs->frontInt = cstring_front_int;
    cs->clear = cstring_clear;
    cs->truncate = cstring_truncate;
    cs->dropBegin = cstring_drop_begin;
    cs->dropEnd = cstring_drop_end;
    cs->length = cstring_len;
    cs->peek = cstring_peek;
    cs->dump = cstring_dump;
}

void cstring_uninit(cstring_t *cs)
{
    if (!cs) return ;

    cs->create = NULL;
    cs->destory = NULL;
    cs->appendStr = NULL;
    cs->appendChar = NULL;
    cs->appendInt = NULL;
    cs->frontStr = NULL;
    cs->frontChar = NULL;
    cs->frontInt = NULL;
    cs->clear = NULL;
    cs->truncate = NULL;
    cs->dropBegin = NULL;
    cs->dropEnd = NULL;
    cs->length = NULL;
    cs->peek = NULL;
    cs->dump = NULL;
}

cstring_t *cstring_create(size_t len)
{
    cstring_t *cs;

    cs = calloc(1, sizeof(*cs));
    cs->str = malloc(len);
    *cs->str = '\0';
    cs->alloced = len;
    cs->len = 0;

    return cs;
}

void cstring_destory(cstring_t *cs)
{
    if (cs == NULL)
        return;
    if (cs->str) {
        free(cs->str);
        cs->str = NULL;
    }
    free(cs);
}

static void cstring_ensure_space(cstring_t *cs, size_t add_len)
{
    if (cs == NULL || add_len == 0)
        return;

    if (cs->alloced >= cs->len + add_len + 1)
        return;

    while (cs->alloced < cs->len + add_len + 1)
    {
        cs->alloced <<= 1;
        if (cs->alloced == 0)
        {
            cs->alloced--;
        }
    }
    cs->str = realloc(cs->str, cs->alloced);
}

void cstring_append_str(cstring_t *cs, const char *str, size_t len)
{
    if (cs == NULL || str == NULL || *str == '\0')
        return;

    if (len == 0)
        len = strlen(str);

    cstring_ensure_space(cs, len);
    memmove(cs->str + cs->len, str, len);
    cs->len += len;
    cs->str[cs->len] = '\0';
}

void cstring_append_char(cstring_t *cs, char c)
{
    if (cs == NULL)
        return;
    cstring_ensure_space(cs, 1);
    cs->str[cs->len] = c;
    cs->len++;
    cs->str[cs->len] = '\0';
}

void cstring_append_int(cstring_t *cs, int val)
{
    char str[12];

    if (cs == NULL)
        return;

    snprintf(str, sizeof(str), "%d", val);
    cstring_append_str(cs, str, 0);
}

void cstring_front_str(cstring_t *cs, const char *str, size_t len)
{
    if (cs == NULL || str == NULL || *str == '\0')
        return;

    if (len == 0)
        len = strlen(str);

    cstring_ensure_space(cs, len);
    memmove(cs->str + len, cs->str, cs->len);
    memmove(cs->str, str, len);
    cs->len += len;
    cs->str[cs->len] = '\0';
}

void cstring_front_char(cstring_t *cs, char c)
{
    if (cs == NULL)
        return;
    cstring_ensure_space(cs, 1);
    memmove(cs->str + 1, cs->str, cs->len);
    cs->str[0] = c;
    cs->len++;
    cs->str[cs->len] = '\0';
}

void cstring_front_int(cstring_t *cs, int val)
{
    char str[12];

    if (cs == NULL)
        return;

    snprintf(str, sizeof(str), "%d", val);
    cstring_front_str(cs, str, 0);
}

void cstring_clear(cstring_t *cs)
{
    if (cs == NULL)
        return;
    cstring_truncate(cs, 0);
}

void cstring_truncate(cstring_t *cs, size_t len)
{
    if (cs == NULL || len >= cs->len)
        return;

    cs->len = len;
    cs->str[cs->len] = '\0';
}

void cstring_drop_begin(cstring_t *cs, size_t len)
{
    if (cs == NULL || len == 0)
        return;

    if (len >= cs->len)
    {
        cstring_clear(cs);
        return;
    }

    cs->len -= len;
    /* +1 to move the NULL. */
    memmove(cs->str, cs->str + len, cs->len + 1);
}

void cstring_drop_end(cstring_t *cs, size_t len)
{
    if (cs == NULL || len == 0)
        return;

    if (len >= cs->len)
    {
        cstring_clear(cs);
        return;
    }
    cs->len -= len;
    cs->str[cs->len] = '\0';
}

size_t cstring_len(const cstring_t *cs)
{
    if (cs == NULL)
        return 0;
    return cs->len;
}

const char *cstring_peek(const cstring_t *cs)
{
    if (!cs) return NULL;
    return cs->str;
}

char *cstring_dump(const cstring_t *cs, size_t *len)
{
    char *out;

    if (!cs) return NULL;

    if (len != NULL)
        *len = cs->len;
    out = malloc(cs->len + 1);
    memcpy(out, cs->str, cs->len + 1);
    return out;
}
