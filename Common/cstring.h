#ifndef __CSTRING_H
#define __CSTRING_H

#include <stddef.h>

typedef struct cstring
{
    char *str;                                                              // 字符串
    size_t alloced;                                                         // 动态分配
    size_t len;                                                             // 字符长度

    struct cstring *(*create)(size_t len);                                  // 创建字符串
    void (*destory)(struct cstring *);                                      // 释放字符串
    void (*appendStr)(struct cstring *cs, const char *str, size_t len);     // 追加字符串
    void (*appendChar)(struct cstring *cs, char c);                         // 追加字符
    void (*appendInt)(struct cstring *cs, int val);                         // 追加整型
    void (*frontStr)(struct cstring *cs, const char *str, size_t len);      // 从头插入字符串
    void (*frontChar)(struct cstring *cs, char c);                          // 从头插入字符
    void (*frontInt)(struct cstring *cs, int val);                          // 从头插入整型
    void (*clear)(struct cstring *cs);                                      // 清空字符串内容
    void (*truncate)(struct cstring *cs, size_t len);                       // 截断字符串
    void (*dropBegin)(struct cstring *cs, size_t len);                      // 丢掉前面指定len的字符
    void (*dropEnd)(struct cstring *cs, size_t len);                        // 丢掉后面指定len的字符
    size_t (*length)(const struct cstring *cs);                             // 求取字符串长度
    const char *(*peek)(const struct cstring *cs);                          // 获取字符串首地址
    char *(*dump)(const struct cstring *cs, size_t *len);                   // 获取字符串内容
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