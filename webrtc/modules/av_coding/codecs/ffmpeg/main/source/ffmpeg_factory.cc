//edit by zhangle
#include "webrtc/modules/av_coding/codecs/ffmpeg/main/source/ffmpeg_factory.h"

#include "third_party/jsoncpp/source/include/json/json.h"
#include "webrtc/avengine/source/avengine_util.h"
#include "webrtc/modules/av_coding/codecs/ffmpeg/main/source/ffmpeg_resample.h"
#include <assert.h>

#if defined(WEBRTC_IOS) || defined(WEBRTC_MAC) || defined(IOS)
#include "../interface/file_ios.h"
#endif

#ifdef  __cplusplus    
extern "C" {
#endif
#include "libavformat/avformat.h"
#include "libavutil/opt.h"
#ifdef  __cplusplus
}
#endif

#ifndef WIN32
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#endif

#if defined(WIN32)
#define snprintf _snprintf
#endif

#define MAX_AUDIO_SIZE		(8192 * 3)

#define FIXED_AUDIO_SAMPLE_FMT  AV_SAMPLE_FMT_S16
#define FIXED_VIDEO_PIX_FMT     AV_PIX_FMT_YUV420P

namespace {

typedef struct {
  //video
  int in_data_format;		//raw(yuv) or stream
  int width;
  int height;
  int gop_size;
  int mtu_size;
  int frame_rate;
  int video_bits_rate;
  int video_codec_id;
  char preset[32];
  //audio
  int audio_bits_rate;
  int audio_codec_id;
  int frame_size;
  int in_channel_count;
  int in_sample_rate;
  int in_sample_fmt;
  //common
  unsigned int debugInfo;
}CodecParams;

typedef struct {
  CodecParams params;
  char audio_buf[MAX_AUDIO_SIZE];
  AVCodecContext *codecCtx;
  AVCodec *codec;
  AVFrame *frame;
  AVBitStreamFilterContext* bsfc;
  void *audio_resample;
  int sdp_flag;
  FILE *ofp;
}MediaStream;

static int ffmpeg_decoder_error = 0;
void ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vl) {
  if (level > av_log_get_level()) {
    return;
  }
	char msg[1024];
	vsprintf(msg, fmt, vl);
	size_t len = strlen(msg);
	if (len > 0 && len < 1024 && msg[len - 1] == '\n') {
		msg[len - 1] = '\0';
	}

  if (ptr) {
    AVClass* avc = *(AVClass **)ptr;
    if (avc) {
      const char *module = avc->item_name(ptr);
      if (strstr(module, "264") && strstr(msg, "error")) {
        AVENGINE_ERR("video decode error %s", msg);
        ffmpeg_decoder_error++;
      }
    }
  }
}

void initFFmpeg() {
  static bool s_inited = false;
  if (!s_inited) {
    s_inited = true;
    av_log_set_level(AV_LOG_INFO);
    av_log_set_callback(ffmpeg_log_callback);
    av_register_all();
    avformat_network_init();
  }
}

// 输入格式写死为了AV_SAMPLE_FMT_S16
int OpenAudioEncoder(MediaStream *mst, const lfrtcAudioEncodeParams *params, int *nb_sample) {
  *nb_sample = 0;
  if (!mst || !params) {
    return -1;
  }

  AVCodecID codec_id = AVCodecID::AV_CODEC_ID_AAC;
  if (params->codec_id) {
    codec_id = (AVCodecID)(params->codec_id);
  }
  mst->codec = avcodec_find_encoder(codec_id);
  if (!mst->codec) {
    return -2;
  }
  AVCodecContext *c = avcodec_alloc_context3(mst->codec);
  if (!c) {
    return -3;
  }
  mst->codecCtx = c;

  c->sample_fmt = FIXED_AUDIO_SAMPLE_FMT;
  if (mst->codec->sample_fmts) {
    bool sample_fmt_supported = false;
    for (int i = 0; mst->codec->sample_fmts[i]; i++) {
      if (mst->codec->sample_fmts[i] == c->sample_fmt) {
        sample_fmt_supported = true;
        break;
      }
    }
    if (!sample_fmt_supported) {
      return -4;
    }
  }

  c->sample_rate = params->sample_rate;
  if (mst->codec->supported_samplerates) {
    bool sample_rate_supported = false;
    for (int i = 0; mst->codec->supported_samplerates[i]; i++) {
      if (mst->codec->supported_samplerates[i] == c->sample_rate) {
        sample_rate_supported = true;
        break;
      }
    }
    if (!sample_rate_supported) {
      return -5;
    }
  }

  uint64_t channel_layout = 0;
  if (params->channel_count == 1) {
    channel_layout = AV_CH_LAYOUT_MONO;
  }
  else if (params->channel_count == 2) {
    channel_layout = AV_CH_LAYOUT_STEREO;
  }
  else {
    return -6;
  }
  c->channel_layout = channel_layout;
  if (mst->codec->channel_layouts) {
    bool channel_layout_supported = false;
    for (int i = 0; mst->codec->channel_layouts[i]; i++) {
      if (mst->codec->channel_layouts[i] == c->channel_layout) {
        channel_layout_supported = true;
        break;
      }
    }
    if (!channel_layout_supported) {
      return -7;
    }
  }
  c->channels = av_get_channel_layout_nb_channels(c->channel_layout);

  c->bit_rate = params->bitrate;
  c->frame_size = 1024;
  c->time_base.den = c->sample_rate;
  c->time_base.num = 1;
  c->flags |= CODEC_FLAG_GLOBAL_HEADER;
  if (avcodec_open2(c, mst->codec, NULL) < 0) {
    return -8;
  }
  if (c->sample_fmt != FIXED_AUDIO_SAMPLE_FMT
    || c->sample_rate != params->sample_rate
    || c->channel_layout != channel_layout) {
    return -9;
  }

  mst->frame = av_frame_alloc();
  mst->frame->channels = c->channels;
  mst->frame->sample_rate = c->sample_rate;
  mst->frame->nb_samples = c->frame_size;
  if (mst->frame->nb_samples <= 0) {
    mst->frame->nb_samples = 1024;
  }
  mst->frame->format = c->sample_fmt;
  mst->frame->channel_layout = c->channel_layout;
  *nb_sample = mst->frame->nb_samples;

  mst->sdp_flag = 1;
  mst->bsfc = av_bitstream_filter_init("aac_adtstoasc");
  return 0;
}

int OpenVideoEncoder(MediaStream *mst, const lfrtcVideoEncodeParams *params) {
  if (!mst) {
    return -1;
  }

  AVCodecID codec_id = AVCodecID::AV_CODEC_ID_H264;
  if (params->codec_id) {
    codec_id = (AVCodecID)(params->codec_id);
  }
  mst->codec = avcodec_find_encoder(codec_id);
  if (!mst->codec) {
    return -2;
  }
  AVCodecContext *c = avcodec_alloc_context3(mst->codec);
  if (!c) {
    return -3;
  }
  mst->codecCtx = c;

  int bitsrate = params->bitrate;
  c->width = params->width;
  c->height = params->height;
  // TODO: zhangle, time_base待研究
  c->time_base.den = params->fps;
  c->time_base.num = 1;
  c->pix_fmt = FIXED_VIDEO_PIX_FMT;
  c->qmax = 40;
  c->qmin = 20;// 26;
  //c->qmax = c->qmin = 30;// 26;//const qp//test
  //
  //c->refs = 2;
  c->b_frame_strategy = false;
  c->max_b_frames = 0;// 3;// 2;// 1;// 0;// 1;// 4;// 1;// 0;
  c->coder_type = c->max_b_frames ? FF_CODER_TYPE_AC : FF_CODER_TYPE_VLC;
  c->coder_type = FF_CODER_TYPE_AC;//test
  //
  //c->cqp = 24;
  c->bit_rate = bitsrate;//params->video_bits_rate;//768000;
  //c->rc_max_rate = bitsrate;
  c->gop_size = params->gop_size;
  //
  c->thread_type = FF_THREAD_SLICE; //FF_THREAD_FRAME;
  //c->thread_count = 1;
  //
  if (!strcmp(params->preset, "")) {
    av_opt_set(c->priv_data, "preset", "superfast", 0);
    av_opt_set(c->priv_data, "tune", "zerolatency", 0);
    // slice-max-size基本上规定了max_nalu，为方便udp传输，将最大值设为1300
    av_opt_set(c->priv_data, "x264opts", "slice-max-size=1300", 0);
  }
  else {
    av_opt_set(c->priv_data, "preset", params->preset, 0);
  }
  c->rc_buffer_size = bitsrate;
  //av_opt_set(c->priv_data, "x264opts","slice-max-size=1400:keyint=50",0);
  //av_opt_set(c->priv_data, "preset", "ultrafast", 0);
  //av_opt_set(c->priv_data, "tune","stillimage,fastdecode,zerolatency",0);
  //av_opt_set(c->priv_data, "x264opts","crf=26:vbv-maxrate=728:vbv-bufsize=364:keyint=50:slice-max-size=1400",0);//:mtu=1400:sliced-threads=1

  c->flags |= CODEC_FLAG_GLOBAL_HEADER;
  if (avcodec_open2(c, mst->codec, NULL) < 0) {
    return -4;
  }

  mst->frame = av_frame_alloc();
  mst->sdp_flag = 1;
  mst->bsfc = av_bitstream_filter_init("h264_mp4toannexb");
  return 0;
}

