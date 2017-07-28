//edited by zhangle
#ifndef WEBRTC_MODULES_AVCODING_CODECS_FFMPEG_MAIN_SOURCE_FFMPEG_RESAMPLE_H_
#define WEBRTC_MODULES_AVCODING_CODECS_FFMPEG_MAIN_SOURCE_FFMPEG_RESAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

void *AudioResampleCreate();
void AudioResampleClose(void *handle);
void AudioResampleSrcConfig(void *handle, int format, int channels, int nb_samples, int sample_rate);
void AudioResampleDstConfig(void *handle, int format, int channels, int nb_samples, int sample_rate);
int AudioResampleProcess(void *handle, unsigned char **dstData, unsigned char **srcData);

#ifdef __cplusplus
}
#endif

#endif
