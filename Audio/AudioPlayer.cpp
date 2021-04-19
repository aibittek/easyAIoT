#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <file.h>
#include <cstring.h>
#include <Windows.h>
#include <stdio.h>
#pragma comment(lib, "winmm.lib")

void pcmPlay(const char *pathname)
{
    const int buf_size = 1024 * 1024 * 30;

    cstring_t *pFile = readFile(pathname);

    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag = WAVE_FORMAT_PCM;   //设置波形声音的格式
    wfx.nChannels = 1;                  //设置音频文件的通道数量
    wfx.nSamplesPerSec = 16000;         //设置每个声道播放和记录时的样本频率
    wfx.wBitsPerSample = 16;            //每隔采样点所占的大小
 
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
  
    HANDLE wait = CreateEvent(NULL, 0, 0, NULL);
    HWAVEOUT hwo;
    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, (DWORD_PTR)wait, 0L, CALLBACK_EVENT); //打开一个给定的波形音频输出装置来进行回放
 
    int data_size = 20480;
    char* data_ptr = pFile->str;
    WAVEHDR wh;
 
    while (data_ptr - pFile->str < pFile->len)
    {
        //这一部分需要特别注意的是在循环回来之后不能花太长的时间去做读取数据之类的工作，不然在每个循环的间隙会有“哒哒”的噪音
        wh.lpData = data_ptr;
        wh.dwBufferLength = data_size;
        wh.dwFlags = 0L;
        wh.dwLoops = 1L;
 
        data_ptr += data_size;
 
        waveOutPrepareHeader(hwo, &wh, sizeof(WAVEHDR)); //准备一个波形数据块用于播放
        waveOutWrite(hwo, &wh, sizeof(WAVEHDR)); //在音频媒体中播放第二个函数wh指定的数据
 
        WaitForSingleObject(wait, INFINITE); //等待
    }
    waveOutClose(hwo);
    CloseHandle(wait);
    cstring_del(pFile);
}
#elif defined(LINUX)
void pcmPlay(const char *pathname)
{
    const char *format = "aplay -r16000 -c1 -f S16_LE %s";
    int len = strlen(format) + strlen(pathname);
    char *cmd = (char *)malloc(len);
    if (!cmd) return;
    sprintf(cmd, format, pathname);
    system(cmd);
    if (cmd) free(cmd);
}
#else
#error "pcm play not support on this platform."
#endif

#ifdef __cplusplus
}
#endif