// edit by zhangle
#ifndef AVENGINE_SOURCE_AVENGINE_INTERNAL_H_
#define AVENGINE_SOURCE_AVENGINE_INTERNAL_H_

#include "webrtc/avengine/interface/avengine_api.h"
#include <vector>

namespace webrtc {
  class VoiceEngine;
  class VoEBase;
  class VoECodec;
  class VoEHardware;
  class VoEAudioProcessing;
  class VoENetwork;
  class VoEVolumeControl;
  class VoERTP_RTCP;
  class VideoEngine;
  class ViERender;
  class ViECodec;
  class ViEExternalCodec;
  class ViENetwork;
  class ViERTP_RTCP;
  class ViEBase;
  class ViECapture;
  class CriticalSectionWrapper;
}

class RtcEngine {
public:
  RtcEngine();
  ~RtcEngine();

  int SetAudioPlayoutDevice(const char *audioszDeviceID);
  int StartPlay(void* userdata, lfrtcAVID &audioid, lfrtcAVID &videoid);
  int SetPlayWindow(lfrtcAVID videoid, void *window);
  int StopPlay(lfrtcAVID audioid, lfrtcAVID videoid);
  
  int SetAudioRtp(const void* data, int dataSize, lfrtcAVID audioid);
  int SetVideoRtp(const void* data, int dataSize, lfrtcAVID videoid);
  int SetAudioRtcp(const void* data, int dataSize, lfrtcAVID audioid);
  int SetVideoRtcp(const void* data, int dataSize, lfrtcAVID videoid);
  int SetAudioSSRC(unsigned int SSRC, lfrtcAVID audioid);
  int SetVideoSSRC(unsigned int SSRC, lfrtcAVID videoid);

  int StartCapture(const lfrtcCaptureConfig *config, lfrtcAVID &captureid);
  int StartPreview(void *window, lfrtcAVID captureid);
  int StartPreview(lfrtcAVID captureid, T_lfrtcPreviewVideoCb callback, lfrtcRawVideoType type, void *userdata);
  int StartEncodeAndSend(lfrtcAVID captureid, const lfrtcEncodeConfig* config, lfrtcAVID &audioid, lfrtcAVID &videoid, void* userdata);
  void StopEncodeAndSend(lfrtcAVID audioid, lfrtcAVID videoid);
  void StopCapture(lfrtcAVID captureid);
  void StopPreview(lfrtcAVID captureid);
  void GetVideoEncodeInfo(lfrtcAVID videoid, unsigned int &fps, unsigned int &bitrate);
  unsigned int GetVideoSenderObservedBitrate(lfrtcAVID videoid);

  int EnableNoiseSuppression(bool enable);
  int EnableAEC(bool enable);

  int GetAudioRecorderDevice(lfrtcDevice* devices, int bufcount);
  int GetAudioPlayoutDevice(lfrtcDevice* devices, int bufcount);
  int GetCameraDevice(lfrtcDevice* devices, int bufcount);
  int GetCameraCapability(const char* deviceid, lfrtcCameraCapability* buf, int count);

  int GetVolume(int chanId, unsigned int &vol, int type);
  int SetVolume(int chanId, unsigned int vol, int type);

  void Release();

private:
  int InitAudioPlayoutDeviceList();
  int InitCameraDeviceList();
  int InitAudioRecorderDeviceList();

private:
  webrtc::VoiceEngine* m_AudioEngine;
  webrtc::VoEBase* m_voeBase;
  webrtc::VoECodec* m_voeCodec;
  webrtc::VoEHardware* m_voeHardware;
  webrtc::VoEAudioProcessing* m_voeApm;
  webrtc::VoENetwork* m_voeNetwork;
  webrtc::VoEVolumeControl* m_voeVolume;
  webrtc::VoERTP_RTCP* m_voeRTPRTCP;
  webrtc::VideoEngine* m_VideoEngine;
  webrtc::ViERender*   m_vieRender;
  webrtc::ViECodec*    m_vieCodec;
  webrtc::ViEExternalCodec* m_vieExtCodec;
  webrtc::ViENetwork*  m_vieNetwork;
  webrtc::ViERTP_RTCP* m_vieRTPRTCP;
  webrtc::ViEBase*     m_vieBase;
  webrtc::ViECapture*  m_vieCapture;
  std::vector<lfrtcDevice> m_AudioPlayoutDevices;
  std::vector<lfrtcDevice> m_CameraDevices;
  std::vector<lfrtcDevice> m_AudioRecorderDevices;
};

#endif
