#ifndef __TTS_H
#define __TTS_H

#include <cstring.h>

/**
 * @brief 文字转语音
 * @param  appid            讯飞appid
 * @param  key              讯飞api key
 * @param  param            讯飞合成参数
 * @param  text             合成的文本内容
 * @param  pathname         合成的结果音频文件路径名称
 * @return true             合成成功
 * @return false            合成失败
 */
bool getTTS(const char *appid, const char *key, const char *param, 
    const char *text, const char *pathname);

#endif