#include <date.h>
#include <iFlyAuth.h>
#include <EIHttpClient.h>
#include <EILog.h>
#include <cstring.h>
#include <cJSON_user_define.h>
#include <base64.h>
#include <urlencode.h>
#include "BankCard.h"

static cstring_t *getHeader(const char *appid, const char *apikey)
{
    unsigned char psMD5[16] = {0};
    unsigned char pszMD5Dist[33] = {0};
    const char *pszParam = "{\"engine_type\": \"bankcard\", \"card_number_image\": \"0\"}";

    // AIUI需要的额外头文件
    const char pszAIUIHeader[] = {
        "X-Appid: %s\r\n"
        "X-CurTime: %lu\r\n"
        "X-Param: %s\r\n"
        "X-CheckSum: %s\r\n"
        "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n"};

    // 获取系统当前NTP时间
    time_t dwTime = datetime.now();

    // 获取参数
    cstring_new_len(base64Param, strlen(pszParam) * 2);
    iBase64Encode(pszParam, base64Param->str, strlen(pszParam));
    base64Param->len = strlen(base64Param->str);

    // 获取checksum
    int size = strlen(apikey) + sizeof(dwTime) * 8 + base64Param->len;
    cstring_new_len(strCheckSum, size);
    snprintf(strCheckSum->str, size, "%s%lu%s", apikey, dwTime, base64Param->str);
    strCheckSum->len = strlen(strCheckSum->str);

    md5String(strCheckSum->str, strlen(strCheckSum->str), pszMD5Dist);

    // 构造Header
    size = strlen(pszAIUIHeader) + strlen(appid) + sizeof(dwTime) * 8 +
           base64Param->len + sizeof(pszMD5Dist);

    cstring_new_len(strHeader, size);
    snprintf(strHeader->str, size, pszAIUIHeader, appid, dwTime, base64Param->str, pszMD5Dist);
    strHeader->len = strlen(strHeader->str);

    cstring_del(base64Param);
    cstring_del(strCheckSum);

    return strHeader;
}

cstring_t *getBody(const char *pathname)
{
    cstring_t *body = readFile(pathname);

    cstring_new_len(base64Body, body->len*2);
    iBase64Encode(body->str, base64Body->str, body->len);
    base64Body->len = strlen(base64Body->str);

    const char *pBodyTemplate = "image=%s";
    int size = strlen(pBodyTemplate) + base64Body->len + 1;
    cstring_new_len(jsonBody, size);
    snprintf(jsonBody->str, size, pBodyTemplate, base64Body->str);
    jsonBody->len = strlen(jsonBody->str);

    cstring_new_len(finalBody, jsonBody->len*1.5);
    urlencode(jsonBody->str, finalBody->str);
    finalBody->len = strlen(finalBody->str);

    cstring_del(body);
    cstring_del(base64Body);
    cstring_del(jsonBody);
    return finalBody;
}

void BandCardRecognition(const char *appid, const char *apikey, const char *image,
                         char *result, int reslen)
{
    bool bRet = false;
    SEIHttpInfo_t stHttpInfo;
    cstring_t *header = NULL, *body = NULL;
    char *szBaseUrl = "http://webapi.xfyun.cn/v1/service/v1/ocr/bankcard";

    do {
        // 构造header
        cstring_t *header = getHeader(appid, apikey);
        if (!header) break;

        // 构造请求body
        cstring_t *body = getBody(image);

        // 发起HTTP Post请求
        LOG(EDEBUG, "header:%s,bodylen:%d", header->str, body->length(body));
        bRet = bConnectHttpServer(&stHttpInfo, szBaseUrl, header->str,
                        body->str, body->len);
        if (!bRet) break;

        LOG(EDEBUG, "card info:%s", stHttpInfo.stResponse.pstBody->sBuffer);
        snprintf(result, reslen, "%s", stHttpInfo.stResponse.pstBody->sBuffer);
    } while(0);

    // 关闭HTTP
    bHttpClose(&stHttpInfo);

    cstring_del(header);
    cstring_del(body);
}