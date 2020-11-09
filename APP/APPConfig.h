#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

typedef struct AppConfig {
    char appid[16];
    char appkey[64];
    char appsecret[64];
}AppConfig_t;

extern AppConfig_t appconfig;

#endif