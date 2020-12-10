#include "date.h"

time_t uDateTimeNow(void);
bool szDateformat(const char *fmt, char *pDate, int iLen);

DateTime_t datetime = {
    .now = uDateTimeNow,
    .format = szDateformat,
};

time_t uDateTimeNow(void)
{
    int iCount = 3;
    time_t dwTime = 0;
    do {
        // dwTime = dwGetNTPtime("cn.ntp.org.cn");
        dwTime = dwGetNTPtime("ntp.aliyun.com");
        if (dwTime > 0) break;
    } while(iCount--);

    return dwTime;
}

bool szDateformat(const char *fmt, char *pDate, int iLen)
{
    int iMon;
    char szTime[5][10];
    time_t dwTime = uDateTimeNow();

    if (0 == dwTime) return false;
 
    if (0 == strcmp(fmt, "GMT") ||
        0 == strcmp(fmt, "gmt")) {
        // 以上获取了北京的NTP时间，北京位于东8区，比GMT(格林尼治)时间快8小时
        dwTime -= 8 * 60 * 60;
    }

    // 把时间转化为字符串格式
    sscanf(ctime(&dwTime), "%s %s %d %s %s", szTime[0], szTime[1], &iMon, szTime[3], szTime[4]);
    snprintf(pDate, iLen, "%s, %02d %s %s %s GMT", szTime[0], iMon, szTime[1],
            szTime[4], szTime[3]);
    return true;
}
