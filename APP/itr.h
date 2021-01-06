#ifndef _ITR_H
#define _ITR_H

#include <stdbool.h>

// 拍照速算识别
bool getITRResult(const char *appid, const char *appkey, const char *appsecret, const char *pathname);

#endif