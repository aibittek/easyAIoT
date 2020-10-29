#ifndef __WAV_MANAGER_H
#define __WAV_MANAGER_H

#include <memory>
#include "common.h"

class WavRecord;
class CVoiceRecording;
class CVoicePlaying;


/**
* Class that implements wav utilities.
*/
class WavManager
{
public:

	/**
	* Constructor.
	*/
	WavManager();

	/**
	* Destructor.
	*/
	~WavManager();

	/**
	* @brief: Record audio.
	* @param: aDurationSecs - Audio record duration.
	* @return: unique_ptr to recorded wav object.
	*
	*/
	std::unique_ptr<WavRecord> recordWavBlocking(UINT aDurationSecs);
	
	/**
	* @brief: Play recorded wav.
	* @param: aWav - Wab object that is containing audio.
	* @return: On error - MY_ERROR, otherwise SUCCESS.
	*/
	int playWavBlocking(const WavRecord*  aWav);


private:
	/*
	* Object that used to record wav.
	*/
	CVoiceRecording* mWaveRecorder;

	/*
	* Object that used to play wav.
	*/
	CVoicePlaying* mWavePlayer;
};


#endif


/********************** End Of File ************************/