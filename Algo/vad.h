#ifndef _VAD_H
#define _VAD_H

#include "EIPlatform.h"

//一帧数据设置位144个采样288Byte,一个(AUDIO_BUFFER_SIZE/2)=1152*2,等于8帧
#define VAD_FRAME_LEN	(144U)
//一秒16000次采样，144次采样(即一帧)用时36ms
uint8_t ucSimpleVad(const int16_t *psVocBuf, const uint32_t ulBufLen);

#endif