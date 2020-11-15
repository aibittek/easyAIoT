#include <stdint.h>
#include <stdio.h>
#include "ntp.h"
#include "EILog.h"
#include "nlp.h"
#include "EIHttpClient.h"
#include "EasyIoT.h"
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

static cstring_t *getNlpHeader(const char *pszAppid, const char *pszKey, const char *pszParam)
{
    unsigned char psMD5[16] = {0};
    unsigned char pszMD5Dist[33] = {0};

    // AIUI需要的额外头文件
    const char pszAIUIHeader[] = {
        "X-Appid: %s\r\n"
        "X-CurTime: %lu\r\n"
        "X-Param: %s\r\n"
        "X-CheckSum: %s\r\n"};

    // 获取系统当前NTP时间
    time_t dwTime = datetime.now();

    // 获取参数
    cstring_new_len(base64Param, strlen(pszParam) * 2);
    iBase64Encode(pszParam, base64Param->str, strlen(pszParam));
    base64Param->len = strlen(base64Param->str);

    // 获取checksum
    int size = strlen(pszKey) + sizeof(dwTime) * 8 + base64Param->len;
    cstring_new_len(strCheckSum, size);
    snprintf(strCheckSum->str, size, "%s%lu%s", pszKey, dwTime, base64Param->str);
    strCheckSum->len = strlen(strCheckSum->str);

    vMD5((uint8_t *)strCheckSum->str, strlen(strCheckSum->str), psMD5);
    for (int i = 0; i < 16; i++)
    {
        sprintf(pszMD5Dist + i * 2, "%2.2x", psMD5[i]);
    }

    // 构造Header
    size = strlen(pszAIUIHeader) + strlen(pszAppid) + sizeof(dwTime) * 8 +
           base64Param->len + sizeof(pszMD5Dist);

    cstring_new_len(strHeader, size);
    snprintf(strHeader->str, size, pszAIUIHeader, pszAppid, dwTime, base64Param->str, pszMD5Dist);
    strHeader->len = strlen(strHeader->str);

    cstring_del(base64Param);
    cstring_del(strCheckSum);

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

    JSON_SERIALIZE_START(json_root, szResponse, iRet);
    JSON_SERIALIZE_GET_INT(json_root, "code", iCode, iRet, JSON_CTRL_BREAK);
    JSON_SERIALIZE_GET_STRING(json_root, "desc", pszDesc, iRet, JSON_CTRL_NULL);
    JSON_SERIALIZE_GET_ARRAY(json_root, "data", json_array, iRet, JSON_CTRL_BREAK);
    JSON_SERIALIZE_ARRAY_FOR_EACH_START(json_array, sub_item, pos, total);
    JSON_SERIALIZE_GET_OBJECT(sub_item, "intent", intent_obj, iRet, JSON_CTRL_CONTINUE);
    JSON_SERIALIZE_GET_STRING(intent_obj, "text", pszText, iRet, JSON_CTRL_NULL);
    LOG(ETRACE, "识别内容：%s", pszText);
    JSON_SERIALIZE_GET_ARRAY(intent_obj, "voice_answer", sub_voice_item, iRet, JSON_CTRL_CONTINUE);
    JSON_SERIALIZE_ARRAY_FOR_EACH_START(sub_voice_item, voice_item, voice_pos, voice_total);
    JSON_SERIALIZE_GET_STRING(voice_item, "content", pszText, iRet, JSON_CTRL_CONTINUE);
    if (pszText)
    {
        result->appendStr(result, pszText, strlen(pszText));
    }
    JSON_SERIALIZE_ARRAY_FOR_EACH_END();
    JSON_SERIALIZE_ARRAY_FOR_EACH_END();
    JSON_SERIALIZE_END(json_root, iRet);

    if (0 == result->length(result))
    {
        cstring_del(result);
    }

    return result;
}

cstring_t *getNlpResult(const char *pszAppid, const char *pszKey, const char *pszParam,
                        void *pAudioData, int iAudioLen)
{
    if (!pszAppid || !pszKey || !pszParam || !pAudioData || iAudioLen <= 0)
    {
        return NULL;
    }
    char pszUrl[1024] = "http://openapi.xfyun.cn/v2/aiui";

    // 生成头文件
    cstring_t *pHeader = getNlpHeader(pszAppid, pszKey, pszParam);

    // 发送HTTP Post语音识别的请求
    SEIHttpInfo_t stHttpInfo;
    bConnectHttpServer(&stHttpInfo, pszUrl, pHeader->str, pAudioData, iAudioLen);
    LOG(EDEBUG, "status:%d", stHttpInfo.stResponse.iStatus);
    LOG(EDEBUG, "body:%s", stHttpInfo.stResponse.pstBody->sBuffer);

    // 获取需要的返回结果
    cstring_t *pResult = getJsonResult(stHttpInfo.stResponse.pstBody->sBuffer);

    // 关闭HTTP
    bHttpClose(&stHttpInfo);

    // 释放资源
    cstring_del(pHeader);

    return pResult;
}

void vTestNlp()
{
    const char *pszAppid = "5d2f27d2";
    const char *pszKey = "a605c4712faefae730cc84b62c0eb92f";
    const char *pszParam = "{\"result_level\":\"plain\",\"auth_id\":\"27853aa9684eb19789b784a89ea5befd\",\"data_type\":\"audio\",\"sample_rate\":\"16000\",\"scene\":\"main_box\"}";

    cstring_t *pAudioData = readFile("../Res/test.pcm");
    if (!pAudioData)
    {
        LOG(EERROR, "read audio file error");
        return;
    }
    LOG(EDEBUG, "pcm len:%d", pAudioData->length(pAudioData));

    cstring_t *pResult = getNlpResult(pszAppid, pszKey, pszParam, pAudioData->str, pAudioData->len);
    if (pResult)
    {
        LOG(EDEBUG, "识别结果：%s", pResult->str);
        cstring_del(pResult);
    }

    cstring_del(pAudioData);
}
