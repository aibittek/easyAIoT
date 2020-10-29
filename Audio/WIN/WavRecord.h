#ifndef __WAV_RECORD_H
#define __WAV_RECORD_H

#include "common.h"

struct WavHeader;
struct WavAudioParams;


/**
* Class that represents wav record.
*/
class WavRecord
{
public:
	/**
	* Default Constructor.
	*/
	WavRecord();

	/**
	* Constructor.
	*/
	WavRecord(LPSTR aRecordBuf, UINT aRecordBufSize, UINT aRecordLengthSecs);

	/**
	* Destructor.
	*/
	~WavRecord();
	
	/**
	* Copy constructor.
	*/
	WavRecord(const WavRecord& other);
	
	/**
	* Move constructor.
	*/
	WavRecord(WavRecord&& other);

	/**
	*
	*/
	int writeRecordToWavFile(const char* aFileName);

	/**
	*
	*/
	UINT getRecordDurationSecs() const;

	/**
	*
	*/
	LPSTR getAudioBuffer() const;

	/**
	*
	*/
	UINT getAudioBufferSize() const;

	/**
	*
	*/
	UINT getAudioLengthSecs() const;

private:

	int fillWavHeader();

	LPSTR mRecordBuffer;
	UINT mRecordBufferSize;
	UINT mRecordLengthSecs;
	WavHeader* mWavHeader;
};


#endif


/******************************* End Of File ********************************/