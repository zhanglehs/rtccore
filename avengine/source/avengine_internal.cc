// edit by zhangle
#include "webrtc/avengine/source/avengine_internal.h"

#include "webrtc/base/json.h"
#include "webrtc/modules/video_coding/codecs/h264/main/interface/h264.h"
#include "webrtc/video_engine/include/vie_base.h"
#include "webrtc/video_engine/include/vie_capture.h"
#include "webrtc/video_engine/include/vie_codec.h"
#include "webrtc/video_engine/include/vie_external_codec.h"
#include "webrtc/video_engine/include/vie_network.h"
#include "webrtc/video_engine/include/vie_render.h"
#include "webrtc/video_engine/include/vie_rtp_rtcp.h"
#include "webrtc/voice_engine/include/voe_audio_processing.h"
#include "webrtc/voice_engine/include/voe_base.h"
#include "webrtc/voice_engine/include/voe_codec.h"
#include "webrtc/voice_engine/include/voe_hardware.h"
#include "webrtc/voice_engine/include/voe_network.h"
#include "webrtc/voice_engine/include/voe_rtp_rtcp.h"
#include "webrtc/voice_engine/include/voe_volume_control.h"

#include "webrtc/avengine/interface/avengine_api.h"
#include "webrtc/avengine/source/avengine_util.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define RTP_PAYLOAD_TYPE_H264 96

namespace {

class CustomExternalTransport;
class CustomExternalRenderer;
class CustomEncoderObserver;

class AVID {
public:
  int m_id;
  unsigned int m_encode_fps;
  unsigned int m_encode_bitrate;
  CustomExternalTransport *m_transport;
  CustomExternalRenderer *m_render;
  CustomEncoderObserver *m_encoder_observer;
  H264Decoder *m_decoder;
  H264Encoder *m_encoder;

  AVID(int id) {
    m_id = id;
    m_encode_fps = 0;
    m_encode_bitrate = 0;
    m_transport = NULL;
    m_render = NULL;
    m_encoder_observer = NULL;
    m_decoder = NULL;
    m_encoder = NULL;
  }

  ~AVID();
};

class CustomExternalTransport : public webrtc::Transport {
public:
  CustomExternalTransport(AVID *avid) : m_avid(avid) {}
  virtual int SendPacket(int channel, const void* data, size_t len) { return 0; }
  virtual int SendRTCPPacket(int channel, const void* data, size_t len) { return 0; }

private:
  AVID *m_avid;
};

class CustomExternalRenderer : public webrtc::ExternalRenderer {
public:
  CustomExternalRenderer(T_lfrtcPreviewVideoCb callback, lfrtcRawVideoType type, void *object) {
    m_callback = callback;
    m_object = object;
    m_type = type;
    m_width = 0;
    m_height = 0;
  }

  virtual int FrameSizeChange(unsigned int width,
    unsigned int height,
    unsigned int number_of_streams) {
    //int capture_id = number_of_streams;
    m_width = width;
    m_height = height;
    return 0;
  }

  virtual int DeliverFrame(unsigned char* buffer,
    size_t buffer_size,
    uint32_t timestamp,
    int64_t ntp_time_ms,
    int64_t render_time_ms,
    void* handle) {
    if (m_callback) {
      char *data[3] = { (char*)buffer, NULL, NULL };
      m_callback(m_object, data, m_type, m_width, m_height);
    }
    return 0;
  }

  virtual int DeliverI420Frame(const webrtc::I420VideoFrame& webrtc_frame) {
    if (webrtc_frame.IsZeroSize()) {
      return 0;
    }

    if (m_callback) {
      char* yuvbuf[3];
      yuvbuf[0] = (char*)webrtc_frame.buffer(webrtc::kYPlane);
      yuvbuf[1] = (char*)webrtc_frame.buffer(webrtc::kUPlane);
      yuvbuf[2] = (char*)webrtc_frame.buffer(webrtc::kVPlane);
      m_callback(m_object, yuvbuf, klfrtcVideoI420, webrtc_frame.width(), webrtc_frame.height());
    }
    return 0;
  }

  virtual bool IsTextureSupported() {
    return false;
  }

private:
  T_lfrtcPreviewVideoCb m_callback;
  void *m_object;
  lfrtcRawVideoType m_type;
  int m_width;
  int m_height;
};

class CustomEncoderObserver: public webrtc::ViEEncoderObserver {
public:
  CustomEncoderObserver(AVID *avid) : m_avid(avid) {}

