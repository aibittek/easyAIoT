#ifndef _EILOG_H
#define _EILOG_H

// 调试代码
enum EDEBUG
{
    EVERBOSE,
    EDEBUG,
    EINFO,
    EWARNNING,
    EERROR,
    EDEBUG_MAX
};

#define EI_DEBUG_LEVEL EERROR
#define LOG(level, format, ...) \
    { \
        if (level >= EI_DEBUG_LEVEL) { \
            printf("[%s %s line:%d]"format"\n",__FILE__,__FUNCTION__,__LINE__, ##__VA_ARGS__); \
        } \
    }

#endif