//edited by zhangle
#ifndef WEBRTC_MODULES_AVCODING_CODECS_FFMPEG_MAIN_SOURCE_FFMPEG_FACTORY_H_
#define WEBRTC_MODULES_AVCODING_CODECS_FFMPEG_MAIN_SOURCE_FFMPEG_FACTORY_H_

#include "webrtc/avengine/interface/ffmpeg_api.h"
#include <vector>

class RtpPacket2Frame {
public:
  RtpPacket2Frame();

  // data: 当前帧所包含的rtp packet按顺序填充在该buf中
  int ParseRtpPackets(const char *data, int len);
  char* GetFrameData();
  int GetFrameLen();
  bool IsFrameComplete();
  bool IsKeyframe();
  // frame的第1个包的seqNum与上次的seqNum是否可以接得上
  bool IsContinuousWithLast();
  // 当前帧的missing count，这个值不准。当first packet没收到时，返回的值可能比实际的大；当last packet没收到时， 返回值可能为0，而实际上该帧有丢包
  int MissingPacketCount();
  unsigned short GetLatestSeqNum();

private:
  unsigned short m_seq_num;
  std::vector<char> m_frame_data;
  int m_frame_len;
  bool m_frame_complete;
  bool m_frame_continuous;
  bool m_keyframe;
  int m_missing_count;
  bool m_init;
};

#endif