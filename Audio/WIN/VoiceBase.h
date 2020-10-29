
#ifndef _VOICE_BASE_H
#define _VOICE_BASE_H

#include "common.h"
#include <iostream>
#include <windows.h>
#include <mmsystem.h>


//Link with "winmm.lib" libraty.
#pragma comment(lib, "winmm.lib")

#define BYTE_LENGTH 8

/*
* Base class for the wave sound recorder
* and player classes.
*/
class CVoiceBase  
{
public:

	/**
	* Constructor.
	*/
	CVoiceBase();

	/**
	* Destructor.
	*/
	virtual ~CVoiceBase();

	/**
	* Set the audio format for that .wav sound.
	*/
	void SetFormat(DWORD nSamplesPerSec,  WORD  wBitsPerSample,WORD  nChannels);

	/**
	* Copy the buffer that contains the audio data.
	*/
	BOOL CopyBuffer(LPSTR lpBuffer, DWORD ntime);

	/**
	* Convert last error code(UINT) to Cstring;
	*/
	void GetMMResult(MMRESULT res);

	/**
	* Get the last error as Cstring.
	*/
	std::string GetLastError();

	/**
	* Release the memory that was allocated to the audio buffer.
	*/
	void destroyBuffer();

	/**
	* Allocate the memory for the buffer for audio data.
	*/
	BOOL PrepareBuffer(DWORD ntime);

	/**
	* Getter for the audio buffer pointer.
	*/
	char* getBuffer();

	/**
	* Get the number of bytes that a second of audio record takes.
	*/
	UINT getBytesPerSec();

	/**
	*
	*/
	void resetBuffer();


	//Return value of windows multimedia system as Cstring.
	std::string m_result;

	//MMRESULT is UINT that represents return value of mmsystem.
	MMRESULT res;

	//Buffer that contains the audio sample.
	char* buffer;

	//The WAVEHDR structure defines the header used to identify a waveform-audio buffer.
	WAVEHDR WaveHeader;

	//Parameters of the sound format and quality.
	WAVEFORMATEX PCMfmt;
};

#endif


/************************ End Of File ***************************/