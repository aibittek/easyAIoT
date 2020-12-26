#ifndef _TRANSLATION_H
#define _TRANSLATION_H

#include <cstring.h>

/**
 * @brief 机器翻译
 * @param  pszAppid         应用APPID
 * @param  pszKey           应用APPKey
 * @param  pszSecret        应用Secret
 * @param  pData            请求翻译数据字符串
 * @return cstring_t*       返回字符串结构体对象指针，错误返回NULL
 */
cstring_t *getITSResult(const char *pszAppid, const char *pszKey, const char *pszParam, const char *pData);

#endif