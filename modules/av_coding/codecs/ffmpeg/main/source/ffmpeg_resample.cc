// edit by zhangle
#include "webrtc/modules/av_coding/codecs/ffmpeg/main/source/ffmpeg_resample.h"

#ifdef  __cplusplus    
extern "C" {
#endif 
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
#ifdef  __cplusplus    
}
#endif 

struct ffmpegAudioResampler {
  struct SwrContext *swr_ctx;
  AVFrame srcFrame;
  AVFrame dstFrame;
};

void *AudioResampleCreate() {
  ffmpegAudioResampler *h = (ffmpegAudioResampler *)malloc(sizeof(ffmpegAudioResampler));
  h->swr_ctx = NULL;
  return h;
}

void AudioResampleClose(void *handle) {
  if (handle) {
    ffmpegAudioResampler *h = (ffmpegAudioResampler *)handle;
    if (h->swr_ctx) {
      swr_free(&h->swr_ctx);
      h->swr_ctx = NULL;
    }
    free(h);
  }
}

void AudioResampleSrcConfig(void *handle, int format, int channels, int nb_samples, int sample_rate) {
  ffmpegAudioResampler *resample = (ffmpegAudioResampler *)handle;
  if (resample) {
    resample->srcFrame.channels = channels;
    resample->srcFrame.nb_samples = nb_samples;
    resample->srcFrame.sample_rate = sample_rate;
    resample->srcFrame.format = format;
  }
}

void AudioResampleDstConfig(void *handle, int format, int channels, int nb_samples, int sample_rate) {
  ffmpegAudioResampler *resample = (ffmpegAudioResampler *)handle;
  if (resample) {
    resample->dstFrame.channels = channels;
    resample->dstFrame.nb_samples = nb_samples;
    resample->dstFrame.sample_rate = sample_rate;
    resample->dstFrame.format = format;
  }
}

int AudioResampleProcess(void *handle, unsigned char **dstData, unsigned char **srcData) {
  ffmpegAudioResampler *resample = (ffmpegAudioResampler *)handle;
  if (resample->swr_ctx == NULL) {
    resample->swr_ctx = swr_alloc();
    if (resample->swr_ctx == NULL) {
      return -1;
    }
    av_opt_set_int(resample->swr_ctx, "in_channel_count", resample->srcFrame.channels, 0);
    av_opt_set_int(resample->swr_ctx, "in_sample_rate", resample->srcFrame.sample_rate, 0);
    av_opt_set_sample_fmt(resample->swr_ctx, "in_sample_fmt", (enum AVSampleFormat)resample->srcFrame.format, 0);
    av_opt_set_int(resample->swr_ctx, "out_channel_count", resample->dstFrame.channels, 0);
    av_opt_set_int(resample->swr_ctx, "out_sample_rate", resample->dstFrame.sample_rate, 0);
    av_opt_set_sample_fmt(resample->swr_ctx, "out_sample_fmt", (enum AVSampleFormat)resample->dstFrame.format, 0);
    if (swr_init(resample->swr_ctx) < 0) {
      return -1;
    }
  }

  const uint8_t *src[AV_NUM_DATA_POINTERS];
  for (int i = 0; i < AV_NUM_DATA_POINTERS; i++) {
    src[i] = srcData[i];
  }
  return swr_convert(resample->swr_ctx,
    dstData, resample->dstFrame.nb_samples,
    src, resample->srcFrame.nb_samples);
}