int OpenAudioDecoder(MediaStream *mst, const lfrtcAudioDecodeParams *params) {
  if (!mst) {
    return -1;
  }

  AVCodecID codec_id = AVCodecID::AV_CODEC_ID_AAC;
  if (params->codec_id) {
    codec_id = (AVCodecID)(params->codec_id);
  }
  mst->codec = avcodec_find_decoder(codec_id);
  if (!mst->codec) {
    return -2;
  }
  AVCodecContext *c = avcodec_alloc_context3(mst->codec);
  if (!c) {
    return -3;
  }
  mst->codecCtx = c;

  if (avcodec_open2(c, mst->codec, NULL) < 0) {
    return -4;
  }

  mst->frame = av_frame_alloc();
  mst->sdp_flag = 1;
  mst->bsfc = av_bitstream_filter_init("aac_adtstoasc");
  return 0;
}

int OpenVideoDecoder(MediaStream *mst, const lfrtcVideoDecodeParams *params) {
  if (!mst) {
    return -1;
  }

  AVCodecID codec_id = AVCodecID::AV_CODEC_ID_H264;
  if (params->codec_id) {
    codec_id = (AVCodecID)(params->codec_id);
  }
  mst->codec = avcodec_find_decoder(codec_id);
  if (!mst->codec) {
    return -2;
  }
  AVCodecContext *c = avcodec_alloc_context3(mst->codec);
  if (!c) {
    return -3;
  }
  mst->codecCtx = c;

  c->thread_type = FF_THREAD_SLICE;

  // 如果每次送给解码器的数据不是一个完整的帧，那么应开启下面的三行代码，
  // 此时ffmpeg会等到检测到下一帧图像的头才去解上一帧
  //if (mst->codec->capabilities & CODEC_CAP_TRUNCATED) {
  //  c->flags |= CODEC_FLAG_TRUNCATED;
  //}
  if (avcodec_open2(c, mst->codec, NULL) < 0) {
    return -4;
  }

  mst->frame = av_frame_alloc();
  mst->sdp_flag = 1;
  mst->bsfc = av_bitstream_filter_init("h264_mp4toannexb");
  return 0;
}

int OpenCodec(MediaStream *mst, bool bEncoder, bool bAudio) {
  if (bEncoder) {
    if (bAudio) {
      int frame_size = 0;
      return OpenAudioEncoder(mst, NULL, &frame_size);
    }
    else {
      return OpenVideoEncoder(mst, NULL);
    }
  }
  else {
    if (bAudio) {
      return OpenAudioDecoder(mst, NULL);
    }
    else {
      return OpenVideoDecoder(mst, NULL);
    }
  }
}

int read_json_int(Json::Value &root, const char *path, int *value) {
  if (root.isNull() || !root.isObject()) {
    return -1;
  }
  Json::Value jvalue = root[path];
  if (jvalue.isIntegral()) {
    *value = jvalue.asInt();
    return 0;
  }
  else if (jvalue.isString()) {
    *value = atoi(jvalue.asCString());
    return 0;
  }
  return -1;
}

int read_json_string(Json::Value &root, const char *path, char *value) {
  if (root.isNull() || !root.isObject()) {
    return -1;
  }
  Json::Value jvalue = root[path];
  if (jvalue.isString()) {
    strcpy(value, jvalue.asCString());
    return 0;
  }
  return -1;
}

}

void lfrtcFFDeleteCodec(void *codecCtx) {
  MediaStream *mst = (MediaStream *)codecCtx;
  if (mst) {
    if (mst->audio_resample) {
      AudioResampleClose((void *)mst->audio_resample);
      mst->audio_resample = NULL;
    }
    if (mst->codecCtx) {
      avcodec_free_context(&mst->codecCtx);
    }
    if (mst->codec) {
      mst->codec = NULL;
    }
    if (mst->frame) {
      av_frame_free(&mst->frame);
    }
    if (mst->bsfc) {
      av_bitstream_filter_close(mst->bsfc);
      mst->bsfc = NULL;
    }
    delete mst;
  }
}

int lfrtcEncodeVideo(void *ctx, char *inBuf[3], int inLinesize[3], char *outBuf[3], int *outLen[3], bool *keyframe) {
  int out_buf_capacity = *(outLen[0]);
  *(outLen[0]) = 0;
  if (outLen[1]) {
    *(outLen[1]) = 0;
  }
  if (outLen[2]) {
    *(outLen[2]) = 0;
  }

  MediaStream *mst = (MediaStream *)ctx;
  AVFrame* frame = mst->frame;
  AVCodecContext *c = mst->codecCtx;

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = NULL;
  pkt.size = 0;
#ifndef FFMSTREAM
  pkt.pts = 0;//c->frame_number * (48000 / 25);//AV_NOPTS_VALUE;
  pkt.dts = 0;//c->frame_number * (48000 / 25);//AV_NOPTS_VALUE;
#endif

  frame->pts = c->frame_number;
  frame->data[0] = (uint8_t *)inBuf[0];
  frame->data[1] = (uint8_t *)inBuf[1];
  frame->data[2] = (uint8_t *)inBuf[2];
  // 不能保证linesize[0] == c->width，因此需要调用者传入
  frame->linesize[0] = inLinesize[0];
  frame->linesize[1] = inLinesize[1];
  frame->linesize[2] = inLinesize[2];
  frame->width = c->width;
  frame->height = c->height;
  frame->format = c->pix_fmt; //AV_PIX_FMT_YUV420P;
  frame->key_frame = (int)(*keyframe);

  *keyframe = false;

  int got_output = 0;
  int ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
  if (ret < 0) {
    av_free_packet(&pkt);
    return -2;
  }
  if (!got_output) {
    // it means the data was buffered.
    av_free_packet(&pkt);
    return 0;
  }

  if (pkt.flags & AV_PKT_FLAG_KEY) {
    *keyframe = true;
  }
  if (mst->sdp_flag && (pkt.flags & AV_PKT_FLAG_KEY)) {
    if (out_buf_capacity < pkt.size + c->extradata_size) {
      av_free_packet(&pkt);
      return -1;
    }
    *(outLen[0]) = pkt.size + c->extradata_size;
    memcpy(outBuf[0], c->extradata, c->extradata_size);
    memcpy(&outBuf[0][c->extradata_size], pkt.data, pkt.size);
    ret = pkt.size + c->extradata_size;
  }
  else {
    if (out_buf_capacity < pkt.size) {
      av_free_packet(&pkt);
      return -1;
    }
    *(outLen[0]) = pkt.size;
    memcpy(outBuf[0], pkt.data, pkt.size);
    ret = pkt.size;
  }

  if (!mst->ofp && mst->params.debugInfo) {
#ifdef _WIN32
    mst->ofp = fopen("e://works//test//video.h264", "wb");
#elif defined(__ANDROID__)
    mst->ofp = fopen("/sdcard/video.h264", "wb");
#elif defined(WEBRTC_IOS)
    char dirName[256] = "video.h264";
    char *pName = MAKE_FILE_NAME(dirName);
    mst->ofp = fopen(pName, "wb");
    free(pName);
#endif
  }
  if (mst->ofp) {
    fwrite(outBuf[0], 1, ret, mst->ofp);
  }

  av_free_packet(&pkt);
  return 0;
}

