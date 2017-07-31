//edited by gxh
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "webrtc/avengine/interface/avengAPI.h"
///#include "webrtc/av-superlogic/api.h"
#include "webrtc/avengine/interface/gxhlog.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
//#include <vld.h>
#include <Mmsystem.h>
#else
#define EXT_WIN
#endif
#include <map>

#define MAX_ORDER	40//20
#define MAX_BUFFER	(4*MAX_ORDER)//(3*MAX_ORDER)
#define EXT_WIN
#define DEV_NAME_LEN	256
#define MAX_CHAN_NUM 16

#ifndef _WIN32
typedef int64_t __int64;
typedef int64_t DWORD;
#endif


typedef struct 
{
	/**//* byte 0 */
	unsigned char csrc_len:4;        /**//* expect 0 */
	unsigned char extension:1;        /**//* expect 1, see RTP_OP below */
	unsigned char padding:1;        /**//* expect 0 */
	unsigned char version:2;        /**//* expect 2 */
	/**//* byte 1 */
	unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */
	unsigned char marker:1;        /**//* expect 1 */
	/**//* bytes 2, 3 */
	unsigned short seq_no;            
	/**//* bytes 4-7 */
	unsigned  long timestamp;        
	/**//* bytes 8-11 */
	unsigned long ssrc;            /**//* stream number is used here. */
} RTP_FIXED_HEADER;
//
#ifndef GXH_TEST_JB
#define MAX_BUFFER_NUM		MAX_BUFFER//200
#define KMaxPacketSize		2000//1500

struct Packet
{
	int size;
	RTP_FIXED_HEADER *pRtpHeader;
	unsigned short seq_no;
	unsigned int timeStamp;
	unsigned char data[KMaxPacketSize];
};
typedef std::map<int, Packet *> JBufferMap;

class AVReceiveJB
{
public:
	AVReceiveJB();
	~AVReceiveJB();
	void SetMaxDelayTime(int time) { _MaxDelayTime = time; }
	void SetMinDelayTime(int time) { _MinDelayTime = time; }
	void SetDefaultDelayTime(int time) { _DefaultDelayTime = time; }
	bool PushJB(unsigned char *data, int len)
	{
		if (len > KMaxPacketSize)
			return false;
		
		unsigned short seq_no = (data[2] << 8) + data[3];
		unsigned int timeStamp = (data[4] << 24) + (data[5] << 16) + (data[6] << 8) + (data[7] << 0);
		int idx = seq_no % MAX_BUFFER_NUM;
		if (seq_no < _MinSeqNum)
		{
			seq_no = _MinSeqNum;
		}
		if (seq_no > _MaxSeqNum)
		{
			seq_no = _MaxSeqNum;
		}
		JBufferMap::iterator it = _jbMap.find(idx);
		if (it != _jbMap.end())
		{
			_jbMap.erase(it);
		}
		Packet *pkt = new Packet;
		pkt->pRtpHeader = (RTP_FIXED_HEADER *)data;
		pkt->size = len;
		memcpy(pkt->data, data, len);
		_jbMap.insert(std::pair<int, Packet *>(idx, pkt));
		_MapSize = _jbMap.size();
		return true;
	}
	int GetDiffTime()
	{
		int difftime = 0;
		if (_MinSeqNum == -1 || !_MaxSeqNum || _MapSize < 3)
		{
			return difftime;
		}
		int idx0 = _MinSeqNum % MAX_BUFFER_NUM;
		int idx1 = _MaxSeqNum % MAX_BUFFER_NUM;
		unsigned int timeStamp0 = 0;
		unsigned int timeStamp1 = 0;
		Packet *pkt = NULL;
		JBufferMap::iterator it = _jbMap.find(idx0);
		if (it != _jbMap.end())
		{
			pkt = (Packet *)it->second;
			timeStamp0 = pkt->timeStamp;
		}
		it = _jbMap.find(idx1);
		if (it != _jbMap.end())
		{
			pkt = (Packet *)it->second;
			timeStamp1 = pkt->timeStamp;
		}
		//
		int factor = (900 * 1000) / 25;//36000
		//(difftime * (1000 / 25)) / ((900 * 1000) / 25)
		//tim = difftime / 900 ms
		difftime = (int)(timeStamp1 - timeStamp0) / 900;
		return difftime;
	}
	bool GetJB(unsigned char *data, int *len)
	{
		JBufferMap::iterator it = _jbMap.begin();//.find(roomId);
		while (it != _jbMap.end())
		{
			it++;
		}
		return true;
	}
	void ReleaseJB()
	{
		JBufferMap::iterator it = _jbMap.begin();//.find(roomId);
		while (it != _jbMap.end())
		{
			_jbMap.erase(it);
			it++;
		}
	}
public:
	int _MaxDelayTime;//retransmite time
	int _MinDelayTime;//timestamp interval
	int _DefaultDelayTime;//one fame time (1000/framerate ms)
	unsigned int _MinTimeStamp;
	unsigned int _MaxTimeStamp;
	unsigned short _MinSeqNum;
	unsigned short _MaxSeqNum;
	int _MapSize;
	JBufferMap _jbMap;
};
AVReceiveJB::AVReceiveJB()
:_MaxDelayTime(0), _MinDelayTime(0), _DefaultDelayTime(40), _MinTimeStamp(0), _MaxTimeStamp(0), _MapSize(0), _MinSeqNum(-1), _MaxSeqNum(0)
{
}
AVReceiveJB::~AVReceiveJB()
{
}
#endif
//
class AVEngineTest
{
public:
	AVEngineTest();
	~AVEngineTest();

