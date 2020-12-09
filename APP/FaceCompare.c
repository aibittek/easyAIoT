#include <date.h>
#include <iFlyAuth.h>
#include <EIHttpClient.h>
#include <EILog.h>
#include <cstring.h>
#include <cJSON_user_define.h>
#include <base64.h>
#include <urlencode.h>
#include "FaceCompare.h"

static const char REQUEST_PARAM[] = "{\"header\":{\"app_id\":\"%s\",\"status\":3},\"parameter\":{\"s67c9c78c\":{\"service_kind\":\"face_compare\",\"face_compare_result\":{\"encoding\":\"utf8\",\"compress\":\"raw\",\"format\":\"json\"}}},\"payload\":{\"input1\":{\"encoding\":\"jpg\",\"status\":3,\"image\":\"%s\"},\"input2\":{\"encoding\":\"jpg\",\"status\":3,\"image\":\"%s\"}}}";

cstring_t *pstGenBody(const char *pszAPPID, const char *pszImagePath1, const char *pszImagePath2)
{
    cstring_t *cs = NULL;
    FILE *fp1 = NULL, *fp2 = NULL;
    int len1, len2, total;

    do
    {
        fp1 = fopen(pszImagePath1, "rb");
        if (!fp1)
            break;
        fseek(fp1, 0, SEEK_END);
        len1 = ftell(fp1);
        fseek(fp1, 0, SEEK_SET);

        fp2 = fopen(pszImagePath2, "rb");
        if (!fp2)
            break;
        fseek(fp2, 0, SEEK_END);
        len2 = ftell(fp2);
        fseek(fp2, 0, SEEK_SET);

        cstring_new_len(str1, len1);
        cstring_new_len(str2, len2);
        fread(str1->str, len1, 1, fp1);
        fread(str2->str, len2, 1, fp2);

        cstring_new_len(strEncode1, len1 * 2);
        cstring_new_len(strEncode2, len2 * 2);
        iBase64Encode(str1->str, strEncode1->str, len1);
        iBase64Encode(str2->str, strEncode2->str, len2);
        strEncode1->len = strlen(strEncode1->str);
        strEncode2->len = strlen(strEncode2->str);

        total = strlen(REQUEST_PARAM) + strlen(pszAPPID) +
                strEncode1->len + strEncode2->len;
        cstring_new_len(str, total);
        snprintf(str->str, total, REQUEST_PARAM, pszAPPID,
                 strEncode1->str, strEncode2->str);
        str->len = strlen(str->str);
        cs = str;

        cstring_del(str1);
        cstring_del(str2);
        cstring_del(strEncode1);
        cstring_del(strEncode2);
    } while (0);

    if (fp1)
        fclose(fp1);
    if (fp2)
        fclose(fp2);

    return cs;
}

static double fGetScoreResult(const char *szResponse)
{
    double fScore = 0.0f;
    int iRet, iRetStauts = 0;
    char *pText = NULL;
    LOG(EDEBUG, "szResponse:%s", szResponse);

    cstring_new(text);
    
    JSON_SERIALIZE_START(json_root, szResponse, iRet);
        JSON_SERIALIZE_GET_OBJECT(json_root, "payload", payload_obj, iRet, JSON_CTRL_BREAK);
        JSON_SERIALIZE_GET_OBJECT(payload_obj, "face_compare_result", result_obj, iRet, JSON_CTRL_BREAK);
        JSON_SERIALIZE_GET_STRING(result_obj, "text", pText, iRet, JSON_CTRL_NULL);
        if (pText) text->appendStr(text, pText, strlen(pText));
    JSON_SERIALIZE_END(json_root, iRet);

    if (text->length(text)) {
        cstring_new_len(textDecode, strlen(text->str));
        iBase64Decode(text->str, textDecode->str);

        cstring_new(score);
        LOG(EDEBUG, "score json:%s", textDecode->str);
        JSON_SERIALIZE_START(json_root, textDecode->str, iRet);
        JSON_SERIALIZE_GET_INT(json_root, "ret", iRetStauts, iRet, JSON_CTRL_NULL);
        JSON_SERIALIZE_GET_DOUBLE(json_root, "score", fScore, iRet, JSON_CTRL_NULL);
        JSON_SERIALIZE_END(json_root, iRet);
        cstring_del(score);

        cstring_del(textDecode);
    }
    cstring_del(text);

    LOG(EDEBUG, "ret:%d, score:%.2f", iRetStauts, fScore);
    
    return fScore;
}

double fFaceCompare(const char *pszAPPID,
                   const char *pszAPPSecret, const char *pszAPPKey,
                   const char *pszImagePath1, const char *pszImagePath2)
{
    double fScore = 0.0f;
    char szDate[64];
    char szAuthData[512];
    char szFullUrl[1024];
    char *szBaseUrl = "http://api.xf-yun.com/v1/private/s67c9c78c?host=%s&date=%s&authorization=%s";

    // 构造authorization请求参数
    datetime.format("GMT", szDate, sizeof(szDate));
    vGetAuth(pszAPPKey, pszAPPSecret, "api.xf-yun.com",
             "POST /v1/private/s67c9c78c HTTP/1.1", szDate,
             szAuthData, sizeof(szAuthData));

    // 构造完整的请求URL
    snprintf(szFullUrl, sizeof(szFullUrl), szBaseUrl, "api.xf-yun.com", szDate, szAuthData);
    LOG(EDEBUG, "url:%s", szFullUrl);

    // 构造body数据
    cstring_t *pBody = pstGenBody(pszAPPID, pszImagePath1, pszImagePath2);
    if (!pBody) {
        LOG(EERROR, "empty body error");
        return .0f;
    }
    // LOG(EDEBUG, "body:#%s#", pBody->str);

    // 构造需要的额外HTTP头
    const char szHeader[] = {
        "content-type: application/json\r\n"
    };

    // 发起HTTP Post语音识别的请求
    SEIHttpInfo_t stHttpInfo;
    bConnectHttpServer(&stHttpInfo, szFullUrl, szHeader,
                       (void *)pBody->peek(pBody), (int)pBody->length(pBody));
    LOG(EDEBUG, "status:%d", stHttpInfo.stResponse.iStatus);
    LOG(EDEBUG, "body:%s", stHttpInfo.stResponse.pstBody->sBuffer);

    // 获得人脸对比得分
    fScore = fGetScoreResult(stHttpInfo.stResponse.pstBody->sBuffer);

    // 关闭HTTP
    bHttpClose(&stHttpInfo);
    cstring_del(pBody);

    return fScore;
}