int lfrtcDecodeVideo(void *ctx, char *inBuf[3], int inLen[3], char *outBuf[3], int outLen[3], int *frameType) {
  MediaStream *mst = (MediaStream *)ctx;
  AVFrame* frame = mst->frame;
  AVCodecContext *c = mst->codecCtx;

  uint8_t *free_buf = NULL;

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.size = inLen[0];
  pkt.data = (uint8_t *)inBuf[0];
  pkt.flags = 0;
  if (mst->sdp_flag) {
    AVPacket tmpPkt = pkt;
    int ret = av_bitstream_filter_filter(mst->bsfc, mst->codecCtx, NULL, &tmpPkt.data, &tmpPkt.size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
    if (ret > 0) {
      pkt = tmpPkt;
      free_buf = tmpPkt.data;
    }
    else if (ret == 0) {
      pkt = tmpPkt;
    }
  }

  frame->key_frame = 0;
  while (pkt.size > 0) {
    // TODO: zhangle，研究ffmpeg_decoder_error
    ffmpeg_decoder_error = 0;
    int got_picture = 0;
    int usedLen = avcodec_decode_video2(c, frame, &got_picture, &pkt);
    if (usedLen < 0) {
      if (free_buf) {
        av_free(free_buf);
      }
      return -2;
    }

    if (got_picture) {
      *frameType = frame->pict_type;

      int y_size = frame->width * frame->height;
      if (outLen[0] < y_size || outLen[1] < y_size / 4 || outLen[2] < y_size / 4) {
        if (free_buf) {
          av_free(free_buf);
        }
        return -1;
      }

      //Y
      for (int i = 0; i < frame->height; i++) {
        memcpy(&outBuf[0][i * frame->width], &frame->data[0][i * frame->linesize[0]], frame->width);
      }
      //U
      for (int i = 0; i < frame->height; i++) {
        if (!(i & 1)) {
          memcpy(&outBuf[1][(i >> 1) * (frame->width >> 1)], &frame->data[1][(i >> 1) * frame->linesize[1]], frame->width >> 1);
        }
      }
      //V
      for (int i = 0; i < frame->height; i++) {
        if (!(i & 1)) {
          memcpy(&outBuf[2][(i >> 1) * (frame->width >> 1)], &frame->data[2][(i >> 1) * frame->linesize[2]], frame->width >> 1);
        }
      }

      if (!mst->ofp && mst->params.debugInfo) {
#ifdef _WIN32
        mst->ofp = fopen("e://works//test//video-d.yuv", "wb");
#elif defined(__ANDROID__)
        mst->ofp = fopen("/sdcard/video-d.yuv", "wb");
#elif defined(WEBRTC_IOS)
        char dirName[256] = "video-d.yuv";
        char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
        mst->ofp = fopen(pName, "wb");
        free(pName);
#endif
      }
      if (mst->ofp) {
        fwrite(outBuf[0], 1, frame->width * frame->height, mst->ofp);
        fwrite(outBuf[1], 1, ((frame->width * frame->height) >> 2), mst->ofp);
        fwrite(outBuf[2], 1, ((frame->width * frame->height) >> 2), mst->ofp);
      }
    }

    if (pkt.data) {
      pkt.size -= usedLen;
      pkt.data += usedLen;
    }
  }

  if (free_buf) {
    av_free(free_buf);
  }
  return 0;
}

int lfrtcEncodeAudio(void *ctx, char *inBuf[3], int inLen[3], char *outBuf[3], int *outLen[3]) {
  int out_buf_capacity = *(outLen[0]);
  *(outLen[0]) = 0;
  if (outLen[1]) {
    *(outLen[1]) = 0;
  }
  if (outLen[2]) {
    *(outLen[2]) = 0;
  }
  if (out_buf_capacity < 7) {
    return -1;
  }

  MediaStream *mst = (MediaStream *)ctx;
  AVFrame* frame = mst->frame;
  AVCodecContext *c = mst->codecCtx;
  assert(c->frame_size == inLen[0] / av_get_bytes_per_sample(FIXED_AUDIO_SAMPLE_FMT) / c->channels);

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = NULL;
  pkt.size = 0;
#ifndef FFMSTREAM
  pkt.pts = 0;//c->frame_number * frame->nb_samples;//AV_NOPTS_VALUE;
  pkt.dts = 0;//c->frame_number * frame->nb_samples;//AV_NOPTS_VALUE;
#endif

  frame->data[0] = (uint8_t *)inBuf[0];
  AVRational q = { 1, c->sample_rate };
  frame->pts = av_rescale_q(c->frame_number * frame->nb_samples, q, c->time_base);

  int got_output = 0;
  int ret = avcodec_encode_audio2(c, &pkt, frame, &got_output);
  if (ret < 0) {
    av_free_packet(&pkt);
    AVENGINE_ERR("avcodec_encode_audio2 failed, ret=%d", ret);
    assert(false);
    return -2;
  }
  if (!got_output) {
    // it means the data was buffered.
    av_free_packet(&pkt);
    return 0;
  }

  char *outData = outBuf[0];

  if (mst->sdp_flag) {
    assert(7 == c->extradata_size);
    char temp = (c->extradata[1] & 0x80);
    int sample_index = (c->extradata[0] & 0x07) << 1;
    switch (c->sample_rate) {
    case 44100:
      sample_index = 0x7;
      break;
    default:
      sample_index = sample_index + (temp >> 7);
      break;
    }
    int channel = ((c->extradata[1] - temp) & 0xff) >> 3;
    char *bits = outData;
    int length = 7 + pkt.size;
    bits[0] = 0xff;
    bits[1] = 0xf1;
    bits[2] = 0x40 | (sample_index << 2) | (channel >> 2);
    bits[3] = ((channel & 0x3) << 6) | (length >> 11);
    bits[4] = (length >> 3) & 0xff;
    bits[5] = ((length << 5) & 0xff) | 0x1f;
    bits[6] = 0xfc;

    *(outLen[0]) = c->extradata_size;
  }

  if (out_buf_capacity < *(outLen[0]) + pkt.size) {
    *(outLen[0]) = 0;
    av_free_packet(&pkt);
    return -1;
  }
  memcpy(outData + (*(outLen[0])), pkt.data, pkt.size);
  (*(outLen[0])) += pkt.size;
  av_free_packet(&pkt);
  return 0;
}

int lfrtcDecodeAudio(void *ctx, char *inBuf[3], int inLen[3], char *outBuf[3], int *outLen[3]) {
  int out_buf_capacity = *(outLen[0]);
  *(outLen[0]) = 0;
  if (outLen[1]) {
    *(outLen[1]) = 0;
  }
  if (outLen[2]) {
    *(outLen[2]) = 0;
  }

  MediaStream *mst = (MediaStream *)ctx;
  AVCodecContext *c = mst->codecCtx;
  AVFrame* frame = mst->frame;

  AVPacket pkt;
  int got_frame = 0;
  int out_size = 0;

  av_init_packet(&pkt);
  pkt.size = inLen[0];
  pkt.data = (uint8_t *)inBuf[0];

  while (pkt.size > 0) {
    int read_size = avcodec_decode_audio4(c, frame, &got_frame, &pkt);
    if (read_size < 0) {
      return -2;
    }

    if (got_frame) {
      if (mst->audio_resample == NULL) {
        mst->audio_resample = AudioResampleCreate();
      }
      AudioResampleSrcConfig(mst->audio_resample, frame->format, frame->channels, frame->nb_samples, frame->sample_rate);
      int dst_nb_samples = (48000 * frame->nb_samples) / frame->sample_rate;
      AudioResampleDstConfig(mst->audio_resample, FIXED_AUDIO_SAMPLE_FMT, frame->channels, dst_nb_samples, 48000);
      uint8_t *resample_out[1];
      resample_out[0] = (uint8_t *)mst->audio_buf;
      int decoded_len = AudioResampleProcess(mst->audio_resample, resample_out, frame->data) * av_get_bytes_per_sample(c->sample_fmt);
      if (out_size + decoded_len > out_buf_capacity) {
        return -1;
      }
      memcpy(outBuf[0] + out_size, mst->audio_buf, decoded_len);
      out_size += decoded_len;
    }

    if (pkt.data) {
      pkt.size -= read_size;
      pkt.data += read_size;
    }
  }

  *(outLen[0]) = out_size;
  return 0;
}

void *lfrtcCreateAudioEncoder(const lfrtcAudioEncodeParams *params, int *nb_sample) {
  initFFmpeg();

  MediaStream *mst = new MediaStream;
  memset(mst, 0, sizeof(MediaStream));
  if (OpenAudioEncoder(mst, params, nb_sample) < 0) {
    lfrtcFFDeleteCodec(mst);
    return NULL;
  }
  return mst;
}

void *lfrtcCreateAudioDecoder(const lfrtcAudioDecodeParams *params) {
  initFFmpeg();

  MediaStream *mst = new MediaStream;
  memset(mst, 0, sizeof(MediaStream));
  if (OpenAudioDecoder(mst, params) < 0) {
    lfrtcFFDeleteCodec(mst);
    return NULL;
  }
  return mst;
}

void *lfrtcCreateVideoEncoder(const lfrtcVideoEncodeParams *params) {
  initFFmpeg();

  MediaStream *mst = new MediaStream;
  memset(mst, 0, sizeof(MediaStream));
  if (OpenVideoEncoder(mst, params) < 0) {
    lfrtcFFDeleteCodec(mst);
    return NULL;
  }
  return mst;
}

void *lfrtcCreateVideoDecoder(const lfrtcVideoDecodeParams *params) {
  initFFmpeg();

  MediaStream *mst = new MediaStream;
  memset(mst, 0, sizeof(MediaStream));
  if (OpenVideoDecoder(mst, params) < 0) {
    lfrtcFFDeleteCodec(mst);
    return NULL;
  }
  return mst;
}

int lfrtcSaveFrameToJPEG(unsigned char** yuvbuf, int width, int height, const char *dumpPath) {
  AVFrame* frame = NULL;

  frame = av_frame_alloc();
  if (!frame)
  {
    AVENGINE_ERR("Could not get memory of frame!");
    return -1;
  }

  frame->data[0] = yuvbuf[0];
  frame->data[1] = yuvbuf[1];
  frame->data[2] = yuvbuf[2];
  frame->linesize[0] = width;
  frame->linesize[1] = width >> 1;
  frame->linesize[2] = width >> 1;
  frame->width = width;
  frame->height = height;
  frame->format = AV_PIX_FMT_YUV420P;

  //char *dumpPath = "/sdcard/lf-dump.jpeg";
  AVFormatContext* pFormatCtx = avformat_alloc_context();

  pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);
  if (avio_open(&pFormatCtx->pb, dumpPath, AVIO_FLAG_READ_WRITE) < 0) {
    AVENGINE_ERR("avio_open error!");
    return -1;
  }
  if (!pFormatCtx->oformat) {
    AVENGINE_ERR("oformat error!");
    return -1;
  }
  AVStream* pAVStream = avformat_new_stream(pFormatCtx, 0);
  if (pAVStream == NULL) {
    AVENGINE_ERR("pAVStream error!");
    return -1;
  }

  AVCodecContext* pCodecCtx = pAVStream->codec;
  pCodecCtx->codec_id = pFormatCtx->oformat->video_codec;
  pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
  pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
  pCodecCtx->width = width;
  pCodecCtx->height = height;
  pCodecCtx->time_base.num = 1;
  pCodecCtx->time_base.den = 25;
  // Begin Output some information
  av_dump_format(pFormatCtx, 0, dumpPath, 1);
  // End Output some information
  AVCodec* pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
  if (!pCodec) {
    AVENGINE_ERR("pCodec error!");
    return -1;
  }
  if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
    AVENGINE_ERR("avcodec_open2 error!");
    return -1;
  }
  //Write Header
  avformat_write_header(pFormatCtx, NULL);
  int y_size = pCodecCtx->width * pCodecCtx->height;
  //Encode
  AVPacket pkt;
  av_new_packet(&pkt, y_size * 3);

  int got_picture = 0;
  int ret = avcodec_encode_video2(pCodecCtx, &pkt, frame, &got_picture);
  if (ret < 0) {
    AVENGINE_ERR("avcodec_encode_video2 error!");
    return -1;
  }
  if (got_picture == 1) {
    //pkt.stream_index = pAVStream->index;
    ret = av_write_frame(pFormatCtx, &pkt);
  }
  av_free_packet(&pkt);
  //Write Trailer
  av_write_trailer(pFormatCtx);
  if (pAVStream) {
    avcodec_close(pAVStream->codec);
  }
  avio_close(pFormatCtx->pb);
  avformat_free_context(pFormatCtx);

  if (frame)
    av_frame_free(&frame);

  return 0;
}