  virtual void OutgoingRate(const int video_channel,
    const unsigned int framerate,
    const unsigned int bitrate) {
    m_avid->m_encode_fps = framerate;
    m_avid->m_encode_bitrate = bitrate;
  }

  virtual void SuspendChange(int video_channel, bool is_suspended) {
  }

private:
  AVID *m_avid;
};

AVID::~AVID() {
  delete m_transport;
  delete m_render;
  delete m_encoder_observer;
  delete m_decoder;
  delete m_encoder;
}

int CopyDevices(const std::vector<lfrtcDevice> &devices, lfrtcDevice* buf, int count) {
  if (count <= 0) {
    return 0;
  }
  memset(buf, 0, count * sizeof(lfrtcDevice));
  if (devices.size() <= 0) {
    return 0;
  }
  if (count > (int)devices.size()) {
    count = (int)devices.size();
  }
  memcpy(buf, &devices[0], count * sizeof(lfrtcDevice));
  return count;
}

}

int RtcEngine::InitAudioPlayoutDeviceList() {
  int deviceCount = 0;
#ifndef __ANDROID__
  m_voeHardware->GetNumOfPlayoutDevices(deviceCount);
  lfrtcDevice item;
  char deviceName[sizeof(item.szDeviceName)];
  char deviceId[sizeof(item.szDeviceID)];
  m_AudioPlayoutDevices.clear();
  for (int i = 0; i < deviceCount; i++) {
    if (0 == m_voeHardware->GetPlayoutDeviceName(i, deviceName, deviceId)){
      strcpy(item.szDeviceName, deviceName);
      strcpy(item.szDeviceID, deviceId);
      m_AudioPlayoutDevices.push_back(item);
    }
  }
#endif
  return deviceCount;
}

int RtcEngine::InitCameraDeviceList() {
  int deviceCount = m_vieCapture->NumberOfCaptureDevices();
  lfrtcDevice item;
  char deviceName[sizeof(item.szDeviceName)];
  char deviceId[sizeof(item.szDeviceID)];
  m_CameraDevices.clear();
  for (int i = 0; i < deviceCount; i++) {
    if (0 == m_vieCapture->GetCaptureDevice(i, deviceName, sizeof(deviceName)-1, deviceId, sizeof(deviceId)-1)) {
      strcpy(item.szDeviceName, deviceName);
      strcpy(item.szDeviceID, deviceId);
      m_CameraDevices.push_back(item);
    }
  }
  return deviceCount;
}

int RtcEngine::InitAudioRecorderDeviceList() {
  int deviceCount = 0;
#ifndef __ANDROID__
  m_voeHardware->GetNumOfRecordingDevices(deviceCount);
  lfrtcDevice item;
  char deviceName[sizeof(item.szDeviceName)];
  char deviceId[sizeof(item.szDeviceID)];
  m_AudioRecorderDevices.clear();
  for (int i = 0; i < deviceCount; i++) {
    if (0 == m_voeHardware->GetRecordingDeviceName(i, deviceName, deviceId)) {
      strcpy(item.szDeviceName, deviceName);
      strcpy(item.szDeviceID, deviceId);
      m_AudioRecorderDevices.push_back(item);
    }
  }
#endif
  return deviceCount;
}

int RtcEngine::StartCapture(const lfrtcCaptureConfig *config, lfrtcAVID &captureid) {
  captureid = NULL;
  if (config == NULL) {
    return -1;
  }
  InitCameraDeviceList();
  InitAudioRecorderDeviceList();
  if (m_CameraDevices.size() <= 0
#ifndef __ANDROID__
    || m_AudioRecorderDevices.size() <= 0
#endif
    ) {
    return -1;
  }

  const char *videoDeviceID = m_CameraDevices[0].szDeviceID;
  for (auto it = m_CameraDevices.begin(); it != m_CameraDevices.end(); it++) {
    if (0 == strcmp(config->video_deviceid, it->szDeviceID)) {
      videoDeviceID = config->video_deviceid;
      break;
    }
  }
  int id = -1;
  if (m_vieCapture->AllocateCaptureDevice(videoDeviceID, strlen(videoDeviceID), id) < 0) {
    return -1;
  }
  captureid = new AVID(id);

  webrtc::CaptureCapability captureCap;
  captureCap.width = config->video_capture_width;
  captureCap.height = config->video_capture_height;
  captureCap.maxFPS = 30;
  m_vieCapture->StartCapture(id, captureCap);

#ifndef __ANDROID__
  int audio_index = -1;
  for (int i = 0; i < (int)m_AudioRecorderDevices.size(); i++) {
    if (0 == strcmp(config->audio_deviceid, m_AudioRecorderDevices[i].szDeviceID)) {
      audio_index = i;
    }
  }
  m_voeHardware->SetRecordingDevice(audio_index);
#endif
  assert(config->audio_frequence == 48000);
  m_voeHardware->SetRecordingSampleRate(config->audio_frequence);
  return 0;
}

