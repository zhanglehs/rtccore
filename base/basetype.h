#ifndef BASTYPE_H_
#define BASTYPE_H_
//edited by gxh

typedef struct
{
	/**//* byte 0 */
	unsigned char csrc_len : 4;        /**//* expect 0 */
	unsigned char extension : 1;        /**//* expect 1, see RTP_OP below */
	unsigned char padding : 1;        /**//* expect 0 */
	unsigned char version : 2;        /**//* expect 2 */

	/**//* byte 1 */
	unsigned char payload : 7;        /**//* RTP_PAYLOAD_RTSP */
	unsigned char marker : 1;        /**//* expect 1 */

	/**//* bytes 2, 3 */
	unsigned short seq_no;
	/**//* bytes 4-7 */
	unsigned  int timestamp;
	/**//* bytes 8-11 */
	unsigned int ssrc;            /**//* stream number is used here. */
} RTP_FIXED_HEADER;

// 扩展头的前两个字段是官方的规定，后面的字段是字定义的
typedef struct {
	unsigned short rtp_extend_profile; //profile used
	unsigned short rtp_extend_length;
	unsigned short rtp_extend_rtplen;
  unsigned char padding1 : 6;
  unsigned char first_packet : 1;
  unsigned char keyframe : 1;
  unsigned char padding2;
} EXTEND_HEADER;

struct Packet
{
	int size;
	//int stream_type;
	char *data;
};
#endif