//////////////////////////////////////////////////////////////////////////

class lfRtcAudioEncoderInternal {
public:
  lfRtcAudioEncoderInternal();
  ~lfRtcAudioEncoderInternal();
  int Open(const lfrtcAudioEncodeParams *params);
  int GetFrameSamplesPerChannel();
  void Close();

  int Process(unsigned char *data[3], int len[3]);
  void* GetEncodeData();
  int GetEncodeLen();

private:
  int OpenInternal();
  std::vector<unsigned char> m_encoded_data;
  lfrtcAudioEncodeParams m_params;
  int m_nb_samples;

  AVCodec *m_codec;
  AVFrame *m_frame;
  AVCodecContext *m_codec_ctx;
  AVBitStreamFilterContext* m_bsfc;
  int m_sdp_flag;
};

lfRtcAudioEncoderInternal::lfRtcAudioEncoderInternal() {
  memset(&m_params, 0, sizeof(m_params));
  m_nb_samples = 0;
  m_codec = NULL;
  m_frame = NULL;
  m_codec_ctx = NULL;
  m_bsfc = NULL;
  m_sdp_flag = 1;
}

lfRtcAudioEncoderInternal::~lfRtcAudioEncoderInternal() {
  Close();
}

int lfRtcAudioEncoderInternal::Open(const lfrtcAudioEncodeParams *params) {
  initFFmpeg();
  Close();
  memcpy(&m_params, params, sizeof(m_params));
  if (m_params.codec_id == 0) {
    m_params.codec_id = AV_CODEC_ID_AAC;
  }
  int ret = OpenInternal();
  if (ret < 0) {
    Close();
  }
  return ret;
}

int lfRtcAudioEncoderInternal::GetFrameSamplesPerChannel() {
  return m_nb_samples;
}

void lfRtcAudioEncoderInternal::Close() {
  if (m_codec_ctx) {
    avcodec_free_context(&m_codec_ctx);
    m_codec_ctx = NULL;
  }
  if (m_codec) {
    m_codec = NULL;
  }
  if (m_frame) {
    av_frame_free(&m_frame);
    m_frame = NULL;
  }
  if (m_bsfc) {
    av_bitstream_filter_close(m_bsfc);
    m_bsfc = NULL;
  }
}

