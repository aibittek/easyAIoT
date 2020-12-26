#ifndef _BANKCARD_H
#define _BANKCARD_H

/**
 * @brief 银行卡识别
 * @param  appid            应用ID
 * @param  apikey           应用KEY
 * @param  image            被识别的银行卡图片，jpg格式
 * @param  result           识别结果
 * @param  reslen           识别结果字符串长度
 */
void BandCardRecognition(const char *appid, const char *apikey, const char *image, 
    char *result, int reslen);

#endif