#include <stdint.h>
#include <stdio.h>
#include "ntp.h"
#include "EILog.h"
#include "its.h"
#include "EIHttpClient.h"
#include "base64.h"
#include "md5.h"
#if defined(UNIX)
#include "tinycap.h"
#endif
#include "vad.h"
#include <cstring.h>
#include <date.h>
#include <file.h>
#include "cJSON_user_define.h"

static cstring_t *getITSHeader(const char *pszAppid, const char *pszKey, const char *pszSecret, cstring_t *body)
{
    char szDate[64];
    char Digest[128] = {0};
    char Sha256Digest[64] = {0};
    char szAuth[1024] = {0};
    unsigned char psMD5[16] = {0};
    unsigned char pszMD5Dist[33] = {0};

    // AIUI需要的额外头文件
    const char pszAIUIHeader[] = {
        "Date: %s\r\n"
        "Digest: SHA-256=%s\r\n"
        "Authorization: %s\r\n"
        "Content-Type:application/json\r\n"
        "Accept:application/json,version=1.0\r\n"};

    // 获取系统当前NTP时间
    datetime.format("GMT", szDate, sizeof(szDate));

    // 获取参数
    cstring_new_len(base64digest, strlen(body->str) * 2);
    sha256(body->str, body->len, Sha256Digest);
    iBase64Encode(Sha256Digest, Digest, 64);

    // 获取Auth字段
    vGetAuth(pszKey, pszSecret, "itrans.xfyun.cn",
             "GET /v2/its HTTP/1.1", szDate, szAuth, sizeof(szAuth));

    // 构造Header
    int size = strlen(pszAIUIHeader) + strlen(pszAppid) + sizeof(szDate) +
           strlen(szAuth) + sizeof(Digest);

    cstring_new_len(strHeader, size);
    snprintf(strHeader->str, size, pszAIUIHeader, pszAppid, szDate, szAuth, strlen(szAuth));
    strHeader->len = strlen(strHeader->str);

    cstring_del(base64digest);

    return strHeader;
}

#if 0
// 获取实时录音数据
static EIString *g_pstAudio = NULL;
static int iStartVad = 0, iEndVad = 0;
static int iCaptureCallback(void *pvData, int iLen)
{
    int iRet = 0;
    for (uint32_t i = 0; i < (iLen / 2 / VAD_FRAME_LEN); i++)
    {
        if (iRet = ucSimpleVad((int16_t *)(&pvData[VAD_FRAME_LEN * i]), VAD_FRAME_LEN))
        {
            if (0 == iStartVad)
            {
                printf("检测到说话\n");
            }
            iStartVad = 1;
        }
    }
    if (0x1 == iRet)
    {
        if (NULL == g_pstAudio)
        {
            g_pstAudio = stEvIStringNew(0);
        }
        if (g_pstAudio)
        {
            stEIStringRealloc(g_pstAudio, pvData, iLen);
        }
    }
    if (1 == iStartVad && 0 == iRet)
    {
        iStartVad = 0;
        printf("结束说话\n");
        return 1;
    }
    return 0;
}
static int iGetPostData(char buf[], int iLen)
{
    int iMaxLen;
    vStartCapture(iCaptureCallback);
    if (g_pstAudio)
    {
        iMaxLen = (g_pstAudio->lSize > iLen) ? iLen : g_pstAudio->lSize;
        memcpy(buf, g_pstAudio->sBuffer, iMaxLen);
        vEIStringDelete(g_pstAudio);
        g_pstAudio = NULL;
        return iMaxLen;
    }
    else
    {
        return 0;
    }
}
#endif

static cstring_t *getJsonResult(const char *szResponse)
{
    int iRet;
    int iCode;
    char *pszDesc = NULL;
    char *pszText = NULL;
    cstring_new(result);

    JSON_DESERIALIZE_START(json_root, szResponse, iRet);
    JSON_DESERIALIZE_GET_INT(json_root, "code", iCode, iRet, JSON_CTRL_BREAK);
    JSON_DESERIALIZE_GET_STRING(json_root, "desc", pszDesc, iRet, JSON_CTRL_NULL);
    JSON_DESERIALIZE_GET_ARRAY(json_root, "data", json_array, iRet, JSON_CTRL_BREAK);
    JSON_DESERIALIZE_ARRAY_FOR_EACH_START(json_array, sub_item, pos, total);
    JSON_DESERIALIZE_GET_OBJECT(sub_item, "intent", intent_obj, iRet, JSON_CTRL_CONTINUE);
    JSON_DESERIALIZE_GET_STRING(intent_obj, "text", pszText, iRet, JSON_CTRL_NULL);
    LOG(ETRACE, "识别内容：%s", pszText);
    JSON_DESERIALIZE_GET_ARRAY(intent_obj, "voice_answer", sub_voice_item, iRet, JSON_CTRL_CONTINUE);
    JSON_DESERIALIZE_ARRAY_FOR_EACH_START(sub_voice_item, voice_item, voice_pos, voice_total);
    JSON_DESERIALIZE_GET_STRING(voice_item, "content", pszText, iRet, JSON_CTRL_CONTINUE);
    if (pszText)
    {
        result->appendStr(result, pszText, strlen(pszText));
    }
    JSON_DESERIALIZE_ARRAY_FOR_EACH_END();
    JSON_DESERIALIZE_ARRAY_FOR_EACH_END();
    JSON_DESERIALIZE_END(json_root, iRet);

    if (0 == result->length(result))
    {
        cstring_del(result);
    }

    return result;
}

cstring_t *getITSResult(const char *pszAppid, const char *pszKey, const char *pszSecret, const char *pData)
{
    if (!pszAppid || !pszKey || !pszSecret || !pData)
    {
        return NULL;
    }
    char pszUrl[1024] = "http://itrans.xfyun.cn/v2/its";

    // 获取body数据
    cstring_new_len(body, strlen(pData)*2);
    iBase64Encode(pData, strlen(pData), body->str);
    body->len = strlen(body->str);

    // 生成头文件
    cstring_t *pHeader = getITSHeader(pszAppid, pszKey, pszSecret, body);

    // 发送HTTP Post语音识别的请求
    SEIHttpInfo_t stHttpInfo;
    bConnectHttpServer(&stHttpInfo, pszUrl, pHeader->str, body->str, body->len);
    LOG(EDEBUG, "status:%d", stHttpInfo.stResponse.iStatus);
    LOG(EDEBUG, "result:%s", stHttpInfo.stResponse.pstBody->sBuffer);

    // 关闭HTTP
    bHttpClose(&stHttpInfo);

    // 释放资源
    cstring_del(pHeader);

    return NULL;
}
