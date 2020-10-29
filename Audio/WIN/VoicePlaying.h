
#ifndef _VIOCE_PLAYING_H
#define _VIOCE_PLAYING_H

#include "VoiceBase.h"

/*
* Callback function that us called when the playing is ended.
*/
void CALLBACK VoiceWaveOutProc(HWAVEOUT hwi, UINT uMsg,  DWORD dwInstance, DWORD dw1, DWORD dw2);

class CVoicePlaying : public CVoiceBase  
{
public:

	/*
	* Constructor.
	*/
	CVoicePlaying();

	/*
	* Destructor.
	*/
	virtual ~CVoicePlaying();

	/*
	* Check if windows audio playing objet is open.
	*/
	BOOL IsOpen();

	/*
	*  Close windows wave audio plaing object.
	*/
	BOOL Close();

	/*
	* Open windows wave audio plaing object.
	*/
	BOOL Open();

	/*
	* Play audio buffer.
	*/
    BOOL Play();
	
	
	// Handle for output audio wav.
	HWAVEOUT hWaveOut;

	// Event that is signaled when audio is done playing.
	HANDLE mAudioPlayIsDoneEvent;

};

#endif



/************************* End Of File **************************/