int lfRtcAudioEncoderInternal::Process(unsigned char *data[3], int len[3]) {
  assert(m_nb_samples == len[0] / av_get_bytes_per_sample(FIXED_AUDIO_SAMPLE_FMT) / m_codec_ctx->channels);
  m_encoded_data.clear();

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = NULL;
  pkt.size = 0;

  m_frame->data[0] = data[0];
  // 本段代码实际上没有用到pts，实际上这里可以不用赋值
  m_frame->pts = m_codec_ctx->frame_number * m_frame->nb_samples;

  int got_output = 0;
  int ret = avcodec_encode_audio2(m_codec_ctx, &pkt, m_frame, &got_output);
  if (ret < 0) {
    av_free_packet(&pkt);
    AVENGINE_ERR("avcodec_encode_audio2 failed, ret=%d", ret);
    assert(false);
    return -1;
  }
  if (!got_output) {
    // it means the data was buffered.
    av_free_packet(&pkt);
    return 0;
  }

  if (m_sdp_flag) {
    // extradata[0]和extradata[1]组成了AudioSpecificConfig结构
    // AudioSpecificConfig结构由ISO-14496-3 Audio规定
    // 5bits, audioObjectType
    // 4bits, samplingFrequencyIndex
    // 4bits, channelConfiguration
    // 1bits, frameLengthFlag
    // 1bits, dependsOnCoreCoder
    // 1bits, extensionFlag
    assert(m_codec_ctx->extradata_size >= 2);
    unsigned char AudioSpecificConfig0 = m_codec_ctx->extradata[0];
    unsigned char AudioSpecificConfig1 = m_codec_ctx->extradata[1];
    //unsigned char audioObjectType = AudioSpecificConfig0 >> 3;
    unsigned char samplingFrequencyIndex = ((AudioSpecificConfig0 & 0x07) << 1) | (AudioSpecificConfig1 >> 7);
    unsigned char channelConfiguration = (AudioSpecificConfig1 >> 3) & 0x0F;
    //unsigned char frameLengthFlag = (AudioSpecificConfig1 >> 2) & 0x01;
    //unsigned char dependsOnCoreCoder = (AudioSpecificConfig1 >> 1) & 0x01;
    //unsigned char extensionFlag = AudioSpecificConfig1 & 0x01;

    m_encoded_data.resize(7 + pkt.size);
    unsigned char *outBuf = &m_encoded_data[0];
    int length = 7 + pkt.size;
    outBuf[0] = 0xff;
    outBuf[1] = 0xf1;
    outBuf[2] = 0x40 | (samplingFrequencyIndex << 2) | (channelConfiguration >> 2);
    outBuf[3] = ((channelConfiguration & 0x3) << 6) | (length >> 11);
    outBuf[4] = (length >> 3) & 0xff;
    outBuf[5] = ((length << 5) & 0xff) | 0x1f;
    outBuf[6] = 0xfc;
    memcpy(outBuf + 7, pkt.data, pkt.size);
  }
  else {
    m_encoded_data.resize(pkt.size);
    memcpy(&m_encoded_data[0], pkt.data, pkt.size);
  }

  av_free_packet(&pkt);
  return 0;
}

void* lfRtcAudioEncoderInternal::GetEncodeData() {
  if (m_encoded_data.empty()) {
    return NULL;
  }
  else {
    return &m_encoded_data[0];
  }
}

int lfRtcAudioEncoderInternal::GetEncodeLen() {
  return m_encoded_data.size();
}

AVCodecContext *CreateAudioCodec(AVCodec *codec, enum AVSampleFormat sample_fmt, int sample_rate, int channels, int bitrate) {
  if (codec->sample_fmts) {
    bool sample_fmt_supported = false;
    for (int i = 0; codec->sample_fmts[i] != -1; i++) {
      if (codec->sample_fmts[i] == sample_fmt) {
        sample_fmt_supported = true;
        break;
      }
    }
    if (!sample_fmt_supported) {
      return NULL;
    }
  }

  if (codec->supported_samplerates) {
    bool sample_rate_supported = false;
    for (int i = 0; codec->supported_samplerates[i]; i++) {
      if (codec->supported_samplerates[i] == sample_rate) {
        sample_rate_supported = true;
        break;
      }
    }
    if (!sample_rate_supported) {
      return NULL;
    }
  }

  int channel_layout = 0;
  if (channels == 1) {
    channel_layout = AV_CH_LAYOUT_MONO;
  }
  else if (channels == 2) {
    channel_layout = AV_CH_LAYOUT_STEREO;
  }
  else {
    return NULL;
  }
  if (codec->channel_layouts) {
    bool channel_layout_supported = false;
    for (int i = 0; codec->channel_layouts[i]; i++) {
      if (codec->channel_layouts[i] == channel_layout) {
        channel_layout_supported = true;
        break;
      }
    }
    if (!channel_layout_supported) {
      return NULL;
    }
  }

  AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
  if (!codec_ctx) {
    return NULL;
  }
  codec_ctx->time_base.den = sample_rate;
  codec_ctx->time_base.num = 1;
  codec_ctx->sample_fmt = sample_fmt;
  codec_ctx->sample_rate = sample_rate;
  codec_ctx->channel_layout = channel_layout;
  codec_ctx->channels = channels;
  codec_ctx->bit_rate = bitrate;
  codec_ctx->frame_size = 1024;
  codec_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;
  if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
    avcodec_free_context(&codec_ctx);
    return NULL;
  }
  if (codec_ctx->sample_fmt != sample_fmt
    || codec_ctx->sample_rate != sample_rate
    || codec_ctx->channel_layout != channel_layout) {
    avcodec_free_context(&codec_ctx);
    return NULL;
  }
  return codec_ctx;  // need to be freed(avcodec_free_context)
}

int lfRtcAudioEncoderInternal::OpenInternal() {
  //m_codec = avcodec_find_encoder_by_name("libfdk_aac");
  m_codec = avcodec_find_encoder((AVCodecID)m_params.codec_id);
  if (!m_codec) {
    return -1;
  }

  while (m_codec) {
    // 某个aac编码器不一定支持我们要求的sample_fmt、sample_rate、channels，所以得挨个尝试
    if (m_codec->id == AV_CODEC_ID_AAC) {
      AVCodecContext *codec_ctx
        = CreateAudioCodec(m_codec, FIXED_AUDIO_SAMPLE_FMT, m_params.sample_rate, m_params.channel_count, m_params.bitrate);
      if (codec_ctx) {
        m_codec_ctx = codec_ctx;
        break;
      }
    }
    m_codec = av_codec_next(m_codec);
  }

  if (!m_codec_ctx) {
    return -2;
  }

  m_frame = av_frame_alloc();
  m_frame->channels = m_codec_ctx->channels;
  m_frame->sample_rate = m_codec_ctx->sample_rate;
  m_frame->nb_samples = m_codec_ctx->frame_size;
  if (m_frame->nb_samples <= 0) {
    m_frame->nb_samples = 1024;
  }
  m_frame->format = m_codec_ctx->sample_fmt;
  m_frame->channel_layout = m_codec_ctx->channel_layout;
  m_nb_samples = m_frame->nb_samples;

  m_bsfc = av_bitstream_filter_init("aac_adtstoasc");
  return 0;
}

//////////////////////////////////////////////////////////////////////////

class lfRtcAudioDecoderInternal {
public:
  lfRtcAudioDecoderInternal();
  ~lfRtcAudioDecoderInternal();
  int Open(const lfrtcAudioDecodeParams *params);
  void Close();

  int Process(unsigned char *data, int len);
  void* GetDecodeData(int plain_index);
  int GetDecodeLen(int plain_index);

private:
  int OpenInternal();
  std::vector<unsigned char> m_decoded_data;
  lfrtcAudioDecodeParams m_params;

  AVCodec *m_codec;
  AVFrame *m_frame;
  AVCodecContext *m_codec_ctx;
  AVBitStreamFilterContext* m_bsfc;
  int m_sdp_flag;
  FILE *m_debug_file;
  unsigned char m_audio_buf[2048 * 4 * 4];
  void *m_audio_resample;
};

lfRtcAudioDecoderInternal::lfRtcAudioDecoderInternal() {
  memset(&m_params, 0, sizeof(m_params));
  m_codec = NULL;
  m_frame = NULL;
  m_codec_ctx = NULL;
  m_bsfc = NULL;
  m_sdp_flag = 1;
  m_audio_resample = NULL;
}

lfRtcAudioDecoderInternal::~lfRtcAudioDecoderInternal() {
  Close();
}

int lfRtcAudioDecoderInternal::Open(const lfrtcAudioDecodeParams *params) {
  initFFmpeg();
  Close();
  memcpy(&m_params, params, sizeof(m_params));
  if (m_params.codec_id == 0) {
    m_params.codec_id = AV_CODEC_ID_AAC;
  }
  int ret = OpenInternal();
  if (ret < 0) {
    Close();
  }
  return ret;
}

