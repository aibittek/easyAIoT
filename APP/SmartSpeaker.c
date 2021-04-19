#include <vad.h>
#include <EILog.h>
#include <date.h>
#include <cstring.h>
#include <EIHttpClient.h>
#include <AudioPlayer.h>
#include <AudioRecorder.h>
#include <tts.h>
#include <file.h>
#include <cJSON_user_define.h>
#include "SmartSpeaker.h"

#define APPID "xxxxxxxx"

#define AIUI_API_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define AIUI_PARAM "{\"result_level\":\"plain\",\"auth_id\":\"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\",\"data_type\":\"audio\",\"sample_rate\":\"16000\",\"scene\":\"main_box\"}"

#define TTS_APISECRET "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TTS_APIKEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

typedef struct record_data {
    cstring_t *pData;
    int iStartVad;
    int iEndVad;
}record_data_t;

static void RecordCB(void* pvHandle, int32_t iType, void* pvUserData, void* pvData, int32_t iLen)
{
    uint32_t bytesWritten = 0;
    static uint32_t totalWriten = 0;
    record_data_t *pPriData = (record_data_t *)pvUserData;

    if (AUDIO_DATA == iType) {
        int iRet = 0;
        for (uint32_t i=0; i<(iLen/2/VAD_FRAME_LEN); i++) {
            if(iRet = ucSimpleVad((int16_t *)(&pvData[VAD_FRAME_LEN*i]), VAD_FRAME_LEN)) {
                if (0 == pPriData->iStartVad) {
                    printf("检测到说话\n");
                }
                pPriData->iStartVad = 1;
            }
        }
        if (0x1 == iRet) {
            if (NULL == pPriData->pData) {
                pPriData->pData = cstring_create(iLen);
                cstring_init(pPriData->pData);
            }
            if (pPriData->pData) {
                pPriData->pData->appendStr(pPriData->pData, pvData, iLen);
            }
        }
        if (1 == pPriData->iStartVad && 0 == iRet) {
            pPriData->iStartVad = 0;
            printf("结束说话\n");
            Recorder.stop();
        }
    }
    else if (AUDIO_CLOSE == iType) {
        Recorder.stop();
    }
}

static cstring_t *getAudioData()
{
    record_data_t pPriData = {0};

    // 16000K采样，16bit采样深度，1是单声道，RecordCB是音频信息的回调函数，test.pcm是传递给回调的参数
    AudioConfig_t stAudioConfig = {16000, 16, 1, RecordCB, (void *)&pPriData};
    // 打开音频设备
    Recorder.open(&stAudioConfig);
    // 开始录音，阻塞函数，需要关闭录音在其他地方调用Recorder.close()
    Recorder.start();
    // 关闭录音设备
    Recorder.close();

    return pPriData.pData;
}

static cstring_t *getHeader(const char *appid, const char *apikey, const char *param)
{
    unsigned char psMD5[16] = {0};
    unsigned char pszMD5Dist[33] = {0};

    // 需要的额外头文件
    const char pszAIUIHeader[] = {
        "X-Appid: %s\r\n"
        "X-CurTime: %lu\r\n"
        "X-Param: %s\r\n"
        "X-CheckSum: %s\r\n"};

    // 获取系统当前NTP时间
    time_t dwTime = datetime.now();

    // 获取参数
    cstring_new_len(base64Param, strlen(param) * 2);
    iBase64Encode(param, base64Param->str, strlen(param));
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

static cstring_t *getJSONResult(const char *szResponse)
{
    int iRet;
    int iCode;
    char *pszDesc = NULL;
    char *content = NULL;

    cstring_new(result);

    JSON_DESERIALIZE_START(json_root, szResponse, iRet);
        JSON_DESERIALIZE_GET_INT(json_root, "code", iCode, iRet, JSON_CTRL_BREAK);
        JSON_DESERIALIZE_GET_STRING(json_root, "desc", pszDesc, iRet, JSON_CTRL_NULL);
        JSON_DESERIALIZE_GET_ARRAY(json_root, "data", json_array, iRet, JSON_CTRL_BREAK);
        JSON_DESERIALIZE_ARRAY_FOR_EACH_START(json_array, sub_item, pos, total);
            JSON_DESERIALIZE_GET_OBJECT(sub_item, "intent", intent_obj, iRet, JSON_CTRL_CONTINUE);
            JSON_DESERIALIZE_GET_STRING(intent_obj, "text", content, iRet, JSON_CTRL_NULL);
            // result->appendStr(result, text, strlen(text));
            JSON_DESERIALIZE_GET_ARRAY(intent_obj, "voice_answer", sub_voice_item, iRet, JSON_CTRL_CONTINUE);
            JSON_DESERIALIZE_ARRAY_FOR_EACH_START(sub_voice_item, voice_item, voice_pos, voice_total);
                JSON_DESERIALIZE_GET_STRING(voice_item, "content", content, iRet, JSON_CTRL_CONTINUE);
                LOG(ETRACE, "AIUI识别内容：%s", content);
                result->appendStr(result, content, strlen(content));
            JSON_DESERIALIZE_ARRAY_FOR_EACH_END();
        JSON_DESERIALIZE_ARRAY_FOR_EACH_END();
    JSON_DESERIALIZE_END(json_root, iRet);
    
    return result;
}

cstring_t *getAIUIResult(const char *appid, const char *apikey, const char *param)
{
    // 接口请求URL
    const char *pUrl = "http://openapi.xfyun.cn/v2/aiui";

    // 获取有效录音数据
    cstring_t *pAudio = getAudioData();

    // 获取AIUI请求头信息
    cstring_t *pHeader = getHeader(appid, apikey, param);

    // 发送Http请求
    SEIHttpInfo_t stHttpInfo;
    bConnectHttpServer(&stHttpInfo, pUrl, pHeader->str, pAudio->str, pAudio->len);

    // http返回状态和body内容
    LOG(EDEBUG, "status:%d", stHttpInfo.stResponse.iStatus);
    LOG(EDEBUG, "body:%s", stHttpInfo.stResponse.pstBody->sBuffer);

    // 返回结果数据
    cstring_t *result = NULL;
    if (200 == stHttpInfo.stResponse.iStatus) {
        result = getJSONResult(stHttpInfo.stResponse.pstBody->sBuffer);
    }

    // 释放不再使用的资源
    bHttpClose(&stHttpInfo);
    cstring_del(pAudio);
    cstring_del(pHeader);

    return result;
}

void testSmartSpeaker()
{
    const char *pathname = "tts.pcm";

    while (1) {
        // 获取AIUI请求结果
        cstring_t *pText = getAIUIResult(APPID, AIUI_API_KEY, AIUI_PARAM);
        LOG(EDEBUG, "text:%s", pText->str);

        // TTS语音合成
        getTTS(APPID, TTS_APIKEY, TTS_APISECRET, pText->str, pathname);

        // pcm音频文件播放
        pcmPlay(pathname);
        unlink(pathname);
        LOG(EDEBUG, "播放结束");

        cstring_del(pText);
    }
}