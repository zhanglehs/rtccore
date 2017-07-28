//edited by zhangle
#ifndef WEBRTC_MODULES_AVCODING_CODECS_FFMPEG_MAIN_SOURCE_FFMPEG_FACTORY_H_
#define WEBRTC_MODULES_AVCODING_CODECS_FFMPEG_MAIN_SOURCE_FFMPEG_FACTORY_H_

#include "webrtc/avengine/interface/ffmpeg_api.h"
#include <vector>

class RtpPacket2Frame {
public:
  RtpPacket2Frame();

  // data: ��ǰ֡��������rtp packet��˳������ڸ�buf��
  int ParseRtpPackets(const char *data, int len);
  char* GetFrameData();
  int GetFrameLen();
  bool IsFrameComplete();
  bool IsKeyframe();
  // frame�ĵ�1������seqNum���ϴε�seqNum�Ƿ���Խӵ���
  bool IsContinuousWithLast();
  // ��ǰ֡��missing count�����ֵ��׼����first packetû�յ�ʱ�����ص�ֵ���ܱ�ʵ�ʵĴ󣻵�last packetû�յ�ʱ�� ����ֵ����Ϊ0����ʵ���ϸ�֡�ж���
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