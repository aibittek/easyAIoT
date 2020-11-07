/**
 * @file AudioRecorder.h
 * @brief 音频录音应用层接口
 * @author likui 55239610@qq.com
 * @version 1.0
 * @date 2020-11-07
 * 
 * @copyright MIT (c) 2020-2021 likui
 * 
 */
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
/**
 * @brief 录音回调
 * @param pvHandle      录音对象，AudioRecorder_t类型的指针
 * @param iType         录音状态，AUDIO_OPEN表示录音设备打开成功，AUDIO_DATA表示录音数据
 * @param pvUserData    用户传递到回调的私有数据
 * @param pvData        录音数据，当iType=AUDIO_DATA时有效
 * @param iLen          录音数据长度，当iType=AUDIO_DATA时有效
 */
typedef void    (*RecordCallback)(void* pvHandle, int32_t iType, void* pvUserData, void* pvData, int32_t iLen);

/**
 * @brief 录音配置结构体
 */
typedef struct AudioConfig {
    uint32_t                iSampleRate;        /* 每秒采样次数 */
    uint8_t                 byBitsPerSample;    /* 每次采样深度 */
    uint8_t                 byChannels;         /* 采样通道 */
    RecordCallback          pfCallback;         /* 获取音频数据的回调 */
    void*                   pvUserData;         /* 传递给回调的参数 */
}AudioConfig_t;

/**
 * @brief 录音功能结构体
 */
typedef struct AudioRecorder {
    /**
     * @brief 打开录音设备
     * @param   pAudioConfig    录音配置信息     
     * @return  bool            false打开失败，true打开成功 
     */
    bool (*open)(AudioConfig_t* pAudioConfig);
    /**
     * @brief 开始流式录音，录音信息通过open时注册的回调函数返回给用户
     */
    bool (*start)(void);
    /**
     * @brief 停止录音，异步
     */
    void (*stop)(void);
    /**
     * @brief 关闭录音设备
     */
    void (*close)(void);
    AudioHandle             pvAudioHandle;      /* 音频设备的句柄 */
    AudioConfig_t           stAudioConfig;      /* 音频设备的配置 */
    bool                    bRecording;         /* 是否正在录音 */
}AudioRecorder_t;

/**
 * @brief 录音对象单例
 */
extern AudioRecorder_t Recorder;

#ifdef __cplusplus
}
#endif

#endif