	static int RtpSendCallBack(void *object, int cmd, int channel, char *buf,int len);
	int RtpSend(int cmd, int channel, char *buf, int len);
	//int Ready();
	int StartEnc(int64_t streamId, char *cparams, void *cb, void *obj, char (avDev[3])[128]);
	int StartDec(int64_t streamId, char *cparams, void *cb, void *obj, char (avDev[3])[128]);
	int StopEnc(int64_t streamId);
	int StopDec(int64_t streamId);
	void Release();
public:
	//MyWindows *wind;
	void *pHandle;
	void *wind;
	void *wind_1;
	int captureID;
	int channelID;
	int recoderId;
	int aChannelId;
	int dChannelID;
	int aDChannelId;
	char capDev[DEV_NAME_LEN];
	char recoderDev[DEV_NAME_LEN];
	char playerDev[DEV_NAME_LEN];
	unsigned char videoBuf[MAX_BUFFER][1500];
	int videoPktSize[MAX_BUFFER];
	__int64 videoPktIdx;
	__int64 lastVideoPktIdx;
	int testReorder;
	unsigned int lastTimeStamp;
	int groupId[MAX_CHAN_NUM];
	int64_t streamID[MAX_CHAN_NUM];
	int groupIdx;
	int winIdx;
#ifndef GXH_TEST_JB
	AVReceiveJB JB;
#endif
	webrtc::CriticalSectionWrapper* _critsect;
};

#if	1
int RawDataOutputCallBack0(void *object, int type, int chanId, char *buf[3], int len, __int64 timestamp)
{
	int ret = 0;
	if(type)
	{
#if 0
		static FILE* fp0 = NULL;
		if(!fp0) fp0 = fopen("c://works//test//video_d.yuv","wb");
		if(fp0)
		{
			int size = (len * 2) / 3;
			int wsize = fwrite(buf[0], 1, size, fp0);
			wsize = fwrite(buf[1], 1, size >> 2, fp0);
			wsize = fwrite(buf[2], 1, size >> 2, fp0);

		}
#endif
		//printf("video output dec len = %d \t object=%d\n", len, object);
	}
	else
	{
#if 0
		static FILE* fp1 = NULL;
		if(!fp1) fp1 = fopen("c://works//test//audio_d.pcm","wb");
		if(fp1)
		{
			int wsize = fwrite(buf[0], 1, len, fp1);
		}
#endif
		//printf("audio output dec len = %d \t object=%d \n", len, object);
	}

	return 0;
}
#endif

