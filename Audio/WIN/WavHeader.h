#ifndef __WAV_HEADER_H
#define __WAV_HEADER_H

#include "common.h"

#define SAMPLES_PER_SEC 16000
#define SOUND_CHANNELS 1
#define N_BLOCK_ALLIGN 1
#define BITS_PER_SAMPLE 16

typedef struct WavHeader {
	char RIFF[4];
	DWORD bytes;
	char WAVE[4];
	char fmt[4];
	int siz_wf;
	WORD wFormatTag;
	WORD nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD nBlockAlign;
	WORD wBitsPerSample;
	char data[4];
	DWORD pcmbytes;

} WavHeader;

#endif