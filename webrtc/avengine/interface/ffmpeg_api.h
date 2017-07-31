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
  // ���������Ǳ���������PCM���ݵĲ���
  // int sample_fmt; // ֻ����AV_SAMPLE_FMT_S16��ʽ
  int channel_count; // ����1��2ʱ����ʾAV_CH_LAYOUT_MONO/AV_CH_LAYOUT_STEREO��ʽ
  int sample_rate;

  // ���²����ǶԱ�����������
  int bitrate;
  int codec_id;  // �μ�ffmpeg��enum AVCodecID�� ����0ʱ�����Ĭ����AAC����
};

struct lfrtcAudioDecodeParams {
  // ��������������ҵ�������õ���PCM���ݸ�ʽ������������������ʽ������֮���������resample�������
  // int sample_fmt; // ֻ����AV_SAMPLE_FMT_S16��ʽ
  int channel_count; // ����1��2ʱ����ʾAV_CH_LAYOUT_MONO/AV_CH_LAYOUT_STEREO��ʽ
  int sample_rate;

  int codec_id;  // �μ�ffmpeg��enum AVCodecID�� ����0ʱ�����Ĭ����AAC����
};

struct lfrtcVideoEncodeParams {
  // �����ĸ��Ǳ���������ԭʼ���ݵĲ���
  // int data_format; // ֻ����AV_PIX_FMT_YUV420P��ʽ
  int width;
  int height;
  int fps;

  int bitrate;
  int gop_size;
  int codec_id;  // �μ�ffmpeg��enum AVCodecID�� ����0ʱ�����Ĭ����H264����
  char preset[32];
};

struct lfrtcVideoDecodeParams {
  // int data_format; // ֻ����AV_PIX_FMT_YUV420P��ʽ

  int codec_id;  // �μ�ffmpeg��enum AVCodecID�� ����0ʱ�����Ĭ����H264����
};

#if 0
// nb_sample��������֧�ֵ�֡��С
FFAVCODEC_API void *lfrtcCreateAudioEncoder(const lfrtcAudioEncodeParams *params, int *nb_sample);
FFAVCODEC_API void *lfrtcCreateAudioDecoder(const lfrtcAudioDecodeParams *params);
FFAVCODEC_API void *lfrtcCreateVideoEncoder(const lfrtcVideoEncodeParams *params);
FFAVCODEC_API void *lfrtcCreateVideoDecoder(const lfrtcVideoDecodeParams *params);

FFAVCODEC_API void lfrtcFFDeleteCodec(void *codecCtx);

// inBuf, inlen, outBuf, outLen: ��plane���ݣ����ֻ��һ��plane����ֻʹ������ĵ�1��Ԫ�أ����������ֵΪ0
// outLen: Ϊ�����������������ʱ��ʾbuf�����������ʱ��ʾʵ�����ݵĳ���
// keyframe: Ϊ�����������������ʱkeyframe = true��ʾǿ��IDR֡���룬���ʱ��ʾ��ǰ��ʵ������
// frameType: Ϊ���������ʵ����ӦΪenum AVPictureType���ͣ�ffmpeg��libavutil/frame.h�ж��壩
// ����ֵ��>= 0Ϊ�ɹ�������Ϊʧ�ܡ�
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
// 1. Լ��
//    ������Ĭ�Ͻ���H264�����ݣ������������ݹ̶�ΪAV_PIX_FMT_YUV420P
// 2. ��������
//    ���贫�����������������ý�������ʼ����
// 3. ���ؽ��������
//    Ϊ�˷�ֹ���������ݱ����⿽������������ݵ�buf�ɵ�����ά��(��Process()�д����outBuf��
//    ����ɹ��󣬻ᰴ��AV_PIX_FMT_YUV420P��ʽ��linesize=width��������д��outBuf��
//    ���Դ����width��height���ͱ�ʶ��outBuf�Ĵ�С��
// 4. width��height
//    ������֪���ֱ����Ƕ��٣��������߲���֪�������Ա����ṩ�˷ֱ��ʵĻ�ȡ�ӿڣ�
//    Process()�д����width��height����ʶ��outBuf�Ĵ�С�����п�����ʵ�����������
//    ����ֱ��ʱ仯����ʱProcess()<0�������ڲ���Ȼ����룬���ѽ��������ݷŵ��ڲ�buf�У�
//    ��˵�Process()=VIDEO_DECODE_SUCCESS_ON_SIZEʱ��������Ӧ����width��height��outBuf
//    ����GetExtraDecodeData()��ȡ����
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
  // ����ֵ��<0��ʾ����ʧ�ܣ�>=0��ʾ����ɹ���=0��ʾ����ɹ�����outBuf��д��������
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