int RtcEngine::StartPreview(void *window, lfrtcAVID captureid) {
  int id = -1;
  if (captureid) {
    id = ((AVID*)captureid)->m_id;
  }
  if (id < 0) {
    return -1;
  }
  m_vieRender->RemoveRenderer(id);
  m_vieRender->AddRenderer(id, window, 0, 0.0f, 0.0f, 1.0f, 1.0f);
  m_vieRender->StartRender(id);
  return 0;
}

int RtcEngine::StartPreview(lfrtcAVID captureid, T_lfrtcPreviewVideoCb callback, lfrtcRawVideoType type, void *userdata) {
  if (captureid == NULL) {
    return -1;
  }
  int id = ((AVID*)captureid)->m_id;
  if (id < 0) {
    return -1;
  }
  m_vieRender->RemoveRenderer(id);
  delete ((AVID*)captureid)->m_render;
  ((AVID*)captureid)->m_render = NULL;
  CustomExternalRenderer *render = new CustomExternalRenderer(callback, type, userdata);
  ((AVID*)captureid)->m_render = render;
  webrtc::RawVideoType webrtc_type = webrtc::kVideoI420;
  switch (type) {
  case klfrtcVideoARGB:
    webrtc_type = webrtc::kVideoARGB;
    break;
  case klfrtcVideoRGB24:
    webrtc_type = webrtc::kVideoRGB24;
    break;
  default:
    break;
  }
  m_vieRender->AddRenderer(id, webrtc_type, render);
  m_vieRender->StartRender(id);
  return 0;
}

int RtcEngine::StartEncodeAndSend(lfrtcAVID captureid, const lfrtcEncodeConfig* config, lfrtcAVID &audioid, lfrtcAVID &videoid, void* userdata) {
  audioid = NULL;
  videoid = NULL;
  if (captureid == NULL) {
    return -1;
  }
  int id = ((AVID*)captureid)->m_id;
  if (id < 0) {
    return -1;
  }

  int aChannelID = -1;
  int vChannelID = -1;
  m_vieBase->CreateChannel(vChannelID);
  aChannelID = m_voeBase->CreateChannel();
  if (vChannelID < 0 || aChannelID < 0 || m_vieCapture->ConnectCaptureDevice(id, vChannelID) < 0) {
    m_voeBase->DeleteChannel(aChannelID);
    m_vieBase->DeleteChannel(vChannelID);
    return -1;
  }

  {
    videoid = new AVID(vChannelID);
    H264Encoder *encoder = new H264Encoder();
    ((AVID*)videoid)->m_encoder = encoder;
    Json::Value arrayvalue;
    arrayvalue["in_data_format"] = 0;
    arrayvalue["width"] = config->video_encode_width;
    arrayvalue["height"] = config->video_encode_height;
    arrayvalue["gop_size"] = config->video_gop;
    arrayvalue["mtu_size"] = config->video_mtu_size;
    arrayvalue["frame_rate"] = config->video_max_fps;
    arrayvalue["video_bits_rate"] = config->video_bitrate;
    Json::Value jRoot;
    jRoot["codecParams"] = arrayvalue;
    Json::FastWriter writer;
    std::string EncodeParams = writer.write(jRoot);
    strcpy(encoder->_EncParams, EncodeParams.c_str());

    m_vieExtCodec->RegisterExternalSendCodec(vChannelID,
      RTP_PAYLOAD_TYPE_H264, encoder, false);
    webrtc::VideoCodec codec;
    int count = m_vieCodec->NumberOfCodecs();
    for (int i = 0; i < count; i++) {
      m_vieCodec->GetCodec(i, codec);
      if (0 == strcmp(codec.plName, "H264")) {
        break;
      }
    }
    codec.plType = RTP_PAYLOAD_TYPE_H264;
    codec.cb = (void*)g_EncodedVideoCallback;
    codec.object = userdata;
    codec.chanId = vChannelID;
    codec.width = config->video_encode_width;
    codec.height = config->video_encode_height;
    codec.maxFramerate = config->video_max_fps;
    codec.startBitrate = config->video_bitrate / 1024;
    codec.maxBitrate = config->video_bitrate / 1024;
    codec.minBitrate = config->video_bitrate / 1024 / 10;
    codec.targetBitrate = config->video_bitrate / 1024 + 1; // +1 for test
    m_vieCodec->SetSendCodec(vChannelID, codec);
    CustomEncoderObserver *encoder_observer = new CustomEncoderObserver((AVID*)videoid);
    ((AVID*)videoid)->m_encoder_observer = encoder_observer;
    m_vieCodec->RegisterEncoderObserver(vChannelID, *encoder_observer);
    CustomExternalTransport *transport = new CustomExternalTransport((AVID*)videoid);
    ((AVID*)videoid)->m_transport = transport;
    m_vieNetwork->RegisterSendTransport(vChannelID, *transport);
    m_vieBase->StartSend(vChannelID);
  }

  {
    audioid = new AVID(aChannelID);
    webrtc::CodecInst codec;
    int count = m_voeCodec->NumOfCodecs();
    for (int i = 0; i < count; i++) {
      m_voeCodec->GetCodec(i, codec);
      if (0 == strcmp(codec.plname, "aac1")) {
        break;
      }
    }
    const char* EncodeParams = "{\"codecParams\":{\"audio_bits_rate\":32000,\"frame_size\":2048,\"in_channel_count\":2,\"in_sample_rate\":48000,\"in_sample_fmt\":1}}";
    codec.chanId = aChannelID;
    codec.cb = (void*)g_EncodedAudioCallback;
    codec.object = userdata;
    strcpy(codec.params, EncodeParams);
    m_voeCodec->SetSendCodec(aChannelID, codec);
    CustomExternalTransport *transport = new CustomExternalTransport((AVID*)audioid);
    ((AVID*)audioid)->m_transport = transport;
    m_voeNetwork->RegisterExternalTransport(aChannelID, *transport);
    m_voeBase->StartSend(aChannelID);
  }
  return 0;
}

