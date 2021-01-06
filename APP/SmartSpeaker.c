#include <vad.h>
#include <EILog.h>
#include <date.h>
#include <cstring.h>
#include <EIHttpClient.h>
#include <AudioRecorder.h>
#include <tts.h>
#include <file.h>
#include <cJSON_user_define.h>
#include "SmartSpeaker.h"

#define APPID "5d2f27d2"

#define AIUI_API_KEY "a605c4712faefae730cc84b62c0eb92f"
#define AIUI_PARAM "{\"result_level\":\"plain\",\"auth_id\":\"27853aa9684eb19789b784a89ea5befd\",\"data_type\":\"audio\",\"sample_rate\":\"16000\",\"scene\":\"main_box\"}"

#define TTS_APISECRET "8110566cd9dd13066f9a1e38aeb12a48"
#define TTS_APIKEY "a8331910d59d41deea317a3c76d47b60"

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

#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
#pragma comment(lib, "winmm.lib")
void pcmPlay(const char *pathname)
{
    const int buf_size = 1024 * 1024 * 30;

    cstring_t *pFile = readFile(pathname);

    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag = WAVE_FORMAT_PCM;   //设置波形声音的格式
    wfx.nChannels = 1;                  //设置音频文件的通道数量
    wfx.nSamplesPerSec = 16000;         //设置每个声道播放和记录时的样本频率
    wfx.wBitsPerSample = 16;            //每隔采样点所占的大小
 
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
  
    HANDLE wait = CreateEvent(NULL, 0, 0, NULL);
    HWAVEOUT hwo;
    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, (DWORD_PTR)wait, 0L, CALLBACK_EVENT); //打开一个给定的波形音频输出装置来进行回放
 
    int data_size = 20480;
    char* data_ptr = pFile->str;
    WAVEHDR wh;
 
    while (data_ptr - pFile->str < pFile->len)
    {
        //这一部分需要特别注意的是在循环回来之后不能花太长的时间去做读取数据之类的工作，不然在每个循环的间隙会有“哒哒”的噪音
        wh.lpData = data_ptr;
        wh.dwBufferLength = data_size;
        wh.dwFlags = 0L;
        wh.dwLoops = 1L;
 
        data_ptr += data_size;
 
        waveOutPrepareHeader(hwo, &wh, sizeof(WAVEHDR)); //准备一个波形数据块用于播放
        waveOutWrite(hwo, &wh, sizeof(WAVEHDR)); //在音频媒体中播放第二个函数wh指定的数据
 
        WaitForSingleObject(wait, INFINITE); //等待
    }
    waveOutClose(hwo);
    CloseHandle(wait);
    cstring_del(pFile);
}
#elif defined(LINUX)
void pcmPlay(const char *pathname)
{
    const char *format = "aplay -r16000 -c1 -f S16_LE %s";
    int len = strlen(format) + strlen(pathname);
    char *cmd = (char *)malloc(len);
    if (!cmd) return;
    sprintf(cmd, format, pathname);
    system(cmd);
    if (cmd) free(cmd);
}
#else
#error "pcm play not support on this platform."
#endif

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