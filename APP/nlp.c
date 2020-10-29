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
#include "cJSON_user_define.h"

#define APPID "5d2f27d2"
#define AIUI_API_KEY "a605c4712faefae730cc84b62c0eb92f"
#define AIUI_PARAM "{\"result_level\":\"plain\",\"auth_id\":\"27853aa9684eb19789b784a89ea5befd\",\"data_type\":\"audio\",\"sample_rate\":\"16000\",\"scene\":\"main_box\"}"

static int iGetAIUIHeader(char buf[], int iLen)
{
    int iCount = 3;
    static char pszBase64[1024] = {0};
    static char pszCheckSum[1024] = {0};
    static unsigned char psMD5[16] = {0};
    static unsigned char pszMD5Dist[33] = {0};
    // AIUI需要的额外头文件
    const char pszAIUIHeader[] = {
        "X-Appid: %s\r\n"
        "X-CurTime: %lu\r\n"
        "X-Param: %s\r\n"
        "X-CheckSum: %s\r\n"
    };
    // 获取系统当前NTP时间
    time_t dwTime = 0;
    do {
        // dwTime = dwGetNTPtime("cn.ntp.org.cn");
        dwTime = dwGetNTPtime("ntp.aliyun.com");
        if (dwTime > 0) break;
    } while(iCount--);
    
    // unsigned long dwTime = mg_time();
    // 获取参数
    iBase64Encode(AIUI_PARAM, pszBase64, strlen(AIUI_PARAM));
    // 获取checksum
    snprintf(pszCheckSum, sizeof(pszCheckSum), "%s%lu%s", AIUI_API_KEY, dwTime, pszBase64);
    vMD5((uint8_t *)pszCheckSum, strlen(pszCheckSum), psMD5);
    for(int i=0; i<16; i++) {
        sprintf(pszMD5Dist+i*2, "%2.2x",  psMD5[i]);
    }
    return snprintf(buf, iLen, pszAIUIHeader, APPID, dwTime, pszBase64, pszMD5Dist);
}