void RtcEngine::StopEncodeAndSend(lfrtcAVID audioid, lfrtcAVID videoid) {
  int aChannelID = -1;
  int vChannelID = -1;
  if (audioid) {
    aChannelID = ((AVID*)audioid)->m_id;
  }
  if (videoid) {
    vChannelID = ((AVID*)videoid)->m_id;
  }
  if (aChannelID >= 0) {
    m_voeNetwork->DeRegisterExternalTransport(aChannelID);
    m_voeBase->StopSend(aChannelID);
    m_voeBase->DeleteChannel(aChannelID);
  }
  if (vChannelID >= 0) {
    m_vieCapture->DisconnectCaptureDevice(vChannelID);
    m_vieNetwork->DeregisterSendTransport(vChannelID);
    m_vieBase->StopSend(vChannelID);
    m_vieBase->DeleteChannel(vChannelID);
  }
  delete (AVID*)audioid;
  delete (AVID*)videoid;
}

void RtcEngine::StopCapture(lfrtcAVID captureid) {
  StopPreview(captureid);
  if (captureid == NULL) {
    return;
  }
  int id = ((AVID*)captureid)->m_id;
  m_vieCapture->StopCapture(id);
  m_vieCapture->ReleaseCaptureDevice(id);
  delete (AVID*)captureid;
}

void RtcEngine::StopPreview(lfrtcAVID captureid) {
  if (captureid == NULL) {
    return;
  }
  int id = ((AVID*)captureid)->m_id;
  m_vieRender->StopRender(id);
  m_vieRender->RemoveRenderer(id);
}

void RtcEngine::GetVideoEncodeInfo(lfrtcAVID videoid, unsigned int &fps, unsigned int &bitrate) {
  if (videoid) {
    fps = ((AVID*)videoid)->m_encode_fps;
    bitrate = ((AVID*)videoid)->m_encode_bitrate;
  }
  else {
    fps = 0;
    bitrate = 0;
  }
}

unsigned int RtcEngine::GetVideoSenderObservedBitrate(lfrtcAVID videoid) {
  if (videoid) {
    return m_vieCodec->GetLastObservedBitrateBps(((AVID*)videoid)->m_id);
  }
  return 0;
}

