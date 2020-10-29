/**********************************************************
*
* @author: Konstantin Bulgakov
* 
* @brief: This file is example of WavManager module usage. 
*
*         Usage of the following operations is shown:
*         a. Record audio. 
*         b. Play recorded audio.
*         c. Save recorded audio to disk.
*
**********************************************************/

#include <memory>
#include "WavManager.h"
#include "WavRecord.h"
#include "common.h"

// Length of recorded audio (in seconds).
#define AUDIO_REC_DURATION 10

// Name of the created wav file.
#define WAV_FILE_NAME "record.wav"

/**
* Usage example of WavManager class.
*/
int testWindowsAudio()
{
	debug_log("Starting Example Programm");

	auto lManagerPtr = std::make_unique<WavManager>();

	debug_log("Recording audio");
	auto lRecordPtr = lManagerPtr->recordWavBlocking(AUDIO_REC_DURATION);
	if (!lRecordPtr)
	{
		debug_log("recordWavBlocking failed");
		return MY_ERROR;
	}

	debug_log("Playing recorded audio.");
	if (lManagerPtr->playWavBlocking(lRecordPtr.get()) == MY_ERROR)
	{
		debug_log("playWavBlocking failed.");
		return MY_ERROR;
	}

	debug_log("Writing record to disk");
	if (lRecordPtr->writeRecordToWavFile(WAV_FILE_NAME) == MY_ERROR)
	{
		debug_log("writeRecordToWavFile failed.");
		return MY_ERROR;
	}
	return SUCCESS;
}


/****************************** End Of File ********************************/
