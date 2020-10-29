
#include "VoiceBase.h"
#include "WavHeader.h"

CVoiceBase::CVoiceBase()
{
	//Size, in bytes, of extra information appended to the end of the struct.
   	PCMfmt.cbSize=0;    
	//Format of sound. PCM == Pulse Code Modulation.
	PCMfmt.wFormatTag=WAVE_FORMAT_PCM;   
	//Number of sound channels.
	PCMfmt.nChannels = SOUND_CHANNELS;
	//Audio sample rate.                
 	PCMfmt.nSamplesPerSec = SAMPLES_PER_SEC;
	//In PCM the bit width must be 8 or 16.
	PCMfmt.wBitsPerSample= BITS_PER_SAMPLE;
	//In PCM nBlockAlign = nChannels � wBitsPerSample.
	PCMfmt.nBlockAlign = SOUND_CHANNELS * BITS_PER_SAMPLE / BYTE_LENGTH;
	//In PCM: nAvgBytesPerSec= nSamplesPerSec � nBlockAlign.
	PCMfmt.nAvgBytesPerSec = SAMPLES_PER_SEC * PCMfmt.nBlockAlign;

	buffer=nullptr;
}

CVoiceBase::~CVoiceBase()
{
	destroyBuffer();
}

BOOL CVoiceBase::PrepareBuffer(DWORD sampleTimeSec)
{
   if (buffer!=NULL)
   {
	   delete [] buffer;
	   buffer=NULL;
   }
   
   //Calculate the needed buffer length, and allocate mamory.
   DWORD length= getBytesPerSec() * sampleTimeSec;

   buffer=new char[length];
   if (buffer==NULL)
 	{
 		return FALSE;
 	}

   //Pointer to the buffer with the sound content.
   WaveHeader.lpData=buffer;
   //Buffer length.
   WaveHeader.dwBufferLength=length;
   //Recorded zero bytes yet.
   WaveHeader.dwBytesRecorded=0;

   //Not used flags. Set to default values.
   WaveHeader.dwUser=0;
   WaveHeader.dwFlags=0;
   WaveHeader.reserved=0;
   WaveHeader.lpNext=0;

   return TRUE;
}

void CVoiceBase::destroyBuffer()
{
	if (buffer != NULL)
	{
		delete [] buffer;
		buffer=NULL;
	}
}

void CVoiceBase::resetBuffer()
{
	buffer = NULL;
}

void  CVoiceBase::GetMMResult(MMRESULT result)
{
	switch (result)
	{
	case MMSYSERR_ALLOCATED: 
		m_result="Specified resource is already allocated.";
		break;
		
	case MMSYSERR_BADDEVICEID:
		m_result="Specified device identifier is out of range.";
		break;
		
	case MMSYSERR_NODRIVER:
		m_result="No device driver is present. ";
		break;
		
	case MMSYSERR_NOMEM:
		m_result="Unable to allocate or lock memory. ";
		break;
		
	case WAVERR_BADFORMAT:
		m_result="Attempted to open with an unsupported waveform-audio format.";
		break;
		
	case WAVERR_UNPREPARED:
		m_result="The buffer pointed to by the pwh parameter hasn't been prepared. ";
		break;
		
	case WAVERR_SYNC:
		m_result="The device is synchronous but waveOutOpen was called"
			"without using the WAVE_ALLOWSYNC flag. ";
		break;
		
	case WAVERR_STILLPLAYING:
		m_result="The buffer pointed to by the pwh parameter is still in the queue.";
		break;
		
	case MMSYSERR_NOTSUPPORTED:
		m_result="Specified device is synchronous and does not support pausing. ";
		break;
		
	case MMSYSERR_NOERROR:
		break;
		
	default:
		m_result="Unspecified error";
	}
}

std::string CVoiceBase::GetLastError()
{
	return m_result;
}

BOOL CVoiceBase::CopyBuffer(LPSTR lpBuffer,DWORD ntime)
{
	if (lpBuffer == NULL)
	{
		return FALSE;
	}
	DWORD length= ntime * getBytesPerSec();
	memcpy(lpBuffer, buffer, length);
	return TRUE;
}

void CVoiceBase::SetFormat( DWORD nSamplesPerSec,  WORD  wBitsPerSample,WORD  nChannels)
{
   	PCMfmt.cbSize=0;
	PCMfmt.wFormatTag=WAVE_FORMAT_PCM;
	PCMfmt.nChannels=nChannels;
	PCMfmt.nSamplesPerSec=nSamplesPerSec;
	PCMfmt.wBitsPerSample=wBitsPerSample;
	PCMfmt.nBlockAlign=nChannels*wBitsPerSample/ BYTE_LENGTH;
	PCMfmt.nAvgBytesPerSec=nSamplesPerSec*nChannels*wBitsPerSample/ BYTE_LENGTH;
}

char* CVoiceBase::getBuffer()
{
	return buffer;
}

UINT CVoiceBase::getBytesPerSec()
{
	UINT bytePerSec = (PCMfmt.nSamplesPerSec)*(PCMfmt.nChannels) * 
		              (PCMfmt.wBitsPerSample) / (BYTE_LENGTH);
	return bytePerSec;
}
