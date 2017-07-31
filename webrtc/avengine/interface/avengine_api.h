// edit by zhangle
#ifndef AVENGINE_SOURCE_AVENGINE_API_H_
#define AVENGINE_SOURCE_AVENGINE_API_H_

#include "avengine_types.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define AVENG_API __declspec(dllexport)
#else
#define AVENG_API __attribute__ ((__visibility__("default")))
#endif

enum lfrtcNofityMessageId {
  LFRTC_VIDEO_DECODE_SUCCESS = 1200,  // wParam: last_seq_num
  LFRTC_VIDEO_DECODE_FAILED,          // wParam: last_seq_num
  LFRTC_VIDEO_DECODE_FFMPEG_ERROR,
  LFRTC_VIDEO_GET_RESOLUTION,         // wParam: width, lParam: height
  LFRTC_VIDEO_GET_PLAY_DELAY_TIME,
  LFRTC_VIDEO_GET_IDR_FRAME_SIZE,
  LFRTC_AUDIO_DECODE_SUCCESS,
  LFRTC_AUDIO_DECODE_FAILED,
  LFRTC_VIDEO_FIRST_FRAME,
};

typedef void(*T_lfrtcNotifyCb)(void *playerCtx, unsigned int msgid, int wParam, int lParam);
typedef void(*T_lfrtcReportCb)(void *playerCtx, const char *msg);
// level: LogAvengineLevel
typedef void(*T_lfrtcLogCb)(int level, const char *file, int line_num, const char *format, va_list args);
typedef void(*T_lfrtcEncodedVideoCb)(void *captureCtx, char* data, int len, unsigned int timestamp);
typedef void(*T_lfrtcEncodedAudioCb)(void *captureCtx, char* data, int len, unsigned int timestamp);

enum LogAvengineLevel {
  AVENGINE_LOG_LEVEL_TRC = 1,
  AVENGINE_LOG_LEVEL_RTP = 2,
  AVENGINE_LOG_LEVEL_RTCP = 3,
  AVENGINE_LOG_LEVEL_RECOVER = 4,
  AVENGINE_LOG_LEVEL_DBG = 5,
  AVENGINE_LOG_LEVEL_INF = 6,
  AVENGINE_LOG_LEVEL_WRN = 7,
  AVENGINE_LOG_LEVEL_ERR = 8,
  AVENGINE_LOG_LEVEL_NON = 9,
};

typedef void* lfrtcAVID;

#if defined(__ANDROID__)
AVENG_API int IISetAndroidObj(void* jvm, void* context);
AVENG_API int IIClearAndroidObj();
#endif

AVENG_API void lfrtcInit();
AVENG_API void lfrtcUninit();
AVENG_API int lfrtcStartPlay(void* userdata, lfrtcAVID &audioid, lfrtcAVID &videoid);
AVENG_API int lfrtcSetPlayWindow(void *window, lfrtcAVID videoid);
AVENG_API int lfrtcSetAudioPlayoutDevice(const char *audioszDeviceID);
AVENG_API int lfrtcStopPlay(lfrtcAVID audioid, lfrtcAVID videoid);
AVENG_API int lfrtcSetRtpData(void *data, int len, lfrtcAVID avid);
AVENG_API int lfrtcSetVideoRtcp(const void *data, int len, lfrtcAVID videoid);
AVENG_API int lfrtcSetAudioRtcp(const void *data, int len, lfrtcAVID audioid);
AVENG_API int lfrtcSetVideoSSRC(unsigned int SSRC, lfrtcAVID videoid);
AVENG_API int lfrtcSetAudioSSRC(unsigned int SSRC, lfrtcAVID audioid);
AVENG_API void lfrtcSetControlParams(lfrtcGlobalConfig *config);
AVENG_API void lfrtcRegisterNotifyCb(T_lfrtcNotifyCb callback);
AVENG_API void lfrtcRegisterVideoRawdataCb(T_lfrtcDecodedVideoCb callback);
AVENG_API void lfrtcRegisterAudioRawdataCb(T_lfrtcDecodedAudioCb callback);
AVENG_API void lfrtcRegistReportFunc(T_lfrtcReportCb callback);
AVENG_API void lfrtcRegisterLogFunc(T_lfrtcLogCb callback);

AVENG_API int lfrtcStartCapture(const lfrtcCaptureConfig *config, lfrtcAVID &captureid);
AVENG_API int lfrtcStartPreview(void *window, lfrtcAVID captureid);
AVENG_API int lfrtcStartPreview2(lfrtcAVID captureid, T_lfrtcPreviewVideoCb callback, lfrtcRawVideoType type, void *userdata);
AVENG_API int lfrtcStartEncodeAndSend(lfrtcAVID captureid, const lfrtcEncodeConfig* config, lfrtcAVID &audioid, lfrtcAVID &videoid, void* userdata);
AVENG_API void lfrtcStopEncodeAndSend(lfrtcAVID audioid, lfrtcAVID videoid);
AVENG_API void lfrtcStopCapture(lfrtcAVID captureid);
AVENG_API void lfrtcStopPreview(lfrtcAVID captureid);
AVENG_API void lfrtcRegisterAudioEncodeCb(T_lfrtcEncodedAudioCb callback);
AVENG_API void lfrtcRegisterVideoEncodeCb(T_lfrtcEncodedVideoCb callback);
AVENG_API void lfrtcGetVideoEncodeInfo(lfrtcAVID videoid, unsigned int &fps, unsigned int &bitrate);
AVENG_API unsigned int lfrtcGetVideoTargetEncodeBitrate(lfrtcAVID videoid);

AVENG_API int lfrtcEnableNoiseSuppression(bool enable);
AVENG_API int lfrtcEnableAEC(bool enable);

// return: device count
AVENG_API int lfrtcGetAudioRecorderDevice(lfrtcDevice* buf, int count);
AVENG_API int lfrtcGetAudioPlayoutDevice(lfrtcDevice* buf, int count);
AVENG_API int lfrtcGetCameraDevice(lfrtcDevice* buf, int count);
AVENG_API int lfrtcGetCameraCapability(const char* deviceid, lfrtcCameraCapability* buf, int count);

AVENG_API int lfrtcSaveFrameToJPEG(unsigned char** yuvbuf, int width, int height, const char *dumpPath);

#ifdef  __cplusplus    
}
#endif 

#endif
