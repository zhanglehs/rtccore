// edit by zhangle
#include "webrtc/avengine/interface/avengine_api.h"

#include "webrtc/base/basetype.h"
#include "webrtc/modules/video_coding/main/source/media_optimization.h"
#include "webrtc/system_wrappers/interface/clock.h"
#if defined(__ANDROID__)
#include <jni.h>
#include "webrtc/modules/video_capture/video_capture_internal.h"
#include "webrtc/modules/video_render/video_render_internal.h"
#include "webrtc/voice_engine/include/voe_base.h"
#endif

#include "webrtc/avengine/interface/ffmpeg_api.h"
#include "webrtc/avengine/source/avengine_internal.h"
#include "webrtc/avengine/source/avengine_util.h"

namespace {
RtcEngine *g_avInst = NULL;
}

#if defined(__ANDROID__)
int IISetAndroidObj(void* jvm, void* context) {
#ifndef WEBRTC_ANDROID_OPENSLES
  webrtc::VoiceEngine::SetAndroidObjects(jvm, context);
#endif
  webrtc::SetCaptureAndroidVM((JavaVM *)jvm, static_cast<jobject>(context));
  webrtc::SetRenderAndroidVM((JavaVM *)jvm);
  return 0;
}

int IIClearAndroidObj() {
#ifndef WEBRTC_ANDROID_OPENSLES
  webrtc::VoiceEngine::SetAndroidObjects(NULL, NULL);
#endif
  webrtc::SetRenderAndroidVM(NULL);
  return 0;
}
#endif

void lfrtcRegisterNotifyCb(T_lfrtcNotifyCb callback) {
  g_NotifyCallback = callback;
}

void lfrtcInit() {
  if (!g_avInst) {
    g_avInst = new RtcEngine();
  }
}

void lfrtcUninit() {
  if (g_avInst) {
    g_avInst->Release();
    delete g_avInst;
    g_avInst = NULL;
  }
}

int lfrtcSetRtpData(void *data2, int len, lfrtcAVID avid) {
  if (NULL == data2 || len <= 0) {
    return -1;
  }
  int ret = -1;

  char *data = (char *)data2;
  unsigned char payloadType = data[1] & 0x7f;

  if (payloadType == 96) {
    // zhangle, the following code just like a fool
    // double the RTP_FIXED_HEADER
    unsigned char new_data[2000];
    if (len + sizeof(RTP_FIXED_HEADER)+1 > sizeof(new_data)) {
      AVENGINE_WRN("rtp packet to large, len=%d", len);
      return -1;
    }

    memcpy(new_data, data, sizeof(RTP_FIXED_HEADER));
    if (((RTP_FIXED_HEADER *)new_data)->extension == 0) {
      return -1;
    }
    EXTEND_HEADER *extendHeader = (EXTEND_HEADER *)(data + sizeof(RTP_FIXED_HEADER)); // byte alignment bug
    ((RTP_FIXED_HEADER *)new_data)->extension = 0;
    new_data[sizeof(RTP_FIXED_HEADER)] = extendHeader->first_packet * 2 + extendHeader->keyframe;
    memcpy(new_data + sizeof(RTP_FIXED_HEADER)+1, data, len);
    ret = g_avInst->SetVideoRtp(new_data, len + sizeof(RTP_FIXED_HEADER)+1, avid);
  }
  else if (payloadType == 97) {
    ret = g_avInst->SetAudioRtp(data, len, avid);
  }

  return ret;
}

int lfrtcSetVideoRtcp(const void *data, int len, lfrtcAVID videoid) {
  if (!data) {
    return -1;
  }

  return g_avInst->SetVideoRtcp(data, len, videoid);
}

int lfrtcSetAudioRtcp(const void *data, int len, lfrtcAVID audioid) {
  if (!data) {
    return -1;
  }

  return g_avInst->SetAudioRtcp(data, len, audioid);
}

int lfrtcStartPlay(void* userdata, lfrtcAVID &audioid, lfrtcAVID &videoid) {
  return g_avInst->StartPlay(userdata, audioid, videoid);
}

int lfrtcSetAudioPlayoutDevice(const char *audioszDeviceID) {
  return g_avInst->SetAudioPlayoutDevice(audioszDeviceID);
}

int lfrtcSetPlayWindow(void *window, lfrtcAVID videoid) {
  return g_avInst->SetPlayWindow(videoid, window);
}

