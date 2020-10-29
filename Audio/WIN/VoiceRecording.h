
#ifndef _VOICE_RECORDING_H
#define _VOICE_RECORDING_H

#include "VoiceBase.h"

void CALLBACK VoiceWaveInProc(
						 HWAVEIN hwi,       
						 UINT uMsg,         
						 DWORD dwInstance,  
						 DWORD dwParam1,    
						 DWORD dwParam2     
						 );


/**
* Class that used to record voice from voice input device.
*
*/
class CVoiceRecording : public CVoiceBase  
{
public:
	/**
	* Constructor.
	*/
	CVoiceRecording();
	/*
	* Destructor.
	*/
	virtual ~CVoiceRecording();
	
	/**
	* Check if Wav handle is open. 
	*/
	BOOL IsOpen();

	/**
	* Close Wav handle.
	*/
	BOOL Close();

	/**
	* Open Wav handle.
	*/
	BOOL Open();

	/**
	* Record audio.
	*/
    BOOL Record();

	// Wav input handle.
    HWAVEIN hWaveIn;

	// Event that is signaled when audio record is done.
	HANDLE hRecordIsDoneEvent;

};

#endif


/************************ End Of File **************************/