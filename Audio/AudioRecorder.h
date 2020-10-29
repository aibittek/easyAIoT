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

/// 音频配置
typedef struct AudioConfig {
    uint32_t                iSampleRate;        /* 每秒采样次数 */
    uint8_t                 byBitsPerSample;    /* 每次采样深度 */
    uint8_t                 byChannels;         /* 采样通道 */
}AudioConfig_t;

typedef struct AudioRecorder {
    bool (*open)(struct AudioRecorder* pAudioConfig, RecordCallback cb, void* pvUserData);
    bool (*start)();
    void (*close)();
    AudioHandle             pvAudioHandle;
    AudioConfig_t           stAudioConfig;
}AudioRecorder_t;

extern AudioRecorder_t Recorder;

#ifdef __cplusplus
}
#endif

#endif