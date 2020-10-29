#include "AudioRecorder.h"

AudioRecorder_t AudioRecorder;

#if defined(WIN32)
bool bAudioOpen(AudioRecorder_t* pAudioConfig, RecordCallback cb, void* pvUserData)
{
    if (!pAudioConfig || !cb) return false;
    return true;
}
bool bAudioStart()
{
}
void vAudioClose()
{
}
#elif defined(UNIX)
bool bAudioOpen(AudioRecorder_t* pAudioConfig, RecordCallback cb)
{
    if (!pAudioConfig || !cb) return false;
}
bool bAudioStart()
{
}
void vAudioClose()
{
}
#else
#endif