int RtcEngine::EnableNoiseSuppression(bool enable) {
  int result = 0;
  result += m_voeApm->SetNsStatus(enable, webrtc::kNsDefault);
  result += m_voeApm->EnableHighPassFilter(enable);
  return result;
}

int RtcEngine::EnableAEC(bool enable) {
  //#if defined(ANDROID)
  //  const bool built_in_aec = m_voeHardware->BuiltInAECIsAvailable();
  //  if (built_in_aec) {
  //    m_voeHardware->EnableBuiltInAEC(enable_aec);
  //    if (enable) {
  //      enable = false;
  //    }
  //  }
  //#endif
  int result = 0;
#if defined(_WIN32) || defined(WEBRTC_MAC)
  result += m_voeApm->SetEcStatus(enable, webrtc::kEcAec);
#else
  result += m_voeApm->SetEcStatus(enable, webrtc::kEcAecm);
  result += m_voeApm->SetAecmMode(webrtc::kAecmLoudSpeakerphone, false);
#endif
  return result;
}

int RtcEngine::GetAudioRecorderDevice(lfrtcDevice* buf, int count) {
  InitAudioRecorderDeviceList();
  return CopyDevices(m_AudioRecorderDevices, buf, count);
}

int RtcEngine::GetAudioPlayoutDevice(lfrtcDevice* buf, int count) {
  InitAudioPlayoutDeviceList();
  return CopyDevices(m_AudioPlayoutDevices, buf, count);
}

int RtcEngine::GetCameraDevice(lfrtcDevice* buf, int count) {
  InitCameraDeviceList();
  return CopyDevices(m_CameraDevices, buf, count);
}

int RtcEngine::GetCameraCapability(const char* deviceid, lfrtcCameraCapability* buf, int count) {
  if (NULL == deviceid || NULL == buf || count <= 0) {
    return 0;
  }
  memset(buf, 0, count * sizeof(lfrtcCameraCapability));
  int caps = m_vieCapture->NumberOfCapabilities(deviceid, strlen(deviceid));
  if (caps <= 0) {
    return 0;
  }
  if (count > caps) {
    count = caps;
  }
  int added = 0;
  for (int i = 0; i < count; i++) {
    webrtc::CaptureCapability capability;
    if (0 == m_vieCapture->GetCaptureCapability(deviceid, strlen(deviceid), i, capability)) {
      bool existed = false;
      for (int j = 0; j < added; j++) {
        if (buf[j].width == capability.width && buf[j].height == capability.height
          && buf[j].maxFPS == capability.maxFPS) {
          existed = true;
          break;
        }
      }
      if (existed) {
        continue;
      }
      buf[i].width = capability.width;
      buf[i].height = capability.height;
      buf[i].maxFPS = capability.maxFPS;
      added++;
    }
  }
  return added;
}

RtcEngine::RtcEngine() {
  m_VideoEngine = webrtc::VideoEngine::Create();
  m_AudioEngine = webrtc::VoiceEngine::Create();
  m_vieBase = webrtc::ViEBase::GetInterface(m_VideoEngine);
  m_voeBase = webrtc::VoEBase::GetInterface(m_AudioEngine);
  int result = m_vieBase->Init();
  if (0 != result) {
    AVENGINE_ERR("init video engine error.");
    return;
  }
  result = m_voeBase->Init();
  if (0 != result) {
    AVENGINE_ERR("init audio engine error.");
    return;
  }

  m_vieRender = webrtc::ViERender::GetInterface(m_VideoEngine);
  m_vieCodec = webrtc::ViECodec::GetInterface(m_VideoEngine);
  m_vieExtCodec = webrtc::ViEExternalCodec::GetInterface(m_VideoEngine);
  m_vieNetwork = webrtc::ViENetwork::GetInterface(m_VideoEngine);
  m_vieRTPRTCP = webrtc::ViERTP_RTCP::GetInterface(m_VideoEngine);
  m_vieCapture = webrtc::ViECapture::GetInterface(m_VideoEngine);
  m_voeCodec = webrtc::VoECodec::GetInterface(m_AudioEngine);
  m_voeHardware = webrtc::VoEHardware::GetInterface(m_AudioEngine);
  m_voeApm = webrtc::VoEAudioProcessing::GetInterface(m_AudioEngine);
  m_voeNetwork = webrtc::VoENetwork::GetInterface(m_AudioEngine);
  m_voeVolume = webrtc::VoEVolumeControl::GetInterface(m_AudioEngine);
  m_voeRTPRTCP = webrtc::VoERTP_RTCP::GetInterface(m_AudioEngine);

  result = m_vieBase->SetVoiceEngine(m_AudioEngine);
}

