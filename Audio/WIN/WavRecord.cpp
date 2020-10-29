#include "WavRecord.h"
#include "WavHeader.h"

WavRecord::WavRecord()
{
	mRecordBuffer = NULL;
	mRecordBufferSize = 0;
	mRecordLengthSecs = 0;
	mWavHeader = NULL;
}

WavRecord::WavRecord(LPSTR aRecordBuf, UINT aRecordBufSize, UINT aRecordLengthSecs)
{
	mRecordBuffer = aRecordBuf;
	mRecordBufferSize = aRecordBufSize;
	mRecordLengthSecs = aRecordLengthSecs;
	fillWavHeader();
}

WavRecord::~WavRecord()
{
	if (mRecordBuffer != NULL)
	{
		delete[] mRecordBuffer;
	}
	if (mWavHeader != NULL)
	{
		delete mWavHeader;
	}
}

WavRecord::WavRecord(const WavRecord& other)
{
	mRecordBufferSize = other.mRecordBufferSize;
	mRecordLengthSecs = other.mRecordLengthSecs;
	mRecordBuffer = new char[mRecordBufferSize];
	memcpy(mRecordBuffer, other.mRecordBuffer, mRecordBufferSize);

	mWavHeader = new WavHeader;
	memcpy(mWavHeader, other.mWavHeader, sizeof(WavHeader));

}

WavRecord::WavRecord(WavRecord&& other)
{
	mRecordBufferSize = other.mRecordBufferSize;
	mRecordBuffer = other.mRecordBuffer;
	mWavHeader = other.mWavHeader;
	mRecordLengthSecs = other.mRecordLengthSecs;

	other.mRecordBufferSize = 0;
	mRecordLengthSecs = 0;
	other.mRecordBuffer = NULL;
	other.mWavHeader = NULL;
}

int WavRecord::fillWavHeader()
{
	mWavHeader = new WavHeader;
	if (mWavHeader == NULL)
	{
		return MY_ERROR;
	}

	UINT blocksize = mRecordBufferSize;
	memcpy(mWavHeader->RIFF, "RIFF", 4);
	mWavHeader->bytes = blocksize + 36;
	memcpy(mWavHeader->WAVE, "WAVE", 4);
	memcpy(mWavHeader->fmt, "fmt ", 4);
	mWavHeader->siz_wf = 16;
	memcpy(mWavHeader->data, "data", 4);
	mWavHeader->pcmbytes = blocksize;
	mWavHeader->wFormatTag = 1; //WAVE_FORMAT_PCM
	mWavHeader->nChannels = SOUND_CHANNELS;
	mWavHeader->nSamplesPerSec = SAMPLES_PER_SEC;
	mWavHeader->nBlockAlign = SOUND_CHANNELS * BITS_PER_SAMPLE / 8;
	mWavHeader->nAvgBytesPerSec = SAMPLES_PER_SEC * mWavHeader->nBlockAlign;
	mWavHeader->wBitsPerSample = BITS_PER_SAMPLE;

	return SUCCESS;
}

UINT WavRecord::getRecordDurationSecs() const
{
	return mRecordLengthSecs;
}


LPSTR WavRecord::getAudioBuffer() const
{
	return mRecordBuffer;
}


UINT WavRecord::getAudioBufferSize() const
{
	return mRecordBufferSize;
}

UINT WavRecord::getAudioLengthSecs() const
{
	return mRecordLengthSecs;
}

int WavRecord::writeRecordToWavFile(const char* aFileName)
{
	DWORD bytesWritten;
	HANDLE hFile = CreateFile(aFileName,              // File to create.
		GENERIC_WRITE,         // Open for writing.
		0,                     // Do not share.
		NULL,                  // Default security.
		CREATE_ALWAYS,         // Overwrite existing.
		FILE_ATTRIBUTE_NORMAL, // Normal file.
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return MY_ERROR;
	}

	WriteFile(hFile, (char*)mWavHeader, sizeof(WavHeader), &bytesWritten, NULL);
	WriteFile(hFile, mRecordBuffer, mRecordBufferSize, &bytesWritten, NULL);
	CloseHandle(hFile);
	return SUCCESS;
}
