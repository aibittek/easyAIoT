#include <EILog.h>
#include <file.h>
#include <cstring.h>
#include <sha256.h>
#include <base64.h>
#include <date.h>
#include <iFlyAuth.h>
#include <EIHttpClient.h>
#include <cJSON_user_define.h>
#include "itr.h"

static cstring_t *getBody(const char *appid, const char *pathname)
{
    cstring_t *body = NULL;
    const char *param = "{\"common\": {\"app_id\": \"%s\"}, \"business\": {\"ent\": \"math-arith\", \"aue\": \"raw\"}, \"data\": {\"image\": \"%s\"}}";
    
    // 读取文件内容
    cstring_t *file = readFile(pathname);
    if (!file) return false;

    // 对读取内容进行base64加密
    cstring_new_len(base64image, file->len*1.5);
    if (!base64image) goto base64_fail;
    iBase64Encode(file->str, base64image->str, file->len);
    base64image->len = strlen(base64image->str);

    // 生成body的内容
    int size = strlen(param) + file->len + base64image->len;
    cstring_new_len(bodyParam, size);
    sprintf(bodyParam->str, param, appid, base64image->str);
    bodyParam->len = strlen(bodyParam->str);
    body = bodyParam;

base64_fail:
    cstring_del(base64image);
file_fail:
    cstring_del(file);

    return body;
}

static cstring_t *getHeader(const char *appid, const char *appkey, const char *appsecret, cstring_t *body)
{
    char szDate[64];
    char Digest[64] = {0};
    unsigned char Sha256Digest[65] = {0};
    char szAuth[1024] = {0};

    // AIUI需要的额外头文件
    const char header[] = {
        "Date: %s\r\n"
        "Digest: SHA-256=%s\r\n"
        "Authorization: %s\r\n"
        "Content-Type: application/json\r\n"
        "Accept: application/json,version=1.0\r\n"};

    // 获取系统当前NTP时间
    datetime.format("GMT", szDate, sizeof(szDate));

    // 获取参数
    mbedtls_sha256(body->str, body->len, Sha256Digest, 0);
    // sha256(body->str, body->len, Sha256Digest);
    iBase64Encode(Sha256Digest, Digest, 32);

    // 获取Auth字段
    vGetAuth2(appkey, appsecret, "rest-api.xfyun.cn", "POST /v2/itr HTTP/1.1", 
        "host date request-line digest", "hmac-sha256", szDate, Digest, szAuth, sizeof(szAuth));

    // 构造Header
    int size = strlen(header) + strlen(appid) + sizeof(szDate) +
           strlen(szAuth) + sizeof(Digest);

    cstring_new_len(strHeader, size);
    snprintf(strHeader->str, size, header, szDate, Digest, szAuth);
    strHeader->len = strlen(strHeader->str);

    return strHeader;
}

static void parseResult(const char *pszJsonString)
{
    int iRet;
    // JSON_DESERIALIZE_START(json_root, pszJsonString, iRet);
    //     JSON_DESERIALIZE_GET_OBJECT(json_root, "payload", payload_obj, iRet, JSON_CTRL_BREAK);
    //     JSON_DESERIALIZE_GET_OBJECT(payload_obj, "face_compare_result", result_obj, iRet, JSON_CTRL_BREAK);
    //     JSON_DESERIALIZE_GET_STRING(result_obj, "text", pText, iRet, JSON_CTRL_NULL);
    //     if (pText) text->appendStr(text, pText, strlen(pText));
    // JSON_DESERIALIZE_END(json_root, iRet);
    LOG(EDEBUG, "%s", pszJsonString);
}

bool getITRResult(const char *appid, const char *appkey, const char *appsecret, const char *pathname)
{
    if (!appid || !appkey || !appsecret || !pathname)
    {
        return false;
    }

    const char *url = "http://rest-api.xfyun.cn/v2/itr";
    // 获取http请求body数据
    cstring_t *body = getBody(appid, pathname);

    // 获取http请求header数据
    cstring_t *header = getHeader(appid, appkey, appsecret, body);

    // 发起HTTP Post语音识别的请求
    SEIHttpInfo_t stHttpInfo;
    bConnectHttpServer(&stHttpInfo, url, header->str, body->str, body->len);
    LOG(EDEBUG, "status:%d", stHttpInfo.stResponse.iStatus);
    LOG(EDEBUG, "body:%s", stHttpInfo.stResponse.pstBody->sBuffer);

    // 获取结果信息
    // parseResult(header->str);

    cstring_del(header);
    cstring_del(body);
    bHttpClose(&stHttpInfo);

    return true;
}