#ifndef __ID_H
#define __ID_H

#include <cstring.h>

typedef struct IDCard {
        char address[256];
        char birthday[256];
        bool border_covered;
        bool complete;
        int error_code;
        char error_msg[64];
        bool gray_image;
        bool head_blurred;
        bool head_covered;
        char id_number[20];
        char name[32];
        char people[32];
        char sex[8];
        char type[32];
}IDCard_t;

/**
 * @brief 身份证识别
 * @param  appid                应用ID  
 * @param  apisecret            应用密码
 * @param  appkey               应用key
 * @param  image                身份证照片
 * @return cstring_t*           返回身份证的详细信息
 */
bool getIDCard(const char *appid, const char *apisecret, const char *appkey,
        const char *image, IDCard_t *pIDCard);

#endif