#if defined(_WIN32)
static int iGetPostData(char buf[], int iLen)
{
    // 读取音频文件到buf
    int iRet, iCurSize = 0;
    FILE* fp = fopen("./test.pcm", "rb");
    if (fp) {
        while (iRet = fread(buf+iCurSize, 1, iLen, fp)) {
            if (iRet <= 0) break;
            else iCurSize += iRet;
        }
        fclose(fp);
        return iCurSize;
    }
    
    return -1;
}
#else
static EIString *g_pstAudio = NULL;
static int iStartVad = 0, iEndVad = 0;
static int iCaptureCallback(void *pvData, int iLen)
{
    int iRet = 0;
    for (uint32_t i=0; i<(iLen/2/VAD_FRAME_LEN); i++) {
        if(iRet = ucSimpleVad((int16_t *)(&pvData[VAD_FRAME_LEN*i]), VAD_FRAME_LEN)) {
            if (0 == iStartVad) {
                printf("检测到说话\n");
            }
            iStartVad = 1;
        }
    }
    if (0x1 == iRet) {
        if (NULL == g_pstAudio) {
            g_pstAudio = stEvIStringNew(0);
        }
        if (g_pstAudio) {
            stEIStringRealloc(g_pstAudio, pvData, iLen);
        }
    }
    if (1 == iStartVad && 0 == iRet) {
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
    if (g_pstAudio) {
        iMaxLen = (g_pstAudio->lSize > iLen)?iLen:g_pstAudio->lSize;
        memcpy(buf, g_pstAudio->sBuffer, iMaxLen);
        vEIStringDelete(g_pstAudio);
        g_pstAudio = NULL;
        return iMaxLen;
    } else {
        return 0;
    }
}
#endif

static int iGetResult(const char *szResponse, char *szResult, int iLen)
{
    int iRet;
    int iCode;
    char *szDesc;
    szResult[0] = '\0';
    JSON_SERIALIZE_START(json_root, szResponse, iRet);
        JSON_SERIALIZE_GET_INT(json_root, "code", iCode, iRet, 0);
        JSON_SERIALIZE_GET_STRING(json_root, "desc", szDesc, iRet, 0);
        JSON_SERIALIZE_GET_ARRAY(json_root, "data", json_array, iRet, 1);
        JSON_SERIALIZE_ARRAY_FOR_EACH_START(json_array, sub_item, pos, total);
            JSON_SERIALIZE_GET_STRING_COPY(sub_item, "text", szResult+strlen(szResult), iLen-strlen(szResult), iRet, 0);
        JSON_SERIALIZE_ARRAY_FOR_EACH_END();
    JSON_SERIALIZE_END(json_root, iRet);
    return iRet;
}

static int iGetNLPResult(const char *szResponse, char *szResult, int iLen)
{
    int iRet;
    int iCode;
    char *pszDesc = NULL;
    char szText[1024] = {0};
    szResult[0] = '\0';
    LOG(EDEBUG, "%s\n", szResponse);
    JSON_SERIALIZE_START(json_root, szResponse, iRet);
        JSON_SERIALIZE_GET_INT(json_root, "code", iCode, iRet, JSON_CTRL_BREAK);
        JSON_SERIALIZE_GET_STRING(json_root, "desc", pszDesc, iRet, JSON_CTRL_NULL);
        JSON_SERIALIZE_GET_ARRAY(json_root, "data", json_array, iRet, JSON_CTRL_BREAK);
        JSON_SERIALIZE_ARRAY_FOR_EACH_START(json_array, sub_item, pos, total);
            JSON_SERIALIZE_GET_OBJECT(sub_item, "intent", intent_obj, iRet, JSON_CTRL_CONTINUE);
            JSON_SERIALIZE_GET_STRING_COPY(intent_obj, "text", szText, sizeof(szText), iRet, JSON_CTRL_NULL);
            printf("识别内容：%s\n", szText);
            JSON_SERIALIZE_GET_ARRAY(intent_obj, "voice_answer", sub_voice_item, iRet, JSON_CTRL_CONTINUE);
            JSON_SERIALIZE_ARRAY_FOR_EACH_START(sub_voice_item, voice_item, voice_pos, voice_total);
                JSON_SERIALIZE_GET_STRING_COPY(voice_item, "content", szResult+strlen(szResult), iLen-strlen(szResult), iRet, JSON_CTRL_CONTINUE);
            JSON_SERIALIZE_ARRAY_FOR_EACH_END();
        JSON_SERIALIZE_ARRAY_FOR_EACH_END();
    JSON_SERIALIZE_END(json_root, iRet);
    
    return iRet;
}

int iNLPDemo(char *pszNLPResult, int iLen)
{
    char pszUrl[1024] = "http://openapi.xfyun.cn/v2/aiui";
    char szHeader[4096] = {0};
    char szResponse[10240] = {0};
    char sBody[204800] = {0};
    char szResult[1024];

    // 1. 获取当前目录下的test.pcm语音数据到sBody中
    // EIString *pstBody = pstGetPostData("./test.pcm");
    printf("开始获取录音数据\n");
    size_t dwBodyLen = iGetPostData(sBody, sizeof(sBody));

    // 2. 生成AI平台需要的额外HTTP头
    iGetAIUIHeader(szHeader, sizeof(szHeader));

    // 3. 发送HTTP Post语音识别的请求
    SEIHttpInfo_t stHttpInfo;
    bConnectHttpServer(&stHttpInfo, pszUrl, szHeader, sBody, dwBodyLen);
    // LOG(EDEBUG, "status:%d", stHttpInfo.stResponse.iStatus);
    LOG(EERROR, "body:%s", stHttpInfo.stResponse.pstBody->sBuffer);
    
    // 4. 获取需要的返回结果
    iGetNLPResult(stHttpInfo.stResponse.pstBody->sBuffer, pszNLPResult, iLen);
    printf("识别结果：%s\n", pszNLPResult);

    // 5. 关闭HTTP
    bHttpClose(&stHttpInfo);

    return stHttpInfo.stResponse.iStatus;
}

// int main(void) {
//     char szNLPResult[1024];
//     iNLPDemo(szNLPResult, sizeof(szNLPResult));
//     return 0;
// }
