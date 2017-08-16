//edited by zhangle
#ifndef AVENGINE_SOURCE_FFMPEG_API_H_
#define AVENGINE_SOURCE_FFMPEG_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define FFAVCODEC_API __declspec(dllexport)
#else
#define FFAVCODEC_API __attribute__ ((__visibility__("default")))
#endif

struct lfrtcAudioEncodeParams {
  // 以下三个是编码器输入PCM数据的参数
  // int sample_fmt; // 只接受AV_SAMPLE_FMT_S16格式
  int channel_count; // 等于1和2时，表示AV_CH_LAYOUT_MONO/AV_CH_LAYOUT_STEREO格式
  int sample_rate;

  // 以下参数是对编码器的设置
  int bitrate;
  int codec_id;  // 参见ffmpeg的enum AVCodecID。 等于0时程序会默认以AAC编码
};

struct lfrtcAudioDecodeParams {
  // 以下三个参数是业务方期望得到的PCM数据格式，如果解码器的输出格式不符合之，将会进行resample后再输出
  // int sample_fmt; // 只接受AV_SAMPLE_FMT_S16格式
  int channel_count; // 等于1和2时，表示AV_CH_LAYOUT_MONO/AV_CH_LAYOUT_STEREO格式
  int sample_rate;

  int codec_id;  // 参见ffmpeg的enum AVCodecID。 等于0时程序会默认以AAC解码
};

struct lfrtcVideoEncodeParams {
  // 以下四个是编码器输入原始数据的参数
  // int data_format; // 只接收AV_PIX_FMT_YUV420P格式
  int width;
  int height;
  int fps;

  int bitrate;
  int gop_size;
  int codec_id;  // 参见ffmpeg的enum AVCodecID。 等于0时程序会默认以H264编码
  char preset[32];
};

struct lfrtcVideoDecodeParams {
  // int data_format; // 只接收AV_PIX_FMT_YUV420P格式

  int codec_id;  // 参见ffmpeg的enum AVCodecID。 等于0时程序会默认以H264解码
};

#if 0
// nb_sample，编码器支持的帧大小
FFAVCODEC_API void *lfrtcCreateAudioEncoder(const lfrtcAudioEncodeParams *params, int *nb_sample);
FFAVCODEC_API void *lfrtcCreateAudioDecoder(const lfrtcAudioDecodeParams *params);
FFAVCODEC_API void *lfrtcCreateVideoEncoder(const lfrtcVideoEncodeParams *params);
FFAVCODEC_API void *lfrtcCreateVideoDecoder(const lfrtcVideoDecodeParams *params);

FFAVCODEC_API void lfrtcFFDeleteCodec(void *codecCtx);

// inBuf, inlen, outBuf, outLen: 接plane传递，如果只有一个plane，则只使用数组的第1个元素，数组的其它值为0
// outLen: 为输入输出参数。输入时表示buf的容量，输出时表示实际数据的长度
// keyframe: 为输入输出参数。输入时keyframe = true表示强制IDR帧编码，输出时表示当前的实际类型
// frameType: 为输出参数。实际上应为enum AVPictureType类型（ffmpeg的libavutil/frame.h中定义）
// 返回值：>= 0为成功，其它为失败。
FFAVCODEC_API int lfrtcEncodeVideo(void *codecCtx, char *inBuf[3], int inLinesize[3], char *outBuf[3], int *outLen[3], bool *keyframe);
FFAVCODEC_API int lfrtcDecodeVideo(void *codecCtx, char *inBuf[3], int inLen[3], char *outBuf[3], int outLen[3], int *frameType);
FFAVCODEC_API int lfrtcEncodeAudio(void *codecCtx, char *inBuf[3], int inLen[3], char *outBuf[3], int *outLen[3]);
FFAVCODEC_API int lfrtcDecodeAudio(void *codecCtx, char *inBuf[3], int inLen[3], char *outBuf[3], int *outLen[3]);
#endif

class lfRtcAudioEncoderInternal;
class lfRtcAudioEncoder {
public:
  lfRtcAudioEncoder();
  ~lfRtcAudioEncoder();
  int Open(const lfrtcAudioEncodeParams *params);
  int GetFrameSamplesPerChannel();
  void Close();

  int Process(unsigned char *data[3], int len[3]);
  void* GetEncodeData();
  int GetEncodeLen();

private:
  lfRtcAudioEncoderInternal *m_internal;
};

class lfRtcAudioDecoderInternal;
class lfRtcAudioDecoder {
public:
  lfRtcAudioDecoder();
  ~lfRtcAudioDecoder();
  int Open(const lfrtcAudioDecodeParams *params);
  void Close();

  int Process(unsigned char *data, int len);
  void* GetDecodeData(int plain_index = 0);
  int GetDecodeLen(int plain_index = 0);

private:
  lfRtcAudioDecoderInternal *m_internal;
};

class lfRtcVideoEncoderInternal;
class lfRtcVideoEncoder {
public:
  lfRtcVideoEncoder();
  ~lfRtcVideoEncoder();
  int Open(const lfrtcVideoEncodeParams *params);
  void Close();

  int Process(unsigned char *data[3], int linesize[3], bool *keyframe = 0);
  void* GetEncodeData();
  int GetEncodeLen();

private:
  lfRtcVideoEncoderInternal *m_internal;
};

class lfRtcVideoDecoderInternal;
// 1. 约定
//    解码器默认解码H264的数据，解码后输出数据固定为AV_PIX_FMT_YUV420P
// 2. 依赖参数
//    无需传入其它参数，即可让解码器开始工作
// 3. 返回解码后数据
//    为了防止解码后的数据被额外拷贝，解码后数据的buf由调用者维护(即Process()中传入的outBuf）
//    解码成功后，会按照AV_PIX_FMT_YUV420P格式（linesize=width）将数据写到outBuf。
//    所以传入的width和height，就标识了outBuf的大小。
// 4. width和height
//    解码器知道分辨率是多少，而调用者并不知道，所以本类提供了分辨率的获取接口，
//    Process()中传入的width和height，标识了outBuf的大小，但有可能与实际情况不符。
//    例如分辨率变化，此时Process()<0，但其内部仍然会解码，并把解码后的数据放到内部buf中，
//    因此当Process()=VIDEO_DECODE_SUCCESS_ON_SIZE时，调用者应更新width、height和outBuf
//    并从GetExtraDecodeData()获取数据
class lfRtcVideoDecoder {
public:
  lfRtcVideoDecoder();
  ~lfRtcVideoDecoder();
  int Open(const lfrtcVideoDecodeParams *params);
  void Close();

  enum {
    VIDEO_DECODE_FAILED = -1,
    VIDEO_DECODE_SUCCESS = 0,
    VIDEO_DECODE_SUCCESS_NO_PICTRUE,
    VIDEO_DECODE_SUCCESS_ON_SIZE,
  };
  // 返回值，<0表示解码失败；>=0表示解码成功；=0表示解码成功，且outBuf被写入了数据
  int Process(unsigned char *inBuf, int inLen, unsigned char *outBuf[3], int width, int height);
  int GetWidth();
  int GetHeight();
  int GetFrameType();
  void* GetExtraDecodeData(int plain_index);

private:
  lfRtcVideoDecoderInternal *m_internal;
};

#ifdef  __cplusplus
}
#endif

#endif
