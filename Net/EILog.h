#ifndef _EILOG_H
#define _EILOG_H

// 调试代码
enum EDEBUG
{
    ETRACE,
    EDEBUG,
    EINFO,
    EWARN,
    EERROR,
    EFATAL,
    EDEBUG_MAX
};

#ifdef __cplusplus
extern "C" {
#endif

#define EI_DEBUG_LEVEL EDEBUG
#define LOG(level, format, ...) \
    { \
        if (level >= EI_DEBUG_LEVEL) { \
            printf("[%s][%s %s line:%d]", #level, __FILE__,__FUNCTION__,__LINE__); \
            printf(format, ##__VA_ARGS__); \
            printf("\n"); \
        } \
    }

#ifdef __cplusplus
}
#endif

#endif