void lfRtcAudioDecoderInternal::Close() {
  if (m_audio_resample) {
    AudioResampleClose(m_audio_resample);
    m_audio_resample = NULL;
  }
  if (m_codec_ctx) {
    avcodec_free_context(&m_codec_ctx);
    m_codec_ctx = NULL;
  }
  if (m_codec) {
    m_codec = NULL;
  }
  if (m_frame) {
    av_frame_free(&m_frame);
    m_frame = NULL;
  }
  if (m_bsfc) {
    av_bitstream_filter_close(m_bsfc);
    m_bsfc = NULL;
  }
}

int lfRtcAudioDecoderInternal::Process(unsigned char *data, int len) {
  m_decoded_data.clear();

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.size = len;
  pkt.data = data;

  int got_frame = 0;
  while (pkt.size > 0) {
    int read_size = avcodec_decode_audio4(m_codec_ctx, m_frame, &got_frame, &pkt);
    if (read_size < 0) {
      return -1;
    }

    if (got_frame) {
      if (m_audio_resample == NULL) {
        m_audio_resample = AudioResampleCreate();
      }
      AudioResampleSrcConfig(m_audio_resample, m_frame->format, m_frame->channels, m_frame->nb_samples, m_frame->sample_rate);
      int dst_nb_samples = (m_params.sample_rate * m_frame->nb_samples) / m_frame->sample_rate;
      AudioResampleDstConfig(m_audio_resample, FIXED_AUDIO_SAMPLE_FMT, m_params.channel_count, dst_nb_samples, m_params.sample_rate);
      uint8_t *resample_out[1] = { m_audio_buf };
      int decoded_len = AudioResampleProcess(m_audio_resample, resample_out, m_frame->data) * av_get_bytes_per_sample(FIXED_AUDIO_SAMPLE_FMT) * m_params.channel_count;

      int outLen = m_decoded_data.size();
      m_decoded_data.resize(outLen + decoded_len);
      memcpy(&m_decoded_data[outLen], m_audio_buf, decoded_len);
    }

    if (pkt.data) {
      pkt.size -= read_size;
      pkt.data += read_size;
    }
  }
  return 0;
}

void* lfRtcAudioDecoderInternal::GetDecodeData(int plain_index) {
  if (plain_index != 0 || m_decoded_data.empty()) {
    return NULL;
  }
  else {
    return &m_decoded_data[0];
  }
}

int lfRtcAudioDecoderInternal::GetDecodeLen(int plain_index) {
  if (plain_index == 0) {
    return m_decoded_data.size();
  }
  else {
    return 0;
  }
}

int lfRtcAudioDecoderInternal::OpenInternal() {
  m_codec = avcodec_find_decoder((AVCodecID)m_params.codec_id);
  if (!m_codec) {
    return -1;
  }
  m_codec_ctx = avcodec_alloc_context3(m_codec);
  if (!m_codec_ctx) {
    return -2;
  }
  if (avcodec_open2(m_codec_ctx, m_codec, NULL) < 0) {
    return -3;
  }

  m_frame = av_frame_alloc();
  m_bsfc = av_bitstream_filter_init("aac_adtstoasc");
  return 0;
}

//////////////////////////////////////////////////////////////////////////

lfRtcAudioEncoder::lfRtcAudioEncoder() {
  m_internal = new lfRtcAudioEncoderInternal();
}

lfRtcAudioEncoder::~lfRtcAudioEncoder() {
  delete m_internal;
}

int lfRtcAudioEncoder::Open(const lfrtcAudioEncodeParams *params) {
  return m_internal->Open(params);
}

int lfRtcAudioEncoder::GetFrameSamplesPerChannel() {
  return m_internal->GetFrameSamplesPerChannel();
}

void lfRtcAudioEncoder::Close() {
  m_internal->Close();
}

int lfRtcAudioEncoder::Process(unsigned char *data[3], int len[3]) {
  return m_internal->Process(data, len);
}

void* lfRtcAudioEncoder::GetEncodeData() {
  return m_internal->GetEncodeData();
}

int lfRtcAudioEncoder::GetEncodeLen() {
  return m_internal->GetEncodeLen();
}

//////////////////////////////////////////////////////////////////////////

lfRtcAudioDecoder::lfRtcAudioDecoder() {
  m_internal = new lfRtcAudioDecoderInternal();
}

lfRtcAudioDecoder::~lfRtcAudioDecoder() {
  delete m_internal;
}

int lfRtcAudioDecoder::Open(const lfrtcAudioDecodeParams *params) {
  return m_internal->Open(params);
}

void lfRtcAudioDecoder::Close() {
  m_internal->Close();
}

int lfRtcAudioDecoder::Process(unsigned char *data, int len) {
  return m_internal->Process(data, len);
}

void* lfRtcAudioDecoder::GetDecodeData(int plain_index) {
  return m_internal->GetDecodeData(plain_index);
}

int lfRtcAudioDecoder::GetDecodeLen(int plain_index) {
  return m_internal->GetDecodeLen(plain_index);
}

//////////////////////////////////////////////////////////////////////////

class lfRtcVideoEncoderInternal {
public:
  lfRtcVideoEncoderInternal();
  ~lfRtcVideoEncoderInternal();
  int Open(const lfrtcVideoEncodeParams *params);
  void Close();

  int Process(unsigned char *data[3], int linesize[3], bool *keyframe);
  void* GetEncodeData();
  int GetEncodeLen();

private:
  int OpenInternal();
  std::vector<unsigned char> m_encoded_data;
  lfrtcVideoEncodeParams m_params;

  AVCodec *m_codec;
  AVFrame *m_frame;
  AVCodecContext *m_codec_ctx;
  AVBitStreamFilterContext* m_bsfc;
  int m_sdp_flag;
};

lfRtcVideoEncoderInternal::lfRtcVideoEncoderInternal() {
  memset(&m_params, 0, sizeof(m_params));
  m_codec = NULL;
  m_frame = NULL;
  m_codec_ctx = NULL;
  m_bsfc = NULL;
  m_sdp_flag = 1;
}

lfRtcVideoEncoderInternal::~lfRtcVideoEncoderInternal() {
  Close();
}

int lfRtcVideoEncoderInternal::Open(const lfrtcVideoEncodeParams *params) {
  initFFmpeg();
  Close();
  memcpy(&m_params, params, sizeof(m_params));
  if (m_params.codec_id == 0) {
    m_params.codec_id = AV_CODEC_ID_H264;
  }
  int ret = OpenInternal();
  if (ret < 0) {
    Close();
  }
  return ret;
}

void lfRtcVideoEncoderInternal::Close() {
  if (m_codec_ctx) {
    avcodec_free_context(&m_codec_ctx);
    m_codec_ctx = NULL;
  }
  if (m_codec) {
    m_codec = NULL;
  }
  if (m_frame) {
    av_frame_free(&m_frame);
    m_frame = NULL;
  }
  if (m_bsfc) {
    av_bitstream_filter_close(m_bsfc);
    m_bsfc = NULL;
  }
}

