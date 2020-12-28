#include <date.h>
#include <iFlyAuth.h>
#include <EIHttpClient.h>
#include <EILog.h>
#include <cstring.h>
#include <cJSON_user_define.h>
#include <base64.h>
#include <urlencode.h>
#include <file.h>
#include "IDCard.h"

static cstring_t *getHeader(const char *appid, const char *apikey)
{
    unsigned char psMD5[16] = {0};
    unsigned char pszMD5Dist[33] = {0};
    const char *pszParam = "{\"engine_type\":\"idcard\",\"head_portrait\":\"0\",\"crop_image\":\"0\"}";

    // 需要的额外头文件
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

static cstring_t *getBody(const char *pathname)
{
    cstring_t *body = readFile(pathname);
    if (!body) return NULL;
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

static bool getJSONResult(const char *szResponse, IDCard_t *pIDCard)
{
    int code, iRet;

    JSON_DESERIALIZE_START(json_root, szResponse, iRet);
        JSON_DESERIALIZE_GET_INT(json_root, "code", code, iRet, JSON_CTRL_BREAK);
        JSON_DESERIALIZE_GET_OBJECT(json_root, "data", data_obj, iRet, JSON_CTRL_BREAK);
        JSON_DESERIALIZE_GET_STRING_COPY(data_obj, "address", pIDCard->address, sizeof(pIDCard->address), iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_STRING_COPY(data_obj, "birthday", pIDCard->birthday, sizeof(pIDCard->birthday), iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_INT(data_obj, "border_covered", pIDCard->border_covered, iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_INT(data_obj, "complete", pIDCard->complete, iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_INT(data_obj, "error_code", pIDCard->error_code, iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_STRING_COPY(data_obj, "error_msg", pIDCard->error_msg, sizeof(pIDCard->error_msg), iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_INT(data_obj, "gray_image", pIDCard->gray_image, iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_INT(data_obj, "head_blurred", pIDCard->head_blurred, iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_INT(data_obj, "head_covered", pIDCard->head_covered, iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_STRING_COPY(data_obj, "id_number", pIDCard->id_number, sizeof(pIDCard->id_number), iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_STRING_COPY(data_obj, "name", pIDCard->name, sizeof(pIDCard->name), iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_STRING_COPY(data_obj, "people", pIDCard->people, sizeof(pIDCard->people), iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_STRING_COPY(data_obj, "sex", pIDCard->sex, sizeof(pIDCard->sex), iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_STRING_COPY(data_obj, "type", pIDCard->type, sizeof(pIDCard->type), iRet, JSON_CTRL_NULL);
    JSON_DESERIALIZE_END(json_root, iRet);
}

bool getIDCard(const char *appid, const char *apisecret, const char *appkey,
        const char *image, IDCard_t *pIDCard)
{
    bool bRet = false;
    SEIHttpInfo_t stHttpInfo;
    cstring_t *header = NULL, *body = NULL;
    char *szBaseUrl = "http://webapi.xfyun.cn/v1/service/v1/ocr/idcard";

    cstring_new(result);

    do {
        // 构造header
        cstring_t *header = getHeader(appid, appkey);
        if (!header) break;

        // 构造请求body
        cstring_t *body = getBody(image);

        // 发起HTTP Post请求
        LOG(EDEBUG, "header:%s,bodylen:%d", header->str, body->length(body));
        bRet = bConnectHttpServer(&stHttpInfo, szBaseUrl, header->str,
                        body->str, body->len);
        if (!bRet) break;
    } while(0);

    if (stHttpInfo.stResponse.pstBody->sBuffer) {
        bRet = getJSONResult(stHttpInfo.stResponse.pstBody->sBuffer, pIDCard);
    }

    // 关闭HTTP
    bHttpClose(&stHttpInfo);

    cstring_del(header);
    cstring_del(body);

    return bRet;
}