AVEngineTest::AVEngineTest()
	:dChannelID(-1),
	aDChannelId(-1),
	videoPktIdx(0),
	lastVideoPktIdx(0),
	lastTimeStamp(0),
	testReorder(MAX_ORDER),
	groupIdx(0),
	winIdx(0),
	_critsect(webrtc::CriticalSectionWrapper::CreateCriticalSection())
{
	pHandle = NULL;
	wind = NULL;
	wind_1 = NULL;
	//
	memset(groupId, -1, MAX_CHAN_NUM * sizeof(int));
	memset(streamID, -1, MAX_CHAN_NUM * sizeof(int));
	memset(capDev, 0, DEV_NAME_LEN * sizeof(char));
	memset(recoderDev, 0, DEV_NAME_LEN * sizeof(char));
	memset(playerDev, 0, DEV_NAME_LEN * sizeof(char));
	//
	char *devStr = "";
#ifndef EXT_WIN
	int winNum = 2;
	int extwin = 0;
#else
	int winNum = 1;
	int extwin = 1;
#endif
    LogOut("IICreateAVObj 0 \n",NULL);
	IICreateAVObj(&pHandle, winNum, extwin, devStr);
    LogOut("IICreateAVObj:pHandle = %x = \n", (void *)&pHandle);
}
AVEngineTest::~AVEngineTest()
{

}
int AVEngineTest::StopEnc(int64_t streamId)
{
	int type = kEncoder;
	int groupId = IIFindGroupId(pHandle, type, streamId);
	int ret = IIStopAVCodec(pHandle, groupId, 0);
	return ret;
}
int AVEngineTest::StopDec(int64_t streamId)
{
	int type = kDecoder;
	int groupId = IIFindGroupId(pHandle, type, streamId);;
	int ret = IIStopAVCodec(pHandle, groupId, 0);
	return ret;
}
void AVEngineTest::Release()
{
	IICloseAVEngine(pHandle);
	IIDeleteAVObj(pHandle);
	delete _critsect;
	_critsect = NULL;
}
int AVEngineTest::RtpSendCallBack(void *object, int cmd, int channel, char *buf,int len)
{
	return static_cast<AVEngineTest*>(object)->RtpSend(cmd, channel, buf, len);
}
int PktTransfor(char *inBuf, int inLen, char *outBuf, int outLen, int flag)
{
	int ret = 0;
	uint8_t kFirstPacketBit = 0x02;
	int kHeadSize = 12;
	static unsigned int lastTimeStamp = 0;
	static unsigned short lastSeqNum = 0;
	if(flag)
	{
		memcpy(outBuf, inBuf, kHeadSize);
		memcpy(&outBuf[kHeadSize + 1], &inBuf[kHeadSize],inLen - kHeadSize);
		outBuf[kHeadSize] = kFirstPacketBit;//此标志丢失将无法判断是否完整帧
		ret = inLen + 1;
	}
	else
	{
		memcpy(outBuf, inBuf, inLen);
		ret = inLen;
	}
	{
		RTP_FIXED_HEADER *rtpHeader0 = (RTP_FIXED_HEADER *)&outBuf[kHeadSize + flag];
		RTP_FIXED_HEADER *rtpHeader = (RTP_FIXED_HEADER *)outBuf;
		unsigned short seq_no = (outBuf[2] << 8) + outBuf[3];
		unsigned int timeStamp = (outBuf[4] << 24) + (outBuf[5] << 16) + (outBuf[6] << 8) + (outBuf[7] << 0);
		unsigned int SSRC = (outBuf[8] << 24) + (outBuf[9] << 16) + (outBuf[10] << 8) + (outBuf[11] << 0);
		const bool          M  = ((outBuf[1] & 0x80) == 0) ? false : true;

		if(rtpHeader->marker)
			//if(M)
		{
			printf("time:%u \t",timeStamp);//rtpHeader->timestamp);
			unsigned int diffTime = timeStamp - lastTimeStamp;
			if(timeStamp > lastTimeStamp)
			{
				printf("difftime:%u \t",diffTime);
			}
			else
			{
				diffTime = lastTimeStamp - timeStamp;
				printf("difftime:%u -\t",diffTime);
			}
			//if(diffTime > 3600*10)
			//{
			//	printf("");
			//}
			printf("marker:%d \n",seq_no);//(rtpHeader->seq_no >> 3));
			lastTimeStamp = timeStamp;
			kFirstPacketBit = 0x02;
		}
		else
		{
			kFirstPacketBit = 0;
		}
		lastSeqNum = seq_no;
	}
	
	
	return ret;
}
int AVEngineTest::RtpSend(int cmd, int channel, char *buf, int len)
{
	int ret = 0;
	DWORD time0 = 0;
	DWORD time1 = 0;
	int diff = 0;
	_critsect->Enter();
	if(cmd == video && dChannelID >= 0)
	{
		unsigned char testBuf[1500] = "";
		unsigned char testBuf1[1500] = "";
		static int flag = 0;
		//int order[MAX_ORDER] = { 0, 1, 2, 18, 17, 15, 6, 7, 8, 9, 10, 11, 12, 13, 14, 5, 16, 4, 3, 19 };
		//int order[MAX_ORDER] = { 0, 1, 2, 3, 34, 35, 36, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 4, 5, 6, 37, 38, 39 };
		int order[MAX_ORDER] = { 0, 1, 2, 3, 24, 25, 26, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 4, 5, 6, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };
		//int order[MAX_ORDER] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };
		int i = videoPktIdx % MAX_BUFFER;
		int n = i / MAX_ORDER;
		int m = i % MAX_ORDER;
		int k = n * MAX_ORDER + order[m];
//		time0 = timeGetTime();
		memcpy(videoBuf[k], buf, len);
		videoPktSize[k] = len;

		//videoPktIdx++;
#if 0
		static FILE *fp = NULL;
		if(!fp)
		{
			fp = fopen("c://works//test//video-rtp.h264", "wb");
		}
		if(fp)
		{
			int rsize = fwrite(&len, 4, 1, fp);
			rsize = fwrite(buf, 1, len, fp);
		}
#endif
#if 0
		static FILE *fp = NULL;
		if(!fp)
		{
			fp = fopen("c://works//test//video-rtp.h264", "rb");	flag = 0;
			//fp = fopen("c://works//test//rtpd.h264", "rb");	flag = 1;
		}
		if(fp)
		{
			int pktSize = 30;
			char tmp[4] = "";
			//int rsize = fread(tmp, 4, 1, fp);
			//pktSize = (tmp[0] << 24) | (tmp[1] << 16) | (tmp[2] << 8) | tmp[3];
			int rsize = fread(&pktSize, 4, 1, fp);
			rsize = fread(testBuf, 1, pktSize, fp);
			len = PktTransfor((char *)testBuf, pktSize, (char *)testBuf1, 1500, flag);

		}
		ret = IIPlayVideo(pHandle, testBuf1, len, dChannelID);
#elif(1)

		if (videoPktIdx > testReorder)
		{
			static int idx = 0;
			int size = videoPktSize[idx];
#ifndef GXH_TEST_JB
			{
				JB.PushJB((unsigned char *)&videoBuf[idx], videoPktSize[idx]);
				JB.GetDiffTime();
			}

#endif
			ret = IIPlayVideo(pHandle, &videoBuf[idx], size, dChannelID);

			if (idx)
			{
				RTP_FIXED_HEADER *rtpHeader0 = (RTP_FIXED_HEADER *)&videoBuf[idx];
				RTP_FIXED_HEADER *rtpHeader = (RTP_FIXED_HEADER *)&videoBuf[idx - 1];

				unsigned short seq_no0 = (videoBuf[idx][2] << 8) + videoBuf[idx][3];
				unsigned int timeStamp0 = (videoBuf[idx][4] << 24) + (videoBuf[idx][5] << 16) + (videoBuf[idx][6] << 8) + (videoBuf[idx][7] << 0);
				unsigned short seq_no1 = (videoBuf[idx - 1][2] << 8) + videoBuf[idx - 1][3];
				unsigned int timeStamp1 = (videoBuf[idx - 1][4] << 24) + (videoBuf[idx - 1][5] << 16) + (videoBuf[idx - 1][6] << 8) + (videoBuf[idx - 1][7] << 0);
				unsigned short seq_diff = seq_no1 - seq_no0;
				unsigned int time_diff = timeStamp1 - timeStamp0;
				printf("");
			}

			idx++;
			if (idx >= MAX_BUFFER)
			{
				idx = 0;
			}
		}
#else
		//static unsigned int lastTimeStamp = -1;
		//unsigned char *buffer = (unsigned char *)buf;
		//unsigned int timeStamp = (buffer[4] << 24) + (buffer[5] << 16) + (buffer[6] << 8) + (buffer[7] << 0);
		//unsigned int diffTime = timeStamp - lastTimeStamp;
		////printf("difftime:%u \n", timeStamp);
		//if (lastTimeStamp > 0 && diffTime > 0)
		//{
		//	printf("difftime:%u \n", diffTime);
		//}
		ret = IIPlayVideo(pHandle, buf, len, dChannelID);
		//lastTimeStamp = timeStamp;
#endif		
		videoPktIdx++;
//		time1 = timeGetTime();
//		if(1)
//		{
//			diff = (int)(time1 - time0);
//			if(diff > 1)
//			{
				//printf("videoPktIdx= %d: time = %d \n", videoPktIdx, diff);
//			}
//		}
	}
	else if(aDChannelId >= 0)
	{
		ret = IIPlayAudio(pHandle, buf, len, aDChannelId);
	}
	_critsect->Leave();
	return len;
}
#if 0
int AVEngineTest::Ready()
{
	//video
#ifdef EXT_WIN
	//wind = ?
	IIAddExtWin((void *)pHandle, wind);
#endif
	IISetSndPktCallback((void *)pHandle, (void *)&RtpSendCallBack, this);
	//
	captureID = IIStartPreview((void *)pHandle, wind, capDev);
	//audio
	recoderId = IIStartRecoderDev((void *)pHandle, recoderDev);
	
	return 0;
}
#endif
static char *cparams[4] =
{
"{\"genParams\":{\
\"debugInfo\":\"0\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"8\",\
\"in_data_format\":\"0\",\
\"width\":\"640\",\
\"height\":\"480\",\
\"width1\":\"320\",\
\"height1\":\"240\",\
\"width2\":\"160\",\
\"height2\":\"120\",\
\"gop_size\":\"50\",\
\"mtu_size\":\"1400\",\
\"frame_rate\":\"25\",\
\"min_bits_rate\":\"128000\",\
\"max_bits_rate\":\"512000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"video_bits_rate\":\"256000\",\
\"video_bits_rate1\":\"128000\",\
\"video_bits_rate2\":\"64000\",\
\"video_codec_id\":\"28\",\
\"frame_size\":\"2048\",\
\"in_channel_count\":\"2\",\
\"in_sample_rate\":\"48000\",\
\"in_sample_fmt\":\"8\",\
\"out_channel_count\":\"2\",\
\"out_sample_rate\":\"48000\",\
\"out_sample_fmt\":\"1\",\
\"audio_port\":\"18001\",\
\"in_filename\":\"\",\
\"out_filename\":\"live.flv\",\
\"out_streamname\":\"rtmp://127.0.0.1/live/live\",\
\"sdp_filename\":\"video.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"genParams\":{\
\"debugInfo\":\"0\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"0\",\
\"in_data_format\":\"0\",\
\"width\":\"640\",\
\"height\":\"480\",\
\"gop_size\":\"50\",\
\"mtu_size\":\"1400\",\
\"frame_rate\":\"25\",\
\"min_bits_rate\":\"128000\",\
\"max_bits_rate\":\"512000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"video_bits_rate\":\"256000\",\
\"video_codec_id\":\"28\",\
\"frame_size\":\"2048\",\
\"in_channel_count\":\"2\",\
\"in_sample_rate\":\"48000\",\
\"in_sample_fmt\":\"8\",\
\"out_channel_count\":\"2\",\
\"out_sample_rate\":\"48000\",\
\"out_sample_fmt\":\"1\",\
\"audio_port\":\"19001\",\
\"in_filename\":\"\",\
\"out_filename\":\"\",\
\"out_streamname\":\"\",\
\"sdp_filename\":\"video.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"genParams\":{\
\"debugInfo\":\"0\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"8\",\
\"in_data_format\":\"0\",\
\"width\":\"640\",\
\"height\":\"480\",\
\"gop_size\":\"50\",\
\"mtu_size\":\"1400\",\
\"frame_rate\":\"25\",\
\"min_bits_rate\":\"128000\",\
\"max_bits_rate\":\"512000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"audio_bits_rate\":\"24000\",\
\"audio_codec_id\":\"86018\",\
\"frame_size\":\"2048\",\
\"in_channel_count\":\"2\",\
\"in_sample_rate\":\"48000\",\
\"in_sample_fmt\":\"8\",\
\"out_channel_count\":\"2\",\
\"out_sample_rate\":\"48000\",\
\"out_sample_fmt\":\"1\",\
\"audio_port\":\"18001\",\
\"in_filename\":\"\",\
\"out_filename\":\"live.flv\",\
\"out_streamname\":\"rtmp://127.0.0.1/live/live\",\
\"sdp_filename\":\"audio.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"genParams\":{\
\"debugInfo\":\"0\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"0\",\
\"in_data_format\":\"0\",\
\"width\":\"640\",\
\"height\":\"480\",\
\"gop_size\":\"50\",\
\"mtu_size\":\"1400\",\
\"frame_rate\":\"25\",\
\"min_bits_rate\":\"128000\",\
\"max_bits_rate\":\"512000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"audio_bits_rate\":\"24000\",\
\"audio_codec_id\":\"86018\",\
\"frame_size\":\"2048\",\
\"in_channel_count\":\"2\",\
\"in_sample_rate\":\"48000\",\
\"in_sample_fmt\":\"8\",\
\"out_channel_count\":\"2\",\
\"out_sample_rate\":\"48000\",\
\"out_sample_fmt\":\"1\",\
\"audio_port\":\"19001\",\
\"in_filename\":\"\",\
\"out_filename\":\"\",\
\"out_streamname\":\"\",\
\"sdp_filename\":\"audio.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
"
};
int AVEngineTest::StartEnc(int64_t streamId, char *cparams0, void *cb, void *obj, char (avDev[3])[128])
{
	int ret = 0;
	//start video
	//set params
	char *params = cparams[0];
	if (cparams0 && strcmp(cparams0, ""))
	{
		params = cparams0;
	}
	int testLen = strlen(params);
	IISetParams((void *)pHandle, params);
	if (cb)
	{
		IISetSndPktCallback((void *)pHandle, cb, obj);
	}
	else
	{
		IISetSndPktCallback((void *)pHandle, (void *)&RtpSendCallBack, this);
	}
	//
	char *dev[20];
	if (avDev && strcmp(avDev[1], ""))
	{
		dev[0] = avDev[1];
	}
	else
	{
		char deviceName[5][255] = {};
		
		for (int j = 0; j < 5; j++)
		{
			dev[j] = (char *)deviceName[j];
		}
		ret = IIGetDevList((void *)pHandle, dev, dev_cam);
	}
	//
	captureID = IIStartPreview((void *)pHandle, wind, dev[0]);// capDev);
    LogOut("captureID = %d = \n",(void *)&captureID);
	//audio
#ifdef _WIN32
	if (avDev && strcmp(avDev[0], ""))
	{
		dev[0] = avDev[0];
	}
	else
	{
		char deviceName[5][255] = {};
		for (int j = 0; j < 5; j++)
		{
			dev[j] = (char *)deviceName[j];
		}
		ret = IIGetDevList((void *)pHandle, dev, dev_recoder);
	}
	//
#endif
	recoderId = IIStartRecoderDev((void *)pHandle, dev[0]);// recoderDev);
    LogOut("recoderId = %d = \n",(void *)&recoderId);
	//
	channelID = IIStartVideo((void *)pHandle);
    LogOut("channelID = %d = \n",(void *)&channelID);
	//start audio
	//set params
	params = cparams[2];
	if (cparams0 && strcmp(cparams0, ""))
	{
		params = cparams0;
	}
	IISetParams((void *)pHandle, params);
	aChannelId = IIStartAudio((void *)pHandle);
	LogOut("aChannelId = %d = \n",(void *)&aChannelId);

	AVGroup group;
	group.GroupId = groupIdx;
	group.StreamId = streamId;
	group.type = kVideo | kAudio | kEncoder;
	group.VEChanId = channelID;
	group.AEChanId = aChannelId;
	//group.VDChanId;
	//group.ADChanId;
	group.CaptureId = captureID;
	IIAddGroup((void *)pHandle, &group);
	groupId[groupIdx] = groupIdx;
	streamID[groupIdx] = streamId;//test
	ret = groupIdx;
	groupIdx++;

	return ret;
}
int AVEngineTest::StartDec(int64_t streamId, char *cparams0, void *cb, void *obj, char (avDev[3])[128])
{
	int ret = 0;
	//winIdx = 1;
	//set params
	char *params = cparams[1];
	if (cparams0 && strcmp(cparams0,""))
	{
		params = cparams0;
	}
	IISetParams((void *)pHandle, params);
//	IISetSndPktCallback((void *)pHandle, (void *)&RtpSendCallBack, this);//encoder and decoder share a same transport object
	if (cb)
	{
	//IISetRawDataCallback(pHandle, (void *)&RawDataOutputCallBack0, this);
		IISetRawDataCallback(pHandle, cb, obj);
	}
	
	dChannelID = IIStartVideoDecoder((void *)pHandle, winIdx, NULL);	winIdx++;
    LogOut("dChannelID = %d \n",(void *)&dChannelID);

	params = cparams[3];
	if (cparams0 && strcmp(cparams0, ""))
	{
		params = cparams0;
	}
	IISetParams((void *)pHandle, params);
    char *dev[20] = {};
#ifdef _WIN32
	if (avDev && strcmp(avDev[0], ""))
	{
		dev[0] = avDev[0];
	}
	else
	{
		char deviceName[5][255] = {};
		for (int j = 0; j < 5; j++)
		{
			dev[j] = (char *)deviceName[j];
		}
		ret = IIGetDevList((void *)pHandle, dev, dev_player);
	}
#endif
	aDChannelId = IIStartAudioDecoder((void *)pHandle, dev[0]);
    LogOut("aDChannelId = %d \n",(void *)&aDChannelId);

	AVGroup group;
	group.GroupId = groupIdx;
	group.StreamId = streamId;
	group.type = kVideo | kAudio | kDecoder;
	//group.VEChanId;
	//group.AEChanId;
	group.VDChanId = dChannelID;
	group.ADChanId = aDChannelId;
	//group.CaptureId;
	IIAddGroup((void *)pHandle, &group);
	groupId[groupIdx] = groupIdx;
	streamID[groupIdx] = streamId;//test
	ret = groupIdx;
	groupIdx++;

	return ret;
}
//===========================================================================================
int IIICreateAVETestObj(void** pTestHnd)
{
    LogOut("IIICreateAVETestObj 0 \n",NULL);
	if (pTestHnd == NULL)
		return 0;
	LogOut("IIICreateAVETestObj 1 \n",NULL);
    void *hnd = (void*)new AVEngineTest;
	//*pTestHnd = (void*)new AVEngineTest;
	LogOut("IIICreateAVETestObj:hnd = %x = \n", (void *)&hnd);
    *pTestHnd = (void*)hnd;
    LogOut("IIICreateAVETestObj:pHandle = %x = \n", (void *)pTestHnd);
	return (int)*pTestHnd;
}
extern FILE *logfp;
int IIIDeleteAVETestObj(void* pTestHnd)
{
	AVEngineTest *avTest = NULL;

	if (pTestHnd == NULL)
		return 0;
	
	avTest = (AVEngineTest*)pTestHnd;
	avTest->Release();
	delete avTest;
	return 1;
}
int IIIPlayAV(void *hnd, void *data, int len, int channel, int avType)
{
	AVEngineTest *avTest = (AVEngineTest *)hnd;
	int ret = 0;
	if (avType == kAudio)
	{
		int chanId = channel < 0 ? 0 : channel;
		ret = IIPlayAudio((void *)(avTest->pHandle), data, len, chanId);
	}
	else
	{
		int chanId = channel < 0 ? 0 : channel;
		ret = IIPlayVideo((void *)(avTest->pHandle), data, len, chanId);
	}
	return ret;
}
void *IIIGetGroup(void *hnd, int type, long long streamId)
{
	AVEngineTest *avTest = (AVEngineTest *)hnd;
	//kAudio = 1, kVideo = 2, kEncoder = 4, kDecoder
	void *ret = IIGetGroup((void *)(avTest->pHandle), type, streamId);
	return ret;
}
int IIIGetDev(void *hnd, char (deviceName[10])[128], int devType)
{
	AVEngineTest *avTest = (AVEngineTest *)hnd;
	char *dev[20] = {};
	for (int j = 0; j < 10; j++)
	{
		dev[j] = (char *)deviceName[j];
	}
	int ret = IIGetDevList((void *)(avTest->pHandle), dev, devType);// dev_recoder);
	return ret;
}
int IITStartAVCodec(void *hnd, long long streamId, void *wind, char *cparams, void *cb, void *obj, char (avDev[3])[128], int EncOrDec)
{
	AVEngineTest *avTest = (AVEngineTest *)hnd;
	int ret = -1;
	if (wind)
	{
		IIAddExtWin((void *)avTest->pHandle, wind);
		avTest->wind = wind;
	}
	if (EncOrDec)
	{
		ret = avTest->StartDec(streamId, cparams, cb, obj, avDev);
	}
	else
	{
		ret = avTest->StartEnc(streamId, cparams, cb, obj, avDev);
	}
	return ret;
}
int IITStopAVCodec(void *hnd, long long streamId, int EncOrDec)
{
	AVEngineTest *avTest = (AVEngineTest *)hnd;
	int ret = -1;
	if (EncOrDec)
	{
		ret = avTest->StopDec(streamId);
	}
	else
	{
		ret = avTest->StopEnc(streamId);
	}
	return ret;
}
//AVEngineTest avTest;
#if 0
int IIAVETestMain(void *wind0, void *wind1)
{
	void *pHandle = NULL;
	int ret = 0;
	long long streamId = 1;

	ret = IIICreateAVETestObj(&pHandle);
	
	ret = IITStartAVCodec(pHandle, streamId, wind1, cparams[1], 1);//dec
	ret = IITStartAVCodec(pHandle, streamId, wind0, cparams[0], 0);//enc
#ifdef _WIN32
	int j = 0, baseCnt = 1000;
	do
	{
		Sleep(100);
		//usleep(2000000);

	} while (j++ < baseCnt);
#endif
	ret = IITStopAVCodec(pHandle, streamId, 1);//dec
	ret = IITStopAVCodec(pHandle, streamId, 0);//dec
	ret = IIIDeleteAVETestObj(pHandle);
#ifdef _WIN32
	j = 0, baseCnt = 100;
	do
	{
		Sleep(100);
		//usleep(2000000);

	} while (j++ < baseCnt);
#endif
	return ret;
}
#elif(1)
int IIAVETestMain(void *wind0, void *wind1)
{
	void *pHandle = NULL;
	int ret = 0;
	long long streamId = 1;

	ret = IIICreateAVETestObj(&pHandle);
#if 1
	char (deviceName[10])[128] = {};
	char (avDev[3])[128] = {};
	int devType = dev_player;//dev_cam = 0, dev_recoder, dev_player
    LogOut(" IIIGetDev \n",NULL);
	ret = IIIGetDev(pHandle, deviceName, devType);//dev_player
	strcpy(avDev[0], deviceName[0]);
    LogOut(" IITStartAVCodec \n",NULL);
	ret = IITStartAVCodec(pHandle, streamId, wind1, cparams[1], NULL, NULL, avDev, 1);//dec
	//
	devType = dev_recoder;
    LogOut(" IIIGetDev2 \n",NULL);
	ret = IIIGetDev(pHandle, deviceName, devType);//dev_recoder
	strcpy(avDev[0], deviceName[0]);
	devType = dev_cam;
    LogOut(" IIIGetDev3 \n",NULL);
	ret = IIIGetDev(pHandle, deviceName, devType);//dev_cam
	strcpy(avDev[0], deviceName[0]);
    LogOut(" IITStartAVCodec2 \n",NULL);
	ret = IITStartAVCodec(pHandle, streamId, wind0, cparams[0], NULL, NULL, avDev, 0);//enc
	//
	int type = kAudio | kDecoder;//kAudio = 1, kVideo = 2, kEncoder = 4, kDecoder
	void *group0 = IIIGetGroup(pHandle, type, streamId);
	type = kVideo | kDecoder;
	void *group1 = IIIGetGroup(pHandle, type, streamId);
	AVGroup *gpAudio = (AVGroup *)group0;
	AVGroup *gpVideo = (AVGroup *)group0;
	///	IIIPlayAV(pHandle, data, len, gpAudio->ADChanId, kAudio);
	///	IIIPlayAV(pHandle, data, len, gpVideo->VDChanId, kVideo);
    LogOut(" IIAVETestMain is ok \n",NULL);
#else
	ret = _pIITStartAVCodec(pHandle, streamId, wind1, cparams[1], NULL, NULL, NULL, 1);//dec
	ret = _pIITStartAVCodec(pHandle, streamId, wind0, cparams[0], NULL, NULL, NULL, 0);//enc

ret = IITStartAVCodec(pHandle, streamId, wind1, cparams[1], NULL, NULL, NULL, 1);//dec
    ret = IITStartAVCodec(pHandle, streamId, wind0, cparams[0], NULL, NULL, NULL, 0);//enc
#endif

#ifdef _WIN32
	int j = 0, baseCnt = 100;
	do
	{
		Sleep(100);
		//usleep(2000000);

	} while (j++ < baseCnt);

	ret = IITStopAVCodec(pHandle, streamId, 1);//dec
	ret = IITStopAVCodec(pHandle, streamId, 0);//dec
	ret = IIIDeleteAVETestObj(pHandle);

	j = 0, baseCnt = 100;
	do
	{
		Sleep(100);
		//usleep(2000000);

	} while (j++ < baseCnt);
#endif
	return ret;
}
#elif(0)
int IIAVETestMain(void *wind0, void *wind1)
{
	AVEngineTest *avTest = new  AVEngineTest;
#ifdef EXT_WIN
	// wind_1 == ?
	LogOut("wind1 = %x = \n", (void *)&wind1);
	IIAddExtWin((void *)avTest->pHandle, wind1);
#endif
	//avTest->Ready();
	avTest->StartDec(1, "", NULL, NULL, NULL);
#ifdef EXT_WIN
	// wind_1 == ?
	LogOut("wind0 = %x = \n", (void *)&wind0);
	IIAddExtWin((void *)avTest->pHandle, wind0);
	avTest->wind = wind0;
#endif
	avTest->StartEnc(1, "", NULL, NULL, NULL);
#ifdef _WIN32
	int j = 0, baseCnt = 1000;
	do
	{
		Sleep(100);
		//usleep(2000000);

	} while (j++ < baseCnt);
	avTest->StopEnc(1);
	avTest->StopDec(1);
	avTest->Release();
#endif
	
	return 0;
}
#else
int IIAVETestMain(void *wind0, void *wind1)
{
	//AVEngineTest *avTest = new  AVEngineTest;
    LogOut(" ready to start avengine \n",NULL);
	int i = 0, runNum = 100;
	do
	{
		AVEngineTest *avTest = new  AVEngineTest;
#ifdef EXT_WIN
		// wind_1 == ?
		LogOut("wind0 = %x = \n", (void *)&wind0);
		IIAddExtWin((void *)avTest->pHandle, wind0);
		LogOut("wind1 = %x = \n", (void *)&wind1);
		IIAddExtWin((void *)avTest->pHandle, wind1);
#endif
		//avTest->Ready();
		avTest->StartDec(1);
		avTest->StartEnc(1);
		//avTest->StartDec();
#if 1
		int j = 0, baseCnt = 10000;
		do
		{
#ifdef _WIN32
			Sleep(100);
#else
			usleep(2000000);
#endif
		} while (j++ < baseCnt);
#endif
		avTest->Release();
		delete avTest;
		i++;
	} while (i < runNum);
    return 0;
}
#endif