int lfrtcStopPlay(lfrtcAVID audioid, lfrtcAVID videoid) {
  return g_avInst->StopPlay(audioid, videoid);
}

void lfrtcRegistReportFunc(T_lfrtcReportCb sendreport) {
  g_ReportCallback = sendreport;
}

void lfrtcSetControlParams(lfrtcGlobalConfig *config) {
  g_global_config = config;
  AVENGINE_INF("is_lostpacketStrategy %d IlostpacketToScreen %d PlostpacketToScreen %d is_yuvDump %d",
    g_global_config->is_lostpacketStrategy, g_global_config->IlostpacketToScreen,
    g_global_config->PlostpacketToScreen, g_global_config->is_yuvDump);
}

void lfrtcRegisterLogFunc(T_lfrtcLogCb callback) {
  g_LogCallback = callback;
}

void lfrtcRegisterVideoRawdataCb(T_lfrtcDecodedVideoCb callback) {
  g_DecodedVideoCallback = callback;
}

void lfrtcRegisterAudioRawdataCb(T_lfrtcDecodedAudioCb callback) {
  g_DecodedAudioCallback = callback;
}

int lfrtcStartCapture(const lfrtcCaptureConfig *config, lfrtcAVID &captureid) {
  return g_avInst->StartCapture(config, captureid);
}

int lfrtcStartPreview(void *window, lfrtcAVID captureid) {
  return g_avInst->StartPreview(window, captureid);
}

int lfrtcStartEncodeAndSend(lfrtcAVID captureid, const lfrtcEncodeConfig* config, lfrtcAVID &audioid, lfrtcAVID &videoid, void* userdata) {
  return g_avInst->StartEncodeAndSend(captureid, config, audioid, videoid, userdata);
}

void lfrtcStopEncodeAndSend(lfrtcAVID audioid, lfrtcAVID videoid) {
  g_avInst->StopEncodeAndSend(audioid, videoid);
}

void lfrtcStopCapture(lfrtcAVID captureid) {
  g_avInst->StopCapture(captureid);
}

void lfrtcStopPreview(lfrtcAVID captureid) {
  g_avInst->StopPreview(captureid);
}

int lfrtcGetAudioRecorderDevice(lfrtcDevice* buf, int count) {
  return g_avInst->GetAudioRecorderDevice(buf, count);
}

int lfrtcGetAudioPlayoutDevice(lfrtcDevice* buf, int count) {
  return g_avInst->GetAudioPlayoutDevice(buf, count);
}

int lfrtcGetCameraDevice(lfrtcDevice* buf, int count) {
  return g_avInst->GetCameraDevice(buf, count);
}

void lfrtcRegisterAudioEncodeCb(T_lfrtcEncodedAudioCb callback) {
  g_EncodedAudioCallback = callback;
}

void lfrtcRegisterVideoEncodeCb(T_lfrtcEncodedVideoCb callback) {
  g_EncodedVideoCallback = callback;
}

int lfrtcStartPreview2(lfrtcAVID captureid, T_lfrtcPreviewVideoCb callback, lfrtcRawVideoType type, void *userdata) {
  return g_avInst->StartPreview(captureid, callback, type, userdata);
}

int lfrtcGetCameraCapability(const char* deviceid, lfrtcCameraCapability* buf, int count) {
  return g_avInst->GetCameraCapability(deviceid, buf, count);
}

void lfrtcGetVideoEncodeInfo(lfrtcAVID videoid, unsigned int &fps, unsigned int &bitrate) {
  g_avInst->GetVideoEncodeInfo(videoid, fps, bitrate);
}

int lfrtcSetVideoSSRC(unsigned int SSRC, lfrtcAVID videoid) {
  return g_avInst->SetVideoSSRC(SSRC, videoid);
}

int lfrtcSetAudioSSRC(unsigned int SSRC, lfrtcAVID audioid) {
  return g_avInst->SetAudioSSRC(SSRC, audioid);
}

unsigned int lfrtcGetVideoTargetEncodeBitrate(lfrtcAVID videoid) {
  return g_avInst->GetVideoSenderObservedBitrate(videoid);
}

int lfrtcEnableNoiseSuppression(bool enable) {
  return g_avInst->EnableNoiseSuppression(enable);
}

int lfrtcEnableAEC(bool enable) {
  return g_avInst->EnableAEC(enable);
}