RtcEngine::~RtcEngine() {
}

void RtcEngine::Release() {
  int error = 0;

  m_vieRender->Release();
  m_vieBase->Release();
  m_vieCodec->Release();
  m_vieExtCodec->Release();
  m_vieNetwork->Release();
  m_vieRTPRTCP->Release();
  m_voeVolume->Release();
  webrtc::VideoEngine::Delete(m_VideoEngine);

  m_voeBase->Terminate();
  error += m_voeCodec->Release();
  error += m_voeHardware->Release();
  error += m_voeApm->Release();
  error += m_voeBase->Release();
  error += m_voeNetwork->Release();
  error += m_voeRTPRTCP->Release();
  webrtc::VoiceEngine::Delete(m_AudioEngine);
}

int RtcEngine::GetVolume(int chanId, unsigned int &vol, int type) {
  int result = -1;
  if (type == 0) {
    bool muted = false;
    result = m_voeVolume->GetInputMute(chanId, muted);
    vol = muted;
  }
  else if (type == 1)  {
    result = m_voeVolume->GetMicVolume(vol);
  }
  else if (type == 2) {
    result = m_voeVolume->GetSpeakerVolume(vol);
  }
  return result;
}

int RtcEngine::SetVolume(int chanId, unsigned int vol, int type){
  int result = -1;
  if (type == 0) {
    result = m_voeVolume->SetInputMute(chanId, (bool)vol);
  }
  else if (type == 1) {
    result = m_voeVolume->SetMicVolume(vol);
  }
  else if (type == 2) {
    result = m_voeVolume->SetSpeakerVolume(vol);
  }
  return result;
}

//void RtcEngine::SetStretchMode(int mode) {
//}


int RtcEngine::StartPlay(void* userdata, lfrtcAVID &audioid, lfrtcAVID &videoid) {
  audioid = NULL;
  videoid = NULL;

  int aChannelID = m_voeBase->CreateChannel();
  int vChannelID = -1;
  m_vieBase->CreateChannel(vChannelID);
  if (vChannelID < 0 || aChannelID < 0) {
    m_voeBase->DeleteChannel(aChannelID);
    m_vieBase->DeleteChannel(vChannelID);
    return -1;
  }
  m_vieBase->ConnectAudioChannel(vChannelID, aChannelID);

  {
    audioid = new AVID(aChannelID);
    m_voeCodec->SetDecodeParam(aChannelID, (void*)g_DecodedAudioCallback, userdata, "");
    CustomExternalTransport *transport = new CustomExternalTransport((AVID*)audioid);
    ((AVID*)audioid)->m_transport = transport;
    m_voeNetwork->RegisterExternalTransport(aChannelID, *transport);
    m_voeBase->StartReceive(aChannelID);
    m_voeBase->StartPlayout(aChannelID);
  }
  
  {
    videoid = new AVID(vChannelID);
    H264Decoder *decoder = new H264Decoder();
    ((AVID*)videoid)->m_decoder = decoder;
    m_vieExtCodec->RegisterExternalReceiveCodec(vChannelID, RTP_PAYLOAD_TYPE_H264, decoder, false);

    webrtc::VideoCodec codec;
    int count = m_vieCodec->NumberOfCodecs();
    for (int i = 0; i < count; i++) {
      m_vieCodec->GetCodec(i, codec);
      if (0 == strcmp(codec.plName, "H264")) {
        break;
      }
    }
    codec.plType = RTP_PAYLOAD_TYPE_H264;
    codec.cb = (void *)g_DecodedVideoCallback;
    codec.object = userdata;
    codec.chanId = vChannelID;
    m_vieCodec->SetReceiveCodec(vChannelID, codec);
    //if (delay_ms > 0) {
    //  result = m_vieRTPRTCP->SetReceiverBufferingMode(vChannelID, delay_ms);
    //}
    CustomExternalTransport *transport = new CustomExternalTransport((AVID*)audioid);
    ((AVID*)videoid)->m_transport = transport;
    m_vieNetwork->RegisterSendTransport(vChannelID, *transport);
    m_vieBase->StartReceive(vChannelID);
  }

  return 0;
}

