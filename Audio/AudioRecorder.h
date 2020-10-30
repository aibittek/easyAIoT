#ifndef _AUDIO_RECORDER_H
#define _AUDIO_RECORDER_H

#include <EIPlatform.h>

#define AUDIO_OPEN      0
#define AUDIO_DATA      1
#define AUDIO_CLOSE     2

#ifdef __cplusplus
extern "C" {
#endif

typedef void*   AudioHandle;
typedef void    (*RecordCallback)(void* pvHandle, int32_t iType, void* pvUserData, void* pvData, int32_t iLen);

/// 音频配置信息
typedef struct AudioConfig {
    uint32_t                iSampleRate;        /* 每秒采样次数 */
    uint8_t                 byBitsPerSample;    /* 每次采样深度 */
    uint8_t                 byChannels;         /* 采样通道 */
    RecordCallback          pfCallback;         /* 获取音频数据的回调 */
    void*                   pvUserData;         /* 传递给回调的参数 */
}AudioConfig_t;

/// 录音器主结构体
typedef struct AudioRecorder {
    bool (*open)(AudioConfig_t* pAudioConfig);
    bool (*start)(void);
    void (*stop)(void);
    void (*close)(void);
    AudioHandle             pvAudioHandle;      /* 音频设备的句柄 */
    AudioConfig_t           stAudioConfig;      /* 音频设备的配置 */
}AudioRecorder_t;

extern AudioRecorder_t Recorder;

#ifdef __cplusplus
}
#endif

#endif