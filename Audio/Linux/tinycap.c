#if defined(__unix__)
#include <tinyalsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <limits.h>

static int capturing = 1;
static int prinfo = 1;
typedef int (*capture_callback)(void *pvData, int iLen);
void vStartCapture(capture_callback cb)
{
    // card:0,device:0,channels:1,sample_rate:16000, format:0,period_size:1024,period_count:4
    int card = 0;
    int device = 0;
    int format = 0;
    int rate = 16000;
    int channels = 1;
    unsigned int frames_read;
    unsigned int total_frames_read;
    unsigned int bytes_per_frame;
    unsigned int size;
    char *buffer;
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

    capturing = 1;
    // FILE *file = fopen("./test.pcm", "wb");

    pcm = pcm_open(card, device, PCM_IN, &config);
    if (!pcm || !pcm_is_ready(pcm)) {
        fprintf(stderr, "Unable to open PCM device (%s)\n",
                pcm_get_error(pcm));
        return ;
    }

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Unable to allocate %u bytes\n", size);
        pcm_close(pcm);
        return ;
    }

    if (prinfo) {
        // printf("Capturing sample: %u ch, %u hz, %u bit\n", channels, rate,
        //    pcm_format_to_bits(format));
    }
    bytes_per_frame = pcm_frames_to_bytes(pcm, 1);
    total_frames_read = 0;
    frames_read = 0;
    while (capturing) {
        frames_read = pcm_readi(pcm, buffer, pcm_get_buffer_size(pcm));
        total_frames_read += frames_read;
        if (cb(buffer, bytes_per_frame*frames_read)) capturing = 0;
        // if ((total_frames_read / rate) >= 5000) {
        //     capturing = 0;
        // }
        // printf("bytes_per_frame:%d,frames_read:%d\n", bytes_per_frame, frames_read);
        // if (fwrite(buffer, bytes_per_frame, frames_read, file) != frames_read) {
        //     fprintf(stderr,"Error capturing sample\n");
        //     break;
        // }
    }
    free(buffer);
    pcm_close(pcm);
}

void vStopCapture()
{
    capturing = 0;
}
#endif
