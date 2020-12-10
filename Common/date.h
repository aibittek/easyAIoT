#ifndef __DATE_H
#define __DATE_H

#include <ntp.h>

#define DATE_TIME_LEN   64

/**
 * @brief dateime结构体，通过now获取当前时间，通过format对时间进行格式化输出，输出到pDate中
 */
typedef struct DateTime {
    time_t (*now)(void);
    bool (*format)(const char *fmt, char *pDate, int iLen);
}DateTime_t;

/**
 * @brief datetime对象
 */
extern DateTime_t datetime;

#endif