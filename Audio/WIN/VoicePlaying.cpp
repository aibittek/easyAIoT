
#include "VoicePlaying.h"

CVoicePlaying::CVoicePlaying()
{
	hWaveOut=NULL;
	mAudioPlayIsDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CVoicePlaying::~CVoicePlaying()
{
	if (IsOpen())
	{
        Close();
	}
	CloseHandle(mAudioPlayIsDoneEvent);
}

BOOL CVoicePlaying::Play()
{
	res = waveOutPrepareHeader (hWaveOut,&WaveHeader,sizeof(WaveHeader));
    GetMMResult(res);
	if (res != MMSYSERR_NOERROR)
	{
		debug_log(GetLastError());
		return FALSE;
	}
		
	res = waveOutWrite(hWaveOut,&WaveHeader, sizeof(WaveHeader));
	GetMMResult(res);
	if (res != MMSYSERR_NOERROR)
	{
		debug_log(GetLastError());
		return FALSE;
	}
	WaitForSingleObject(mAudioPlayIsDoneEvent, INFINITE);

	return TRUE;
}

BOOL CVoicePlaying::Open()
{
	if (IsOpen())
	{
		return FALSE;
	}

	res=waveOutOpen (&hWaveOut,(UINT)WAVE_MAPPER,&PCMfmt,
                     (DWORD)VoiceWaveOutProc, (DWORD)this, CALLBACK_FUNCTION);

	GetMMResult(res);
	if (res!=MMSYSERR_NOERROR)
	{
		debug_log(GetLastError());
		hWaveOut=NULL;
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL CVoicePlaying::Close()
{
	res=waveOutClose(hWaveOut);
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

BOOL CVoicePlaying::IsOpen()
{
  if(hWaveOut!=NULL)
	 return TRUE;
  else
     return FALSE;
}

void CALLBACK VoiceWaveOutProc(HWAVEOUT hwo, 
	                           UINT uMsg, 
	                           DWORD dwInstance, 
	                           DWORD dw1, 
	                           DWORD dw2)
{
	
	CVoicePlaying* pVoice = (CVoicePlaying*)dwInstance;
	if (uMsg == WOM_DONE)
	{
		pVoice->res = waveOutUnprepareHeader(pVoice->hWaveOut,
			                                 &pVoice->WaveHeader, 
			                                 sizeof(WAVEHDR));
		pVoice->GetMMResult(pVoice->res);

		if (pVoice->res != MMSYSERR_NOERROR)
		{
			debug_log(pVoice->GetLastError());
		}
		SetEvent(pVoice->mAudioPlayIsDoneEvent);
	}
	
}
