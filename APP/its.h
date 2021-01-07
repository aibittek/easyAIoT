#ifndef _TRANSLATION_H
#define _TRANSLATION_H

#include <stdbool.h>

/**
 * @brief 机器翻译
 * @param  appid            应用APPID
 * @param  apikey           应用APPKey
 * @param  apisecret        应用Secret
 * @param  src              源语言类型，例如"cn"表示中文
 * @param  dst              目标语言类型，例如"en"表示英文
 * @param  data             请求翻译数据字符串
 * @return bool             正确返回true, 错误返回false
 */
bool getITSResult(const char *appid, const char *apikey, const char *apisecret, 
    const char *src, const char *dst, const char *data);

#endif