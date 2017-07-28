#include "webrtc/modules/av_coding/codecs/ffmpeg/main/source/ffmpeg_factory.h"

#include "webrtc/base/basetype.h"
#include <assert.h>
#ifndef WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif

namespace {
  typedef struct {
    //byte 0
    unsigned char TYPE : 5;
    unsigned char NRI : 2;
    unsigned char F : 1;
  } FU_INDICATOR; /**//* 1 BYTES */

  typedef struct {
    //byte 0
    unsigned char TYPE : 5;
    unsigned char R : 1;
    unsigned char E : 1;
    unsigned char S : 1;
  } FU_HEADER; /**//* 1 BYTES */
}

RtpPacket2Frame::RtpPacket2Frame() {
  m_seq_num = 0;
  m_frame_len = 0;
  m_frame_complete = false;
  m_frame_continuous = false;
  m_keyframe = false;
  m_missing_count = 0;
  m_init = true;
}

int RtpPacket2Frame::ParseRtpPackets(const char *data, int len) {
  m_frame_len = 0;
  m_frame_complete = false;
  m_frame_continuous = false;
  m_keyframe = false;
  m_missing_count = 0;
  m_frame_data.clear();
  if (data == NULL || len < (int)sizeof(RTP_FIXED_HEADER)) {
    return -1;
  }

  unsigned short last_seq_num = m_seq_num;
  bool got_first_packet = false;
  bool got_last_packet = false;
  RTP_FIXED_HEADER header;
  EXTEND_HEADER extend;
  while (len > (int)sizeof(RTP_FIXED_HEADER)) {
    memcpy(&header, data, sizeof(RTP_FIXED_HEADER));

    unsigned short seqnum = ntohs(header.seq_no);

    if (m_init) {
      m_init = false;
      m_frame_continuous = true;
      m_seq_num = seqnum;
    }
    else if (last_seq_num + 1 == seqnum) {
      m_frame_continuous = true;
    }

    m_missing_count += int(seqnum - m_seq_num-1);
    m_seq_num = seqnum;

    if (!header.extension || len < (int)sizeof(RTP_FIXED_HEADER)+(int)sizeof(EXTEND_HEADER)) {
      return -1;
    }
    memcpy(&extend, data + sizeof(RTP_FIXED_HEADER), sizeof(EXTEND_HEADER));

    int ext_len = (ntohs(extend.rtp_extend_length) + 1) << 2;
    int rtp_len = ntohs(extend.rtp_extend_rtplen);

    if (extend.first_packet) {
      assert(!got_first_packet);
      assert(m_frame_data.empty());
      got_first_packet = true;
      m_missing_count = 0;
    }
    if (extend.keyframe) {
      m_keyframe = true;
    }
    assert(!got_last_packet);
    if (header.marker) {
      got_last_packet = true;
    }

    if ((ext_len != (int)sizeof(EXTEND_HEADER)) || (rtp_len <= (int)sizeof(RTP_FIXED_HEADER)+ext_len) || len < rtp_len) {
      return -1;
    }

    FU_INDICATOR *fu_ind = (FU_INDICATOR*)(data + sizeof(RTP_FIXED_HEADER)+ext_len); // FU_INDICATOR只有1个字节，直接强转

    if (fu_ind->TYPE == 28) {
      // 1个nal分成了多个packet

      FU_HEADER *fu_hdr = (FU_HEADER*)(data + sizeof(RTP_FIXED_HEADER)+ext_len + 1); // FU_HEADER只有1个字节，直接强转
      int total_header_len = sizeof(RTP_FIXED_HEADER)+ext_len + 2;   // 头部总长度RTP_FIXED_HEADER + ext_len + FU_INDICATOR + FU_HEADER
      int payload_len = rtp_len - total_header_len; // nal的payload长度
      if (rtp_len < total_header_len) {
        return -1;
      }

      if (fu_hdr->S) {
        // nal的第一个包
        const int nal_startcode_len = 4;
        int old_frame_len = m_frame_data.size();
        m_frame_data.resize(old_frame_len + nal_startcode_len + 1 + payload_len);  // 增加的数据为nal_start_code + nal_header(1Byte) + playload
        char *buf = &m_frame_data[old_frame_len];
        buf[0] = 0;
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = 1;
        buf[4] = (fu_ind->F << 7) | (fu_ind->NRI << 5) | fu_hdr->TYPE;
        memcpy(buf + nal_startcode_len + 1, data + total_header_len, payload_len);
      }
      else {
        int old_frame_len = m_frame_data.size();
        m_frame_data.resize(old_frame_len + payload_len);  // 增加的数据为playload
        char *buf = &m_frame_data[old_frame_len];
        memcpy(buf, data + total_header_len, payload_len);
      }
    }
    else {
      // 单个nal被打包成单个packet
      const int nal_startcode_len = 4;

      int total_header_len = sizeof(RTP_FIXED_HEADER)+ext_len;   // 头部总长度RTP_FIXED_HEADER + ext_len
      int payload_len = rtp_len - total_header_len; // nal_header + nal_payload整个作为payload

      int old_frame_len = m_frame_data.size();
      m_frame_data.resize(old_frame_len + nal_startcode_len + payload_len);  // 增加的数据为nal_start_code + playload
      char *buf = &m_frame_data[old_frame_len];
      buf[0] = 0;
      buf[1] = 0;
      buf[2] = 0;
      buf[3] = 1;
      memcpy(buf + nal_startcode_len, data + total_header_len, payload_len);
    }

    len -= rtp_len;
    data += rtp_len;
  }

  m_frame_len = m_frame_data.size();
  m_frame_complete = got_first_packet && got_last_packet && (m_missing_count == 0);
  return 0;
}

char* RtpPacket2Frame::GetFrameData() {
  if (m_frame_len <= 0) {
    return NULL;
  }
  else {
    return &m_frame_data[0];
  }
}

int RtpPacket2Frame::GetFrameLen() {
  return m_frame_len;
}

bool RtpPacket2Frame::IsFrameComplete() {
  return m_frame_complete;
}

bool RtpPacket2Frame::IsKeyframe() {
  return m_keyframe;
}

int RtpPacket2Frame::MissingPacketCount() {
  return m_missing_count;
}

unsigned short RtpPacket2Frame::GetLatestSeqNum() {
  return m_seq_num;
}

bool RtpPacket2Frame::IsContinuousWithLast() {
  return m_frame_continuous;
}
