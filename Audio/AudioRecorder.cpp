#include "AudioRecorder.h"
#include "EILog.h"

#ifdef __cplusplus
extern "C" {
#endif

bool bAudioOpen(AudioConfig_t *pAudioConfig);
bool bAudioStart(void);
void vAudioStop(void);
void vAudioClose(void);

AudioRecorder_t Recorder = {
    .open   = bAudioOpen,
    .start  = bAudioStart,
    .stop   = vAudioStop,
    .close  = vAudioClose,
    .pvAudioHandle = NULL,
    .stAudioConfig = {16000, 16, 1, NULL, NULL},
    .bRecording = false,
};

#if defined(WIN32)
#define RECORD_BUFFER_SIZE 3200
static WAVEHDR stWaveHdr[2];
static char sRecordBuffer[2][RECORD_BUFFER_SIZE];
static HANDLE pvEventHandle = NULL;
static void WaveInitFormat(LPWAVEFORMATEX m_WaveFormat, WORD nCh, DWORD nSampleRate, WORD BitsPerSample)
{
    m_WaveFormat->wFormatTag = WAVE_FORMAT_PCM;
    m_WaveFormat->nChannels = nCh;
    m_WaveFormat->nSamplesPerSec = nSampleRate;
    m_WaveFormat->nAvgBytesPerSec = nSampleRate * nCh * BitsPerSample / 8;
    m_WaveFormat->nBlockAlign = m_WaveFormat->nChannels * BitsPerSample / 8;
    m_WaveFormat->wBitsPerSample = BitsPerSample;
    m_WaveFormat->cbSize = 0;
}

static DWORD CALLBACK RecordCallbackProc(HWAVEIN hwavein, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
#if defined(TEST_RECORD_TO_FILE)
    DWORD bytesWritten = 0;
    static HANDLE hFile = NULL;
    static DWORD totalWriten = 0;
    if (!hFile)
    {
        hFile = CreateFile("test.pcm",            // File to create.
                           GENERIC_WRITE,         // Open for writing.
                           0,                     // Do not share.
                           NULL,                  // Default security.
                           CREATE_ALWAYS,         // Overwrite existing.
                           FILE_ATTRIBUTE_NORMAL, // Normal file.
                           NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            LOG(EDEBUG, "创建设备失败");
            return -1;
        }
    }
#endif
    switch (uMsg)
    {
    case WIM_OPEN:
        LOG(EDEBUG, "\n设备已经打开...\n");
        if (Recorder.stAudioConfig.pfCallback)
        {
            Recorder.stAudioConfig.pfCallback(Recorder.pvAudioHandle, AUDIO_OPEN, Recorder.stAudioConfig.pvUserData, NULL, 0);
        }
        break;

    case WIM_DATA:
        LOG(EVERBOSE, "\n缓冲区%d存满...\n", (int)((LPWAVEHDR)dwParam1)->dwUser);
#if defined(TEST_RECORD_TO_FILE)
        if (hFile)
        {
            // size_t size = fwrite(((LPWAVEHDR)dwParam1)->lpData, 1, ((LPWAVEHDR)dwParam1)->dwBytesRecorded, file);
            // printf("size:%ld\n", ((LPWAVEHDR)dwParam1)->dwBytesRecorded);
            WriteFile(hFile, ((LPWAVEHDR)dwParam1)->lpData, ((LPWAVEHDR)dwParam1)->dwBytesRecorded, &bytesWritten, NULL);
            totalWriten += bytesWritten;
        }
        if (totalWriten > 350 * 1024)
        {
            CloseHandle(hFile);
            waveInClose(hwavein);
            LOG(EDEBUG, "\n设备已经关闭...\n");
            while (1)
                ;
        }
#endif
        if (Recorder.stAudioConfig.pfCallback)
        {
            Recorder.stAudioConfig.pfCallback(Recorder.pvAudioHandle, AUDIO_DATA, Recorder.stAudioConfig.pvUserData,
                                              ((LPWAVEHDR)dwParam1)->lpData, ((LPWAVEHDR)dwParam1)->dwBytesRecorded);
        }
        waveInAddBuffer(hwavein, (LPWAVEHDR)dwParam1, sizeof(WAVEHDR));
        break;

    case WIM_CLOSE:
        LOG(EDEBUG, "\n设备已经关闭...\n");
        if (Recorder.stAudioConfig.pfCallback)
        {
            Recorder.stAudioConfig.pfCallback(Recorder.pvAudioHandle, AUDIO_CLOSE, Recorder.stAudioConfig.pvUserData, NULL, 0);
        }
        break;
    default:
        break;
    }
    return 0;
}

bool bAudioOpen(AudioConfig_t *pAudioConfig)
{
    if (!pAudioConfig)
        return false;
    if (Recorder.pvAudioHandle)
        return true;

    pvEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
    memcpy(&Recorder.stAudioConfig, pAudioConfig, sizeof(AudioConfig_t));
    do
    {
        // 获取输入音频个数
        if (!waveInGetNumDevs())
        {
            LOG(EDEBUG, "can't found record devices");
            break;
        }

        // 获取音频输入设备的名称
        WAVEINCAPS waveIncaps;
        MMRESULT mmResult = waveInGetDevCaps(0, &waveIncaps, sizeof(WAVEINCAPS));
        if (MMSYSERR_NOERROR != mmResult)
        {
            LOG(EERROR, "get record device error");
            break;
        }

        // 打开设备
        WAVEFORMATEX pwfx;
        WaveInitFormat(&pwfx, pAudioConfig->byChannels,
                       pAudioConfig->iSampleRate, pAudioConfig->byBitsPerSample);
        mmResult = waveInOpen((LPHWAVEIN)(&Recorder.pvAudioHandle), WAVE_MAPPER, &pwfx, (DWORD)RecordCallbackProc, NULL, CALLBACK_FUNCTION);
        if (mmResult != MMSYSERR_NOERROR)
        {
            LOG(EERROR, "open record device error");
            break;
        }

        // 设置接收缓冲区
        stWaveHdr[0].lpData = sRecordBuffer[0];
        stWaveHdr[0].dwBufferLength = RECORD_BUFFER_SIZE;
        stWaveHdr[0].dwUser = 1;
        stWaveHdr[0].dwFlags = 0;
        mmResult = waveInPrepareHeader((HWAVEIN)Recorder.pvAudioHandle, &stWaveHdr[0], sizeof(WAVEHDR));
        if (MMSYSERR_NOERROR != mmResult)
            break;

        stWaveHdr[1].lpData = sRecordBuffer[1];
        stWaveHdr[1].dwBufferLength = RECORD_BUFFER_SIZE;
        stWaveHdr[1].dwUser = 2;
        stWaveHdr[1].dwFlags = 0;
        mmResult = waveInPrepareHeader((HWAVEIN)Recorder.pvAudioHandle, &stWaveHdr[1], sizeof(WAVEHDR));
        if (MMSYSERR_NOERROR != mmResult)
            break;

        mmResult = waveInAddBuffer((HWAVEIN)Recorder.pvAudioHandle, &stWaveHdr[0], sizeof(WAVEHDR));
        if (MMSYSERR_NOERROR != mmResult)
            break;
        mmResult = waveInAddBuffer((HWAVEIN)Recorder.pvAudioHandle, &stWaveHdr[1], sizeof(WAVEHDR));
        if (MMSYSERR_NOERROR != mmResult)
            break;

        return true;
    } while (0);

    return false;
}

bool bAudioStart()
{
    if (Recorder.pvAudioHandle)
    {
        MMRESULT mmResult = waveInStart((HWAVEIN)Recorder.pvAudioHandle);
        WaitForSingleObject(pvEventHandle, INFINITE);
        Recorder.bRecording = true;
        return (MMSYSERR_NOERROR == mmResult);
    }
    return false;
}

void vAudioStop()
{
    Recorder.bRecording = false;
    if (pvEventHandle) {
        SetEvent(pvEventHandle);
    }
}

void vAudioClose()
{
    if (Recorder.pvAudioHandle)
    {
        CloseHandle(pvEventHandle);
        pvEventHandle = NULL;
        waveInClose((HWAVEIN)Recorder.pvAudioHandle);
        Recorder.pvAudioHandle = NULL;
    }
}

#elif defined(UNIX) || defined(__linux__)
#include <tinyalsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
bool bAudioOpen(AudioConfig_t *pAudioConfig)
{
    if (!pAudioConfig)
        return false;

    memcpy(&Recorder.stAudioConfig, pAudioConfig, sizeof(AudioConfig_t));

    int card = 0;
    int device = 0;
    int format = 0;
    int rate = pAudioConfig->iSampleRate;
    int channels = pAudioConfig->byChannels;
    switch(pAudioConfig->byBitsPerSample) {
        case 8:
        format = PCM_FORMAT_S8;
        break;
        case 16:
        format = PCM_FORMAT_S16_LE;
        break;
        case 24:
        format = PCM_FORMAT_S24_LE;
        break;
        default:
        break;
    }

    struct pcm *pcm;
    struct pcm_config config;
    memset(&config, 0, sizeof(config));
    config.channels = channels;
    config.rate = rate;
    config.period_size = 1024;
    config.period_count = 4;
    config.format = format;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    // open record device
    pcm = pcm_open(card, device, PCM_IN, &config);
    if (!pcm || !pcm_is_ready(pcm)) {
        LOG(EERROR, "Unable to open PCM device (%s)",
                pcm_get_error(pcm));
        return false;
    }
    Recorder.pvAudioHandle = (AudioHandle)pcm;
    LOG(EDEBUG, "Capturing sample: %u ch, %u hz, %u bit\n", channels, rate,
        pcm_format_to_bits(format));
    
    return true;
}

bool bAudioStart()
{
    unsigned int size;
    unsigned int frames_read;
    unsigned int total_frames_read;
    unsigned int bytes_per_frame;
    char *buffer;
    struct pcm *pcm;

    pcm = (struct pcm *)Recorder.pvAudioHandle;
    if (!pcm) {
        LOG(EDEBUG, "record device not yet initilize.");
        return false;
    }
    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = malloc(size);
    if (!buffer) {
        LOG(EERROR, "Unable to allocate %u bytes\n", size);
        return false;
    }

    bytes_per_frame = pcm_frames_to_bytes(pcm, 1);
    total_frames_read = 0;
    frames_read = 0;
    Recorder.bRecording = true;
    
    while(Recorder.bRecording) {
        frames_read = pcm_readi(pcm, buffer, pcm_get_buffer_size(pcm));
        total_frames_read += frames_read;
        if (Recorder.stAudioConfig.pfCallback) {
            Recorder.stAudioConfig.pfCallback(Recorder.pvAudioHandle, AUDIO_DATA, 
                Recorder.stAudioConfig.pvUserData, buffer, bytes_per_frame*frames_read);
        }
    }
    free(buffer);
}
void vAudioStop()
{
    Recorder.bRecording = false;
}
void vAudioClose()
{
    if (Recorder.pvAudioHandle) {
        pcm_close(Recorder.pvAudioHandle);
    }
}

#else

/* for another platform */

#endif

/* for test */
static void RecordCB(void* pvHandle, int32_t iType, void* pvUserData, void* pvData, int32_t iLen)
{
    uint32_t bytesWritten = 0;
    static uint32_t totalWriten = 0;
    const char *pathname = (const char *)pvUserData;
    static FILE* fp = NULL;
    if (!fp) {
        fp = fopen(pathname, "wb+");
    }
    if (AUDIO_DATA == iType) {
        if (fp) {
            bytesWritten = fwrite(pvData, iLen, 1, fp);
            totalWriten += bytesWritten*iLen;
            LOG(EDEBUG, "totalWriten:%d, iLen:%d", totalWriten, iLen);
            if (totalWriten > 360*1024) {
                Recorder.stop();
            }
        }
    }
    else if (AUDIO_CLOSE == iType) {
        if (fp) fclose(fp);
        fp = NULL;
        Recorder.stop();
    }
}
void vAudioRecordTest()
{
    AudioConfig_t stAudioConfig = {16000, 16, 1, RecordCB, (void*)"test.pcm"};
    Recorder.open(&stAudioConfig);
    Recorder.start();
    Recorder.close();
}

#ifdef __cplusplus
}
#endif