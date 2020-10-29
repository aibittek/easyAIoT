
#include "VoiceRecording.h"


CVoiceRecording::CVoiceRecording()
{
	hWaveIn=NULL;
	hRecordIsDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CVoiceRecording::~CVoiceRecording()
{
	if (IsOpen())
	{
		Close();
	}
	CloseHandle(hRecordIsDoneEvent);
}

BOOL CVoiceRecording::Record()
{
	//The waveInPrepareHeader function prepares a buffer for waveform-audio input.
	res=waveInPrepareHeader(hWaveIn,&WaveHeader,sizeof(WAVEHDR));
	GetMMResult(res);
	if (res != MMSYSERR_NOERROR)
	{
		return FALSE;
	}
	
	//The waveInAddBuffer function sends an input buffer to the given 
	//waveform-audio input device. 
	//When the buffer is filled, the application is notified.
	res=waveInAddBuffer(hWaveIn,&WaveHeader,sizeof(WAVEHDR));
	GetMMResult(res);
	if (res != MMSYSERR_NOERROR)
	{
		return FALSE;
	}
	//The waveInStart function starts input on the given waveform-audio input device.
	res = waveInStart(hWaveIn) ;
	GetMMResult(res);
	if (res != MMSYSERR_NOERROR)
	{
		debug_log(GetLastError());
		return FALSE;
	}

	WaitForSingleObject(hRecordIsDoneEvent, INFINITE);
	return TRUE;
}

BOOL CVoiceRecording::Open()
{
	if (IsOpen())
	{
		return FALSE;
	}
	res=waveInOpen(&hWaveIn, (UINT) WAVE_MAPPER, &PCMfmt, 
		          (DWORD) VoiceWaveInProc, (DWORD) this, CALLBACK_FUNCTION);
	GetMMResult(res);
	if (res!=MMSYSERR_NOERROR)
	{
		debug_log(GetLastError());
		hWaveIn=NULL;
		return FALSE;
	}
	else
		return TRUE;
}

BOOL CVoiceRecording::Close()
{
	res=waveInClose (hWaveIn);
	GetMMResult(res);
	if (res != MMSYSERR_NOERROR)
	{
		debug_log(GetLastError());
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL CVoiceRecording::IsOpen()
{
	if (hWaveIn != NULL)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CALLBACK VoiceWaveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg==WIM_DATA)
	{
		CVoiceRecording* pVoice=(CVoiceRecording*) dwInstance;
		
		pVoice->res=waveInUnprepareHeader(pVoice->hWaveIn, &pVoice->WaveHeader, sizeof(WAVEHDR));
		pVoice->GetMMResult(pVoice->res);
		
		if (pVoice->res != MMSYSERR_NOERROR)
		{
			debug_log(GetLastError());
		}
		SetEvent(pVoice->hRecordIsDoneEvent);
	}
}

