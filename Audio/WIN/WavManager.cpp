
#include "WavManager.h"
#include "VoiceRecording.h"
#include "VoicePlaying.h"
#include "WavRecord.h"

WavManager::WavManager()
{
	mWaveRecorder = new CVoiceRecording();
	mWavePlayer = new CVoicePlaying();
}

WavManager::~WavManager()
{
	if (mWaveRecorder != NULL)
	{
		delete mWaveRecorder;
	}
		
	if (mWavePlayer != NULL)
	{
		delete mWavePlayer;
	}
}


std::unique_ptr<WavRecord> WavManager::recordWavBlocking(UINT aDurationSecs)
{
	mWaveRecorder->PrepareBuffer(aDurationSecs);
	mWaveRecorder->Open();
	if (mWaveRecorder->IsOpen())
	{
		//Record audio sample for soundDuration seconds.
		mWaveRecorder->Record();
	}
	else
	{
		std::unique_ptr<WavRecord> lRetPtr(nullptr);
		return lRetPtr;
	}

	//Save the sampled audio to sound buffer object.
	UINT bytesInBuffer = aDurationSecs * mWaveRecorder->getBytesPerSec();

	std::unique_ptr<WavRecord> lRetPtr(new WavRecord(mWaveRecorder->buffer, bytesInBuffer, aDurationSecs));

	mWaveRecorder->resetBuffer();
	
	return lRetPtr;
}

int WavManager::playWavBlocking(const WavRecord*  aWav)
{
	mWavePlayer->PrepareBuffer(aWav->getRecordDurationSecs());
	mWavePlayer->Open();

	memcpy(mWavePlayer->buffer, aWav->getAudioBuffer(), aWav->getAudioBufferSize());
	if (mWavePlayer->IsOpen())
	{
		mWavePlayer->Play();
		return SUCCESS;
	}
	else
	{
		return MY_ERROR;
	}
}

