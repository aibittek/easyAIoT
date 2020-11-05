#ifndef __DATE_H
#define __DATE_H

#include <ntp.h>

#define DATE_TIME_LEN   64

typedef struct DateTime {
    time_t (*now)(void);
    bool (*format)(const char *fmt, char *pDate, int iLen);
}DateTime_t;

extern DateTime_t datetime;

#endif