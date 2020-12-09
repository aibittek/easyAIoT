#ifndef __NLP_H
#define __NLP_H

#include <cstring.h>

/**
 * @brief 获取自然语言处理的结果（使用讯飞AIUIWebAPI引擎）
 * @param  pszAppid         应用APPID
 * @param  pszKey           应用APPKey
 * @param  pszParam         请求参数
 * @param  pAudioData       请求语音数据
 * @param  iAudioLen        请求语音数据长度
 * @return cstring_t*       返回字符串结构体对象指针，错误返回NULL
 */
cstring_t *getNlpResult(const char *pszAppid, const char *pszKey, const char *pszParam,
                        void *pAudioData, int iAudioLen);

#endif