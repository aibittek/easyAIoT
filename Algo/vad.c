#include "EILog.h"
#include "vad.h"

//一帧数据设置位144个采样288Byte,一个(AUDIO_BUFFER_SIZE/2)=1152*2,等于8帧
#define VAD_FRAME_LEN	(144U)
//一秒16000次采样，144次采样(即一帧)用时36ms

//简单的进行语音端点检测，输入的数据只能是单声道的PCM
//ulBufLen指的是16位的长度（两个字节）
//目前使用的是右声道的参数，如果想用左声道，参数还要调过
uint8_t ucSimpleVad(const int16_t *psVocBuf, const uint32_t ulBufLen)
{
	static uint8_t usVocSta = 0;		//语音状态机
	static uint32_t usScnt = 0;		//连续静音帧数计数器
	
    //------------------------------------------------------------------------------
	uint32_t ulvEnergy = 0;				//短时能量
	uint32_t ulvZerorate = 0;			//短时过零率
	for(uint32_t i=0; i<ulBufLen; i++)
	{
		//计算短时能量
		ulvEnergy += (psVocBuf[i] > 0) ? (psVocBuf[i]) : (-psVocBuf[i]);
		//计算短时过零率
		if(psVocBuf[i]*psVocBuf[i+1] < 0)
		{
			ulvZerorate++;
		}
	}
    //------------------------------------------------------------------------------
	if (ulvEnergy>50000 && ulvZerorate>10)//这一帧数据达到语音帧的标准
	{
		usVocSta = 1;		//置状态机位语音态
		usScnt = 0;			//静音帧数量统计清零
	}
	else if (ulvEnergy<10000 || ulvZerorate<1)//这一帧数据没达到语音帧的标准
	{
		//如果状态机还在语音态且静音帧的数量在60帧以内
		if (usScnt < 60 && usVocSta == 1)	//18*60ms
		{
			usScnt++;						//静音帧数量加一
		}
		else
		{
			usVocSta = 0;					//置状态机位静音态
			usScnt = 0;						//静音统计清零
		}
	}
	else
	{
	}
	//LOG(EDEBUG, "Ener:%d\r\nuZero:%d", ulvEnergy, ulvZerorate);
	return usVocSta;
}