int lfRtcVideoEncoderInternal::Process(unsigned char *data[3], int linesize[3], bool *keyframe) {
  m_encoded_data.clear();

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = NULL;
  pkt.size = 0;

  // pts在有b帧时有用
  m_frame->pts = m_codec_ctx->frame_number;
  m_frame->data[0] = (uint8_t *)data[0];
  m_frame->data[1] = (uint8_t *)data[1];
  m_frame->data[2] = (uint8_t *)data[2];
  // 不能保证linesize[0] == c->width，因此需要调用者传入
  m_frame->linesize[0] = linesize[0];
  m_frame->linesize[1] = linesize[1];
  m_frame->linesize[2] = linesize[2];
  m_frame->width = m_codec_ctx->width;
  m_frame->height = m_codec_ctx->height;
  m_frame->format = m_codec_ctx->pix_fmt; //AV_PIX_FMT_YUV420P;
  if (keyframe) {
    m_frame->key_frame = (int)(*keyframe);
    *keyframe = false;
  }
  else {
    m_frame->key_frame = 0;
  }

  int got_output = 0;
  int ret = avcodec_encode_video2(m_codec_ctx, &pkt, m_frame, &got_output);
  if (ret < 0) {
    av_free_packet(&pkt);
    return -2;
  }
  if (!got_output) {
    // it means the data was buffered.
    av_free_packet(&pkt);
    return 0;
  }

  if (pkt.flags & AV_PKT_FLAG_KEY && keyframe) {
    *keyframe = true;
  }
  if (m_sdp_flag && (pkt.flags & AV_PKT_FLAG_KEY)) {
    m_encoded_data.resize(pkt.size + m_codec_ctx->extradata_size);
    memcpy(&m_encoded_data[0], m_codec_ctx->extradata, m_codec_ctx->extradata_size);
    memcpy(&m_encoded_data[m_codec_ctx->extradata_size], pkt.data, pkt.size);
  }
  else {
    m_encoded_data.resize(pkt.size);
    memcpy(&m_encoded_data[0], pkt.data, pkt.size);
  }

//  if (!mst->ofp && mst->params.debugInfo) {
//#ifdef _WIN32
//    mst->ofp = fopen("e://works//test//video.h264", "wb");
//#elif defined(__ANDROID__)
//    mst->ofp = fopen("/sdcard/video.h264", "wb");
//#elif defined(WEBRTC_IOS)
//    char dirName[256] = "video.h264";
//    char *pName = MAKE_FILE_NAME(dirName);
//    mst->ofp = fopen(pName, "wb");
//    free(pName);
//#endif
//  }
//  if (mst->ofp) {
//    fwrite(outBuf[0], 1, ret, mst->ofp);
//  }

  av_free_packet(&pkt);
  return 0;
}

void* lfRtcVideoEncoderInternal::GetEncodeData() {
  if (m_encoded_data.empty()) {
    return NULL;
  }
  else {
    return &m_encoded_data[0];
  }
}

int lfRtcVideoEncoderInternal::GetEncodeLen() {
  return m_encoded_data.size();
}

int lfRtcVideoEncoderInternal::OpenInternal() {
  m_codec = avcodec_find_encoder((AVCodecID)m_params.codec_id);
  if (!m_codec) {
    return -1;
  }
  m_codec_ctx = avcodec_alloc_context3(m_codec);
  if (!m_codec_ctx) {
    return -2;
  }

  m_codec_ctx->time_base.den = m_params.fps;
  m_codec_ctx->time_base.num = 1;
  int bitsrate = m_params.bitrate;
  m_codec_ctx->width = m_params.width;
  m_codec_ctx->height = m_params.height;
  m_codec_ctx->pix_fmt = FIXED_VIDEO_PIX_FMT;
  m_codec_ctx->qmax = 40;
  m_codec_ctx->qmin = 20;
  //c->refs = 2;
  m_codec_ctx->b_frame_strategy = false;
  m_codec_ctx->max_b_frames = 0;
  m_codec_ctx->coder_type = m_codec_ctx->max_b_frames ? FF_CODER_TYPE_AC : FF_CODER_TYPE_VLC;
  m_codec_ctx->coder_type = FF_CODER_TYPE_AC;//test
  //c->cqp = 24;
  m_codec_ctx->bit_rate = bitsrate;
  //c->rc_max_rate = bitsrate;
  m_codec_ctx->gop_size = m_params.gop_size;
  m_codec_ctx->thread_type = FF_THREAD_SLICE; //FF_THREAD_FRAME;
  //c->thread_count = 1;
  if (!strcmp(m_params.preset, "")) {
    av_opt_set(m_codec_ctx->priv_data, "preset", "superfast", 0);
    av_opt_set(m_codec_ctx->priv_data, "tune", "zerolatency", 0);
    // slice-max-size基本上规定了max_nalu，为方便udp传输，将最大值设为1300
    av_opt_set(m_codec_ctx->priv_data, "x264opts", "slice-max-size=1300", 0);
  }
  else {
    av_opt_set(m_codec_ctx->priv_data, "preset", m_params.preset, 0);
  }
  m_codec_ctx->rc_buffer_size = bitsrate;
  //av_opt_set(c->priv_data, "x264opts","slice-max-size=1400:keyint=50",0);
  //av_opt_set(c->priv_data, "preset", "ultrafast", 0);
  //av_opt_set(c->priv_data, "tune","stillimage,fastdecode,zerolatency",0);
  //av_opt_set(c->priv_data, "x264opts","crf=26:vbv-maxrate=728:vbv-bufsize=364:keyint=50:slice-max-size=1400",0);//:mtu=1400:sliced-threads=1

  m_codec_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;
  if (avcodec_open2(m_codec_ctx, m_codec, NULL) < 0) {
    return -3;
  }
  if (m_codec_ctx->pix_fmt != FIXED_VIDEO_PIX_FMT
    || m_codec_ctx->width != m_params.width
    || m_codec_ctx->height != m_params.height) {
    return -4;
  }

  m_frame = av_frame_alloc();
  m_bsfc = av_bitstream_filter_init("h264_mp4toannexb");
  return 0;
}

//////////////////////////////////////////////////////////////////////////

lfRtcVideoEncoder::lfRtcVideoEncoder() {
  m_internal = new lfRtcVideoEncoderInternal();
}

lfRtcVideoEncoder::~lfRtcVideoEncoder() {
  delete m_internal;
}

int lfRtcVideoEncoder::Open(const lfrtcVideoEncodeParams *params) {
  return m_internal->Open(params);
}

void lfRtcVideoEncoder::Close() {
  m_internal->Close();
}

int lfRtcVideoEncoder::Process(unsigned char *data[3], int linesize[3], bool *keyframe) {
  return m_internal->Process(data, linesize, keyframe);
}

void* lfRtcVideoEncoder::GetEncodeData() {
  return m_internal->GetEncodeData();
}

int lfRtcVideoEncoder::GetEncodeLen() {
  return m_internal->GetEncodeLen();
}

//////////////////////////////////////////////////////////////////////////

class lfRtcVideoDecoderInternal {
public:
  lfRtcVideoDecoderInternal();
  ~lfRtcVideoDecoderInternal();
  int Open(const lfrtcVideoDecodeParams *params);
  void Close();

  int Process(unsigned char *inBuf, int inLen, unsigned char *outBuf[3], int width, int height);
  int GetWidth();
  int GetHeight();
  int GetFrameType();
  void* GetExtraDecodeData(int plain_index);

private:
  int OpenInternal();
  std::vector<unsigned char> m_decoded_data[3];
  lfrtcVideoDecodeParams m_params;

  AVCodec *m_codec;
  AVFrame *m_frame;
  AVCodecContext *m_codec_ctx;
  AVBitStreamFilterContext* m_bsfc;
  int m_sdp_flag;
};

lfRtcVideoDecoderInternal::lfRtcVideoDecoderInternal() {
  memset(&m_params, 0, sizeof(m_params));
  m_codec = NULL;
  m_frame = NULL;
  m_codec_ctx = NULL;
  m_bsfc = NULL;
  m_sdp_flag = 1;
}

lfRtcVideoDecoderInternal::~lfRtcVideoDecoderInternal() {
  Close();
}

int lfRtcVideoDecoderInternal::Open(const lfrtcVideoDecodeParams *params) {
  initFFmpeg();
  Close();
  memcpy(&m_params, params, sizeof(m_params));
  if (m_params.codec_id == 0) {
    m_params.codec_id = AV_CODEC_ID_H264;
  }
  int ret = OpenInternal();
  if (ret < 0) {
    Close();
  }
  return ret;
}

void lfRtcVideoDecoderInternal::Close() {
  if (m_codec_ctx) {
    avcodec_free_context(&m_codec_ctx);
    m_codec_ctx = NULL;
  }
  if (m_codec) {
    m_codec = NULL;
  }
  if (m_frame) {
    av_frame_free(&m_frame);
    m_frame = NULL;
  }
  if (m_bsfc) {
    av_bitstream_filter_close(m_bsfc);
    m_bsfc = NULL;
  }
}