int RtcEngine::StopPlay(lfrtcAVID audioid, lfrtcAVID videoid) {
  int aChannelID = -1;
  int vChannelID = -1;
  if (audioid) {
    aChannelID = ((AVID*)audioid)->m_id;
  }
  if (videoid) {
    vChannelID = ((AVID*)videoid)->m_id;
  }

  if (vChannelID >= 0) {
    m_vieBase->DisconnectAudioChannel(vChannelID);

    m_vieBase->StopReceive(vChannelID);
    m_vieRender->StopRender(vChannelID);
    m_vieRender->RemoveRenderer(vChannelID);
    m_vieNetwork->DeregisterSendTransport(vChannelID);
    m_vieBase->DeleteChannel(vChannelID);
  }
  if (aChannelID >= 0) {
    m_voeBase->StopReceive(aChannelID);
    m_voeBase->StopPlayout(aChannelID);
    m_voeNetwork->DeRegisterExternalTransport(aChannelID);
    m_voeBase->DeleteChannel(aChannelID);
  }
  
  delete (AVID*)audioid;
  delete (AVID*)videoid;
  return 0;
}

int RtcEngine::SetAudioRtp(const void* data, int dataSize, lfrtcAVID audioid) {
  int result = -1;
  int channelID = -1;
  if (audioid) {
    channelID = ((AVID*)audioid)->m_id;
  }
  if (channelID >= 0) {
    result = m_voeNetwork->ReceivedRTPPacket(channelID, data, dataSize);
  }

  return result;
}

int RtcEngine::SetVideoRtp(const void* data, int dataSize, lfrtcAVID videoid) {
  int result = -1;
  int channelID = -1;
  if (videoid) {
    channelID = ((AVID*)videoid)->m_id;
  }
  if (channelID >= 0) {
    webrtc::PacketTime packet_time;
    result = m_vieNetwork->ReceivedRTPPacket(channelID, data, dataSize, packet_time);
  }

  return result;
}

int RtcEngine::SetAudioRtcp(const void* data, int dataSize, lfrtcAVID audioid) {
  int result = 0;
  int channelID = -1;
  if (audioid) {
    channelID = ((AVID*)audioid)->m_id;
  }
  if (channelID >= 0) {
    result = m_voeNetwork->ReceivedRTCPPacket(channelID, data, dataSize);
  }

  return result;
}

int RtcEngine::SetVideoRtcp(const void* data, int dataSize, lfrtcAVID videoid) {
  int result = -1;
  int channelID = -1;
  if (videoid) {
    channelID = ((AVID*)videoid)->m_id;
  }
  if (channelID >= 0) {
    result = m_vieNetwork->ReceivedRTCPPacket(channelID, data, dataSize);
  }
  

  return result;
}

int RtcEngine::SetAudioSSRC(unsigned int SSRC, lfrtcAVID audioid) {
  if (audioid) {
    return m_voeRTPRTCP->SetLocalSSRC(((AVID*)audioid)->m_id, SSRC);
  }
  return -1;
}

int RtcEngine::SetVideoSSRC(unsigned int SSRC, lfrtcAVID videoid) {
  if (videoid) {
    return m_vieRTPRTCP->SetLocalSSRC(((AVID*)videoid)->m_id, SSRC);
  }
  return -1;
}

int RtcEngine::SetPlayWindow(lfrtcAVID videoid, void *window) {
  int result = 0;
  int vChannelID = -1;
  if (videoid) {
    vChannelID = ((AVID*)videoid)->m_id;
  }
  m_vieRender->RemoveRenderer(vChannelID);
  if (window) {
    result += m_vieRender->AddRenderer(vChannelID, window, 0, 0.0f, 0.0f, 1.0f, 1.0f);
    result += m_vieRender->StartRender(vChannelID);
  }
  return result;
}

int RtcEngine::SetAudioPlayoutDevice(const char *audioszDeviceID) {
#ifndef __ANDROID__
  int deviceIndex = -1;
  if (audioszDeviceID) {
    InitAudioPlayoutDeviceList();
    for (int i = 0; i < (int)m_AudioPlayoutDevices.size(); i++) {
      if (0 == strcmp(audioszDeviceID, m_AudioPlayoutDevices[i].szDeviceID)) {
        deviceIndex = i;
        break;
      }
    }
  }
  m_voeHardware->SetPlayoutDevice(deviceIndex);
#endif
  return 0;
}