int lfRtcVideoDecoderInternal::Process(unsigned char *inBuf, int inLen, unsigned char *outBuf[3], int width, int height) {
  m_decoded_data[0].clear();
  m_decoded_data[1].clear();
  m_decoded_data[2].clear();
  m_frame->pict_type = AVPictureType::AV_PICTURE_TYPE_NONE;

  uint8_t *free_buf = NULL;

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.size = inLen;
  pkt.data = inBuf;
  pkt.flags = 0;
  if (m_sdp_flag) {
    uint8_t *filter_buf = NULL;
    int filter_buf_size = 0;
    int ret = av_bitstream_filter_filter(m_bsfc, m_codec_ctx, NULL, &filter_buf, &filter_buf_size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
    if (ret > 0) {
      // 此时有重新分配内存
      pkt.data = filter_buf;
      pkt.size = filter_buf_size;
      free_buf = filter_buf;
    }
    else if (ret == 0 && filter_buf != NULL) {
      // 此时未重新分配内存，但filter_buf指向原内存的某个地方
      pkt.data = filter_buf;
      pkt.size = filter_buf_size;
    }
  }

  int ret = lfRtcVideoDecoder::VIDEO_DECODE_SUCCESS_NO_PICTRUE;
  //frame->key_frame = 0;
  while (pkt.size > 0) {
    m_decoded_data[0].clear();
    m_decoded_data[1].clear();
    m_decoded_data[2].clear();
    // TODO: zhangle，研究ffmpeg_decoder_error
    ffmpeg_decoder_error = 0;
    int got_picture = 0;
    int usedLen = avcodec_decode_video2(m_codec_ctx, m_frame, &got_picture, &pkt);
    if (usedLen < 0) {
      if (free_buf) {
        av_free(free_buf);
      }
      return lfRtcVideoDecoder::VIDEO_DECODE_FAILED;
    }

    if (got_picture && m_frame->width > 0 && m_frame->height > 0) {
      bool use_inter_buf = (width != m_frame->width) || (height != m_frame->height);
      if (use_inter_buf) {
        m_decoded_data[0].resize(m_frame->width * m_frame->height);
        m_decoded_data[1].resize(m_frame->width * m_frame->height / 4);
        m_decoded_data[2].resize(m_frame->width * m_frame->height / 4);
        outBuf[0] = &m_decoded_data[0][0];
        outBuf[1] = &m_decoded_data[1][0];
        outBuf[2] = &m_decoded_data[2][0];
      }

      // 下面的这段代码就是为了使linesize == width
      //Y
      {
        unsigned char *dst_buf = outBuf[0];
        int dst_width = m_frame->width;
        int dst_index = 0;
        unsigned char *src_buf = m_frame->data[0];
        int src_width = m_frame->linesize[0];
        int src_index = 0;
        int copy_count = m_frame->height;
        for (int i = 0; i < copy_count; i++) {
          memcpy(dst_buf + dst_index, src_buf + src_index, dst_width);
          dst_index += dst_width;
          src_index += src_width;
        }
      }
      //U
      {
        unsigned char *dst_buf = outBuf[1];
        int dst_width = m_frame->width/2;
        int dst_index = 0;
        unsigned char *src_buf = m_frame->data[1];
        int src_width = m_frame->linesize[1];
        int src_index = 0;
        int copy_count = m_frame->height/2;
        for (int i = 0; i < copy_count; i++) {
          memcpy(dst_buf + dst_index, src_buf + src_index, dst_width);
          dst_index += dst_width;
          src_index += src_width;
        }
      }
      //V
      {
        unsigned char *dst_buf = outBuf[2];
        int dst_width = m_frame->width / 2;
        int dst_index = 0;
        unsigned char *src_buf = m_frame->data[2];
        int src_width = m_frame->linesize[2];
        int src_index = 0;
        int copy_count = m_frame->height / 2;
        for (int i = 0; i < copy_count; i++) {
          memcpy(dst_buf + dst_index, src_buf + src_index, dst_width);
          dst_index += dst_width;
          src_index += src_width;
        }
      }

      if (use_inter_buf) {
        ret = lfRtcVideoDecoder::VIDEO_DECODE_SUCCESS_ON_SIZE;
      }
      else {
        ret = lfRtcVideoDecoder::VIDEO_DECODE_SUCCESS;
      }

//      if (!mst->ofp && mst->params.debugInfo) {
//#ifdef _WIN32
//        mst->ofp = fopen("e://works//test//video-d.yuv", "wb");
//#elif defined(__ANDROID__)
//        mst->ofp = fopen("/sdcard/video-d.yuv", "wb");
//#elif defined(WEBRTC_IOS)
//        char dirName[256] = "video-d.yuv";
//        char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
//        mst->ofp = fopen(pName, "wb");
//        free(pName);
//#endif
//      }
//      if (mst->ofp) {
//        fwrite(outBuf[0], 1, frame->width * frame->height, mst->ofp);
//        fwrite(outBuf[1], 1, ((frame->width * frame->height) >> 2), mst->ofp);
//        fwrite(outBuf[2], 1, ((frame->width * frame->height) >> 2), mst->ofp);
//      }
    }

    if (pkt.data) {
      pkt.size -= usedLen;
      pkt.data += usedLen;
    }
  }

  if (free_buf) {
    av_free(free_buf);
  }
  return ret;
}

int lfRtcVideoDecoderInternal::GetWidth() {
  if (m_frame) {
    return m_frame->width;
  }
  else {
    return 0;
  }
}

int lfRtcVideoDecoderInternal::GetHeight() {
  if (m_frame) {
    return m_frame->height;
  }
  else {
    return 0;
  }
}

int lfRtcVideoDecoderInternal::GetFrameType() {
  if (m_frame) {
    return m_frame->pict_type;
  }
  else {
    return AVPictureType::AV_PICTURE_TYPE_NONE;
  }
}

void* lfRtcVideoDecoderInternal::GetExtraDecodeData(int plain_index) {
  if (plain_index < 0 || plain_index > 2 || m_decoded_data[plain_index].empty()) {
    return NULL;
  }
  else {
    return &m_decoded_data[plain_index][0];
  }
}

int lfRtcVideoDecoderInternal::OpenInternal() {
  m_codec = avcodec_find_decoder((AVCodecID)m_params.codec_id);
  if (!m_codec) {
    return -1;
  }
  m_codec_ctx = avcodec_alloc_context3(m_codec);
  if (!m_codec_ctx) {
    return -2;
  }

  m_codec_ctx->thread_type = FF_THREAD_SLICE;

  // 如果每次送给解码器的数据不是一个完整的帧，那么应开启下面的三行代码，
  // 此时ffmpeg会等到检测到下一帧图像的头才去解上一帧
  //if (m_codec->capabilities & CODEC_CAP_TRUNCATED) {
  //  m_codec_ctx->flags |= CODEC_FLAG_TRUNCATED;
  //}
  if (avcodec_open2(m_codec_ctx, m_codec, NULL) < 0) {
    return -3;
  }

  m_frame = av_frame_alloc();
  m_bsfc = av_bitstream_filter_init("h264_mp4toannexb");
  return 0;
}

//////////////////////////////////////////////////////////////////////////

lfRtcVideoDecoder::lfRtcVideoDecoder() {
  m_internal = new lfRtcVideoDecoderInternal();
}

lfRtcVideoDecoder::~lfRtcVideoDecoder() {
  delete m_internal;
}

int lfRtcVideoDecoder::Open(const lfrtcVideoDecodeParams *params) {
  return m_internal->Open(params);
}

void lfRtcVideoDecoder::Close() {
  m_internal->Close();
}

int lfRtcVideoDecoder::Process(unsigned char *inBuf, int inLen, unsigned char *outBuf[3], int width, int height) {
  return m_internal->Process(inBuf, inLen, outBuf, width, height);
}

int lfRtcVideoDecoder::GetWidth() {
  return m_internal->GetWidth();
}

int lfRtcVideoDecoder::GetHeight() {
  return m_internal->GetHeight();
}

int lfRtcVideoDecoder::GetFrameType() {
  return m_internal->GetFrameType();
}

void* lfRtcVideoDecoder::GetExtraDecodeData(int plain_index) {
  return m_internal->GetExtraDecodeData(plain_index);
}
