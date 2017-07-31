//edited by gxh

//#ifdef MFC_DEMO
//#include "stdafx.h"
//#endif
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "webrtc/avengine/interface/avengAPI.h"
#include "webrtc/modules/av_coding/codecs/ffmpeg/main/interface/ffmpeg_api.h"
#include "webrtc/av-superlogic/api.h"
#ifdef _WIN32
#include <windows.h>
#endif

#define WINWIDTH	640//1280
#define WINHEIGHT	480//720

//#define WINWIDTH	1280
//#define WINHEIGHT	720

//#define DEBUG_FFMPEG

#ifndef MFC_DEMO

//#define LFDEMO

#ifdef LFDEMO
//#include "webrtc/av-superlogic/player_sdk.h"
#include "webrtc/av-superlogic/api.h"
#endif
#endif

#ifdef _WIN32
#include <windows.h>
//#pragma   comment(linker,   "/subsystem:\"windows\"   /entry:\"mainCRTStartup\""   )  // 设置入口地址
#endif 
static char *g_streamId = NULL;


#if 0
static char *cparams[4] =
{
"{\"codecParams\":{\
\"out_flag\":\"8\",\
\"in_data_format\":\"0\",\
\"width\":\"352\",\
\"height\":\"288\",\
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
\"filePath\":\"C:/works/video/lry.flv\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"codecParams\":{\
\"out_flag\":\"0\",\
\"in_data_format\":\"0\",\
\"width\":\"352\",\
\"height\":\"288\",\
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
"{\"codecParams\":{\
\"out_flag\":\"8\",\
\"in_data_format\":\"0\",\
\"width\":\"352\",\
\"height\":\"288\",\
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
\"filePath\":\"C:/works/video/lry.flv\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"codecParams\":{\
\"out_flag\":\"0\",\
\"in_data_format\":\"0\",\
\"width\":\"352\",\
\"height\":\"288\",\
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
rtmp://127.0.0.1/live/live1
http://fwss.xiu.youku.com/live/f/v1/000000000000000000000000153AFFE5?token=98765
http://30.96.180.95:8090/feed1.ffm
http://30.96.176.144:8090/feed1.ffm
#else
static char *cparams[8] =
{
"{\"genParams\":{\
\"debugInfo\":\"1\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"1\",\
\"streamId\":\"1\",\
\"in_data_format\":\"0\",\
\"width\":\"640\",\
\"height\":\"480\",\
\"width1\":\"320\",\
\"height1\":\"240\",\
\"width2\":\"160\",\
\"height2\":\"120\",\
\"gop_size\":\"50\",\
\"mtu_size\":\"1400\",\
\"frame_rate\":\"15\",\
\"min_bits_rate\":\"256000\",\
\"max_bits_rate\":\"1024000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"video_bits_rate\":\"512000\",\
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
\"out_streamname\":\"rtmp://30.96.194.13/live/live1\",\
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
\"streamId\":\"1\",\
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
\"out_filename\":\"\",\
\"out_streamname\":\"\",\
\"sdp_filename\":\"video.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"genParams\":{\
\"debugInfo\":\"4\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"1\",\
\"streamId\":\"1\",\
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
\"streamId\":\"1\",\
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
\"in_filename\":\"\"rtmp://127.0.0.1/live/live-0,\
\"out_filename\":\"\",\
\"out_streamname\":\"\",\
\"sdp_filename\":\"audio.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"genParams\":{\
\"debugInfo\":\"1\",\
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
\"min_bits_rate\":\"256000\",\
\"max_bits_rate\":\"1024000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"video_bits_rate\":\"512000\",\
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
\"streamId\":\"2\",\
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
\"out_filename\":\"\",\
\"out_streamname\":\"\",\
\"sdp_filename\":\"video.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
"
};
#endif

typedef int(*pIIICreateAVETestObj)(void *hnd);
typedef void*(*pIIIGetGroup)(void *hnd, int type, long long streamId);
typedef int(*pIIIGetDev)(void *hnd, char (deviceName[10])[128], int devType);
typedef int(*pIIIGetCapDevList)(void *hnd, char(deviceName[MAX_CAP_LISTS])[128], int capIdx);
typedef int(*pIIIPlayAV)(void *hnd, void *data, int len, int channel, int avType);
typedef int(*pIITStartAVCodec)(void *hnd, long long streamId, void *wind, char *cparams, void *cb, void *obj, char(avDev[3])[128], int EncOrDec, int startImageWidth, int startImageHeight, unsigned char *data);
typedef int(*pIIIAddExtWin)(void *hnd, int windIdx, void *wind, int width, int height, int orgX, int orgY, int showWidth, int showHeight);
typedef void(*pIIISetStretchMode)(void *hnd, int mode);
typedef void(*pIIISetDelayBuffer)(void *hnd, int delay);
typedef int(*pIIISetReceiveDelay)(void *hnd, int video_channel, int buffer_delay_ms);
typedef int(*pIIIRenewUrl)(void *hnd, long long streamId, char *url, int InOrOut, int FileOrStream, int type);
typedef int(*pIITStopAVCodec)(void *hnd, long long streamId, int EncOrDec);
typedef int(*pIIIReleaseStream)(void *hnd, long long streamId, int EncOrDec);
typedef int(*pIIIChangeCam)(void *hnd, long long streamId, void *wind);
typedef int(*pIIIReadParams)(void *hnd, char *iStr, char *oStr, int *oValue, int groupIdx);
typedef int(*pIIIWriteParams)(void *hnd, char *iStr, char *oStr, int oValue, int groupIdx);
typedef int(*pIIISetFec)(void *hnd, int VideoRate, int VideoSeries, int AudioRate, int AudioSeries, int MaxDelay, int TimeThreshold, bool SkipFec);
typedef int(*pIIIDeleteAVETestObj)(void *hnd);
typedef void *(*pICreatJson)(int processId, void **jsObj, char *filename);
typedef void *(*pIStr2Json)(char *text, void *jsonObject);
typedef void(*pIDeleteJson)(void *jsonObject);
typedef int(*pIReadMember)(void *jsonObject, char *path, int *value, char *contex);
typedef int(*pIAddMember)(FILE *fp, void *jsonObject, char *path, int value, char *contex, int flag);
typedef void(*pIJsonRenew)(void *jObjSrc, void *jObjDst);
typedef int(*pISaveJson)(FILE **fp, void **jsonObject);
//
typedef void(*pFF_FACTORY_STOP)(void);
typedef int(*pFF_AVCODEC_CREATE)(void *hnd);
typedef int(*pFF_CODEC_SET_PARAMS)(void *hnd, char *cParams);
typedef int(*pFF_CODEC_INIT)(void *hnd, int codec_flag);
typedef int(*pFF_RTP_PACKAGE)(char *inBuf, int size, char *outBuf, short *rtpSize, int isKeyFrame, int width, int height);
typedef int(*pFF_RTP_UNPACKAGE)(char *inBuf, short *rtpSize, int count, int rtpLen, char *outBuf, int *oSize);
typedef int(*pFF_CODEC_CODEDED)(void *hnd, char * inBuf[3], int len, char *outBuf[3], int oLen, int *pic_type, int codec_flag);
typedef int(*pFF_WRITE_TAIL)(void *hnd, int codec_flag);
typedef int(*pFF_AVCODEC_DELETE)(void *hnd);
typedef void(*pFF_FACTORY_DELETE)(void *hnd);
typedef void(*pFF_GET_TIIME)(char *name);
//
typedef int(*pICreateFecSndObj)(void** pTestHnd, int cmd, long long streamId, void *cb, void *obj);
typedef void(*pISetFec)(void *hnd, long long streamId, int VideoRate, int VideoSeries, int AudioRate, int AudioSeries, int MaxDelay, int TimeThreshold, bool SkipFec);
typedef void(*pISndFecPushData)(void *hnd, char *buf, int len);
typedef int(*pIDeleteFecSndObj)(void* pTestHnd);
//
typedef void(*pRegisterOnErrorCallback)(void *hndId, const void* error_callback);
//
typedef void *(*pICreateMyWind)(int winNum, int width, int height);
typedef void *(*pIGetWind)(void *hnd, int idx);
typedef void (*pIDeleteMyWind)(void *hnd);
//
typedef int(*pFF_TEST_MAIN)(int flag);
typedef int (*pIIAVETestMain)(void *wind0, void *wind1);
//
pIIICreateAVETestObj _pIIICreateAVETestObj = NULL;
pIIIGetGroup _pIIIGetGroup = NULL;
pIIIGetDev _pIIIGetDev = NULL;
pIIIGetCapDevList _pIIIGetCapDevList = NULL;
pIIIPlayAV _pIIIPlayAV = NULL;
pIITStartAVCodec _pIITStartAVCodec = NULL;
pIIIAddExtWin _pIIIAddExtWin = NULL;
pIIISetStretchMode _pIIISetStretchMode = NULL;
pIIISetDelayBuffer _pIIISetDelayBuffer = NULL;
pIIISetReceiveDelay _pIIISetReceiveDelay = NULL;
pIIIRenewUrl _pIIIRenewUrl = NULL;
pIITStopAVCodec _pIITStopAVCodec = NULL;
pIIIReleaseStream _pIIIReleaseStream = NULL;
pIIIChangeCam _pIIIChangeCam = NULL;
pIIIReadParams _pIIIReadParams = NULL;
pIIIWriteParams _pIIIWriteParams = NULL;
pIIISetFec _pIIISetFec = NULL;
pIIIDeleteAVETestObj _pIIIDeleteAVETestObj = NULL;
pFF_FACTORY_STOP _pFF_FACTORY_STOP = NULL;
pFF_AVCODEC_CREATE _pFF_AVCODEC_CREATE = NULL;
pFF_CODEC_SET_PARAMS _pFF_CODEC_SET_PARAMS = NULL;
pFF_CODEC_INIT _pFF_CODEC_INIT = NULL;
pFF_RTP_PACKAGE _pFF_RTP_PACKAGE = NULL;
pFF_RTP_UNPACKAGE _pFF_RTP_UNPACKAGE = NULL;
pFF_CODEC_CODEDED _pFF_CODEC_CODEDED = NULL;
pFF_WRITE_TAIL _pFF_WRITE_TAIL = NULL;
pFF_AVCODEC_DELETE _pFF_AVCODEC_DELETE = NULL;
pFF_FACTORY_DELETE _pFF_FACTORY_DELETE = NULL;
pFF_GET_TIIME _pFF_GET_TIIME = NULL;
pFF_TEST_MAIN _pFF_TEST_MAIN = NULL;
pIIAVETestMain _pIIAVETestMain = NULL;
pICreatJson _pICreatJson = NULL;
pIStr2Json _pIStr2Json = NULL;
pIDeleteJson _pIDeleteJson = NULL;
pIReadMember _pIReadMember = NULL;
pIAddMember _pIAddMember = NULL;
pIJsonRenew _pIJsonRenew = NULL;
pISaveJson _pISaveJson = NULL;
//
pICreateFecSndObj _pICreateFecSndObj = NULL;
pISetFec _pISetFec = NULL;
pISndFecPushData _pISndFecPushData = NULL;
pIDeleteFecSndObj _pIDeleteFecSndObj = NULL;
//
pICreateMyWind _pICreateMyWind = NULL;
pIGetWind _pIGetWind = NULL;
pIDeleteMyWind _pIDeleteMyWind = NULL;
//
pRegisterOnErrorCallback _pRegisterOnErrorCallback = NULL;

#ifdef LFDEMO
/*
typedef int(*pICreatePlayerSdkObj)(void** pHandle, void* hwnd);
typedef int(*pIDeletePlayerSdkObj)(void* pHandle);
typedef void(*pIStartPlayerSdk)(void* pHandle, char *appid, char *streamid, char *token);
*/
typedef int(*pICreatePlayerSdkObj)(void* hwnd, const int width, const int height, const int type, const int delay);
typedef int(*pIDeletePlayerSdkObj)();
typedef void(*pIStartPlayerSdk)(char *appid, char *streamid, char *token, bool is_test);
typedef int(*pIStopPlayerSdk)();


pICreatePlayerSdkObj _pICreatePlayerSdkObj = NULL;
pIDeletePlayerSdkObj _pIDeletePlayerSdkObj = NULL;
pIStartPlayerSdk _pIStartPlayerSdk = NULL;
pIStopPlayerSdk _pIStopPlayerSdk = NULL;
#endif
int LoadDll(char *dllname)
{
	HINSTANCE hAVEDllHandle = NULL;
	char achTemp[256];
	char* pPos = NULL;
	GetModuleFileNameA(NULL, achTemp, sizeof(achTemp));
	pPos = strrchr(achTemp, '\\');
	*++pPos = '\0';
	//strcat(achTemp, "\\");
	strcat(achTemp, dllname);

	hAVEDllHandle = LoadLibraryA(achTemp);
	if (NULL == hAVEDllHandle)
	{
#ifdef _WIN32
		//ShowMessage(SysErrorMessage(GetLastError()));
		DWORD dwErr = ::GetLastError();
		//hAVEDllHandle = LoadLibraryA("D:/webrtc-checkout/src-ninja/build/Debug/avengine_dll.dll");
#endif
		return -1;
	}
	/*_pFF_AVCODEC_CREATE = (pFF_AVCODEC_CREATE)GetProcAddress(hAVEDllHandle, "FF_AVCODEC_CREATE");
	if (!_pFF_AVCODEC_CREATE)
	{
		return -1;
	}*/
	_pFF_FACTORY_STOP = (pFF_FACTORY_STOP)GetProcAddress(hAVEDllHandle, "FF_FACTORY_STOP");
	if (!_pFF_FACTORY_STOP)
	{
		return -1;
	}
	_pFF_CODEC_INIT = (pFF_CODEC_INIT)GetProcAddress(hAVEDllHandle, "FF_CODEC_INIT");
	if (!_pFF_CODEC_INIT)
	{
		return -1;
	}
	_pFF_CODEC_SET_PARAMS = (pFF_CODEC_SET_PARAMS)GetProcAddress(hAVEDllHandle, "FF_CODEC_SET_PARAMS");
	if (!_pFF_CODEC_SET_PARAMS)
	{
		return -1;
	}
	_pFF_RTP_PACKAGE = (pFF_RTP_PACKAGE)GetProcAddress(hAVEDllHandle, "FF_RTP_PACKAGE");
	if (!_pFF_RTP_PACKAGE)
	{
		return -1;
	}
	_pFF_RTP_UNPACKAGE = (pFF_RTP_UNPACKAGE)GetProcAddress(hAVEDllHandle, "FF_RTP_UNPACKAGE");
	if (!_pFF_RTP_UNPACKAGE)
	{
		return -1;
	}
	_pFF_CODEC_CODEDED = (pFF_CODEC_CODEDED)GetProcAddress(hAVEDllHandle, "FF_CODEC_CODEDED");
	if (!_pFF_CODEC_CODEDED)
	{
		return -1;
	}
	_pFF_WRITE_TAIL = (pFF_WRITE_TAIL)GetProcAddress(hAVEDllHandle, "FF_WRITE_TAIL");
	if (!_pFF_WRITE_TAIL)
	{
		return -1;
	}
	_pFF_AVCODEC_DELETE = (pFF_AVCODEC_DELETE)GetProcAddress(hAVEDllHandle, "FF_AVCODEC_DELETE");
	if (!_pFF_AVCODEC_DELETE)
	{
		return -1;
	}
	_pFF_GET_TIIME = (pFF_GET_TIIME)GetProcAddress(hAVEDllHandle, "FF_GET_TIIME");
	if (!_pFF_GET_TIIME)
	{
		return -1;
	}
	_pFF_FACTORY_DELETE = (pFF_FACTORY_DELETE)GetProcAddress(hAVEDllHandle, "FF_FACTORY_DELETE");
	if (!_pFF_FACTORY_DELETE)
	{
		return -1;
	}
	return 1;
}
int LoadAPI(char *dllname)
{
	HINSTANCE hAVEDllHandle = NULL;
	char achTemp[256];
	char* pPos = NULL;
	GetModuleFileNameA(NULL, achTemp, sizeof(achTemp));
	pPos = strrchr(achTemp, '\\');
	*++pPos = '\0';
	//strcat(achTemp, "\\");
	strcat(achTemp, dllname);

	hAVEDllHandle = LoadLibraryA(achTemp);
	if (NULL == hAVEDllHandle)
	{
#ifdef _WIN32
		//ShowMessage(SysErrorMessage(GetLastError()));
		DWORD dwErr = ::GetLastError();
		//hAVEDllHandle = LoadLibraryA("D:/webrtc-checkout/src-ninja/build/Debug/avengine_dll.dll");
#endif
		return -1;
	}
	_pIIICreateAVETestObj = (pIIICreateAVETestObj)GetProcAddress(hAVEDllHandle, "IIICreateAVETestObj");
	if (!_pIIICreateAVETestObj)
	{
		return -1;
	}
#if TEST_TRANS
	_pIIAddTrans = (pIIAddTrans)GetProcAddress(hAVEDllHandle, "IIAddTrans");
	if (!_pIIAddTrans)
	{
		return -1;
	}
	_pIISubTrans = (pIISubTrans)GetProcAddress(hAVEDllHandle, "IISubTrans");
	if (!_pIISubTrans)
	{
		return -1;
	}
	_pIIGetTrans = (pIIGetTrans)GetProcAddress(hAVEDllHandle, "IIGetTrans");
	if (!_pIIGetTrans)
	{
		return -1;
	}
	_pIITransRelease = (pIITransRelease)GetProcAddress(hAVEDllHandle, "IITransRelease");
	if (!_pIITransRelease)
	{
		return -1;
	}
#endif
	_pIIIGetGroup = (pIIIGetGroup)GetProcAddress(hAVEDllHandle, "IIIGetGroup");
	if (!_pIIIGetGroup)
	{
		return -1;
	}
	_pIIIGetDev = (pIIIGetDev)GetProcAddress(hAVEDllHandle, "IIIGetDev");
	if (!_pIIIGetDev)
	{
		return -1;
	}
	_pIIIGetCapDevList = (pIIIGetCapDevList)GetProcAddress(hAVEDllHandle, "IIIGetCapDevList");
	if (!_pIIIGetCapDevList)
	{
		return -1;
	}
	_pIIIPlayAV = (pIIIPlayAV)GetProcAddress(hAVEDllHandle, "IIIPlayAV");
	if (!_pIIIPlayAV)
	{
		return -1;
	}
	_pIIIChangeCam = (pIIIChangeCam)GetProcAddress(hAVEDllHandle, "IIIChangeCam");
	if (!_pIIIChangeCam)
	{
		printf("IIIChangeCam error \n");
		return -1;
	}
	_pIIIReadParams = (pIIIReadParams)GetProcAddress(hAVEDllHandle, "IIIReadParams");
	if (!_pIIIReadParams)
	{
		printf("IIIReadParams error \n");
		return -1;
	}
	_pIIIWriteParams = (pIIIWriteParams)GetProcAddress(hAVEDllHandle, "IIIWriteParams");
	if (!_pIIIWriteParams)
	{
		printf("IIIWriteParams error \n");
		return -1;
	}
	_pIIISetFec = (pIIISetFec)GetProcAddress(hAVEDllHandle, "IIISetFec");
	if (!_pIIISetFec)
	{
		printf("IIISetFec error \n");
		return -1;
	}
	_pIITStartAVCodec = (pIITStartAVCodec)GetProcAddress(hAVEDllHandle, "IITStartAVCodec");
	if (!_pIITStartAVCodec)
	{
		return -1;
	}
	_pIIIAddExtWin = (pIIIAddExtWin)GetProcAddress(hAVEDllHandle, "IIIAddExtWin");
	if (!_pIIIAddExtWin)
	{
		return -1;
	}
	_pIIISetStretchMode = (pIIISetStretchMode)GetProcAddress(hAVEDllHandle, "IIISetStretchMode");
	if (!_pIIISetStretchMode)
	{
		return -1;
	}
	_pIIISetDelayBuffer = (pIIISetDelayBuffer)GetProcAddress(hAVEDllHandle, "IIISetDelayBuffer");
	if (!_pIIISetDelayBuffer)
	{
		return -1;
	}
	_pIIISetReceiveDelay = (pIIISetReceiveDelay)GetProcAddress(hAVEDllHandle, "IIISetReceiveDelay");
	if (!_pIIISetReceiveDelay)
	{
		return -1;
	}
	_pIIIRenewUrl = (pIIIRenewUrl)GetProcAddress(hAVEDllHandle, "IIIRenewUrl");
	if (!_pIIIRenewUrl)
	{
		return -1;
	}
	_pIITStopAVCodec = (pIITStopAVCodec)GetProcAddress(hAVEDllHandle, "IITStopAVCodec");
	if (!_pIITStopAVCodec)
	{
		return -1;
	}
	_pIIIReleaseStream = (pIIIReleaseStream)GetProcAddress(hAVEDllHandle, "IIIReleaseStream");
	if (!_pIIIReleaseStream)
	{
		return -1;
	}
	_pICreatJson = (pICreatJson)GetProcAddress(hAVEDllHandle, "ICreatJson");
	if (!_pICreatJson)
	{
		return -1;
	}
	_pIStr2Json = (pIStr2Json)GetProcAddress(hAVEDllHandle, "IStr2Json");
	if (!_pIStr2Json)
	{
		return -1;
	}
	_pIDeleteJson = (pIDeleteJson)GetProcAddress(hAVEDllHandle, "IDeleteJson");
	if (!_pIDeleteJson)
	{
		return -1;
	}
	_pIReadMember = (pIReadMember)GetProcAddress(hAVEDllHandle, "IReadMember");
	if (!_pIReadMember)
	{
		return -1;
	}
	_pIAddMember = (pIAddMember)GetProcAddress(hAVEDllHandle, "IAddMember");
	if (!_pIAddMember)
	{
		return -1;
	}
	_pIJsonRenew = (pIJsonRenew)GetProcAddress(hAVEDllHandle, "IJsonRenew");
	if (!_pIJsonRenew)
	{
		return -1;
	}
	_pISaveJson = (pISaveJson)GetProcAddress(hAVEDllHandle, "ISaveJson");
	if (!_pISaveJson)
	{
		return -1;
	}
	_pIIIDeleteAVETestObj = (pIIIDeleteAVETestObj)GetProcAddress(hAVEDllHandle, "IIIDeleteAVETestObj");
	if (!_pIIIDeleteAVETestObj)
	{
		return -1;
	}

	
#ifndef DEBUG_FFMPEG
#if 0
	_pFF_AVCODEC_CREATE = (pFF_AVCODEC_CREATE)GetProcAddress(hAVEDllHandle, "FF_AVCODEC_CREATE");
	if (!_pFF_AVCODEC_CREATE)
	{
		return -1;
	}
	//
	_pFF_FACTORY_STOP = (pFF_FACTORY_STOP)GetProcAddress(hAVEDllHandle, "FF_FACTORY_STOP");
	if (!_pFF_FACTORY_STOP)
	{
		return -1;
	}
	_pFF_CODEC_INIT = (pFF_CODEC_INIT)GetProcAddress(hAVEDllHandle, "FF_CODEC_INIT");
	if (!_pFF_CODEC_INIT)
	{
		return -1;
	}
	_pFF_CODEC_SET_PARAMS = (pFF_CODEC_SET_PARAMS)GetProcAddress(hAVEDllHandle, "FF_CODEC_SET_PARAMS");
	if (!_pFF_CODEC_SET_PARAMS)
	{
		return -1;
	}
	_pFF_RTP_PACKAGE = (pFF_RTP_PACKAGE)GetProcAddress(hAVEDllHandle, "FF_RTP_PACKAGE");
	if (!_pFF_RTP_PACKAGE)
	{
		return -1;
	}
	_pFF_RTP_UNPACKAGE = (pFF_RTP_UNPACKAGE)GetProcAddress(hAVEDllHandle, "FF_RTP_UNPACKAGE");
	if (!_pFF_RTP_UNPACKAGE)
	{
		return -1;
	}
	_pFF_CODEC_CODEDED = (pFF_CODEC_CODEDED)GetProcAddress(hAVEDllHandle, "FF_CODEC_CODEDED");
	if (!_pFF_CODEC_CODEDED)
	{
		return -1;
	}
	_pFF_WRITE_TAIL = (pFF_WRITE_TAIL)GetProcAddress(hAVEDllHandle, "FF_WRITE_TAIL");
	if (!_pFF_WRITE_TAIL)
	{
		return -1;
	}
	_pFF_AVCODEC_DELETE = (pFF_AVCODEC_DELETE)GetProcAddress(hAVEDllHandle, "FF_AVCODEC_DELETE");
	if (!_pFF_AVCODEC_DELETE)
	{
		return -1;
	}
	_pFF_GET_TIIME = (pFF_GET_TIIME)GetProcAddress(hAVEDllHandle, "FF_GET_TIIME");
	if (!_pFF_GET_TIIME)
	{
		return -1;
	}
	_pFF_FACTORY_DELETE = (pFF_FACTORY_DELETE)GetProcAddress(hAVEDllHandle, "FF_FACTORY_DELETE");
	if (!_pFF_FACTORY_DELETE)
	{
		return -1;
	}
#endif
#endif
	//
	_pICreateFecSndObj = (pICreateFecSndObj)GetProcAddress(hAVEDllHandle, "ICreateFecSndObj");
	if (!_pICreateFecSndObj)
	{
		return -1;
	}
	_pISetFec = (pISetFec)GetProcAddress(hAVEDllHandle, "ISetFec");
	if (!_pISetFec)
	{
		return -1;
	}
	_pISndFecPushData = (pISndFecPushData)GetProcAddress(hAVEDllHandle, "ISndFecPushData");
	if (!_pISndFecPushData)
	{
		return -1;
	}
	
	_pIDeleteFecSndObj = (pIDeleteFecSndObj)GetProcAddress(hAVEDllHandle, "IDeleteFecSndObj");
	if (!_pIDeleteFecSndObj)
	{
		return -1;
	}
	//
	_pICreateMyWind = (pICreateMyWind)GetProcAddress(hAVEDllHandle, "ICreateMyWind");
	if (!_pICreateMyWind)
	{
		return -1;
	}
	_pIGetWind = (pIGetWind)GetProcAddress(hAVEDllHandle, "IGetWind");
	if (!_pIGetWind)
	{
		return -1;
	}
	_pIDeleteMyWind = (pIDeleteMyWind)GetProcAddress(hAVEDllHandle, "IDeleteMyWind");
	if (!_pIDeleteMyWind)
	{
		return -1;
	}
	//
	_pIIAVETestMain = (pIIAVETestMain)GetProcAddress(hAVEDllHandle, "IIAVETestMain");
	if (!_pIIAVETestMain)
	{
		return -1;
	}
	
	/*_pFF_TEST_MAIN = (pFF_TEST_MAIN)GetProcAddress(hAVEDllHandle, "FF_TEST_MAIN");
	if (!_pFF_TEST_MAIN)
	{
		return -1;
	}*/
	
#ifdef LFDEMO
	_pRegisterOnErrorCallback = (pRegisterOnErrorCallback)GetProcAddress(hAVEDllHandle, "RegisterOnErrorCallback");
	if (!_pRegisterOnErrorCallback)
	{
		return -1;
	}
	//_pICreatePlayerSdkObj = (pICreatePlayerSdkObj)GetProcAddress(hAVEDllHandle, "ICreatePlayerSdkObj");
	_pICreatePlayerSdkObj = (pICreatePlayerSdkObj)GetProcAddress(hAVEDllHandle, "CreatePlayer");
	if (!_pICreatePlayerSdkObj)
	{
		return -1;
	}
	//_pIDeletePlayerSdkObj = (pIDeletePlayerSdkObj)GetProcAddress(hAVEDllHandle, "IDeletePlayerSdkObj");
	_pIDeletePlayerSdkObj = (pIDeletePlayerSdkObj)GetProcAddress(hAVEDllHandle, "DestroyPlayer");
	if (!_pIDeletePlayerSdkObj)
	{
		return -1;
	}
	//_pIStartPlayerSdk = (pIStartPlayerSdk)GetProcAddress(hAVEDllHandle, "IStartPlayerSdk");
	_pIStartPlayerSdk = (pIStartPlayerSdk)GetProcAddress(hAVEDllHandle, "StartPlay");
	if (!_pIStartPlayerSdk)
	{
		return -1;
	}
	_pIStopPlayerSdk = (pIStopPlayerSdk)GetProcAddress(hAVEDllHandle, "StopPlay");
	if (!_pIStopPlayerSdk)
	{
		return -1;
	}
#endif

	return 0;
}
#if 1

//extern FILE *logfp;
int FF_TEST_MAIN2(int flag)
{
	int ret = 0;
	//
	//testJsonStr();
	//
	printf("start test ffmpeg codec \n");
	void *pVEnc = NULL;
	void *pVDec = NULL;
	void *pAEnc = NULL;
	void *pADec = NULL;
	int hndIdx[4] = { -1, -1, -1, -1 };
	char *vInBuf[3];//
	char *vOutBuf[3];
	char *inBuf[3];//
	char *outBuf[3];
	int vInLen = (704 * 576 * 3) >> 1;
	int vOutLen = 512 * 1024;
	int aInLen = 1024 << 3;
	int aOutLen = 1024;
	int vEncType = kVideoEncStream;
	int vDecType = kVideoDecRaw;
	int encType = kAudioEncStream;
	int decType = kAudioDecRaw;
	//
	short rtpSize[100] = {};
	int frame_type = 0;
	DWORD time0 = 0;
	DWORD time1 = 0;
	DWORD time2 = 0;
	DWORD time3 = 0;
	DWORD time4 = 0;
	int diff = 0;
	//
	int w = 352;//704;
	int h = 288;//576;
	char *yuv0 = new char[(w * h * 3) >> 1];
	char *yuv1 = new char[(w * h * 3) >> 1];
	vInBuf[0] = yuv0;//new char [(704 * 576)];//
	vInBuf[1] = &yuv0[w * h];//new char [(704 * 576) >> 1];//
	vInBuf[2] = &vInBuf[1][(w * h) >> 2];//new char [(704 * 576) >> 1];//
	vOutBuf[0] = yuv1;//
	vOutBuf[1] = &yuv1[w * h];
	vOutBuf[2] = &vOutBuf[1][(w * h) >> 2];

	inBuf[0] = new char[(1024) << 4];//
	outBuf[0] = new char[(1024) << 4];
	
	hndIdx[0] = _pFF_AVCODEC_CREATE(&pVEnc);
	char *params = cparams[0];
	_pFF_CODEC_SET_PARAMS(pVEnc, params);

	hndIdx[1] = _pFF_AVCODEC_CREATE(&pVDec);
	params = cparams[1];
	_pFF_CODEC_SET_PARAMS(pVDec, params);

	hndIdx[2] = _pFF_AVCODEC_CREATE(&pAEnc);
	params = cparams[2];
	_pFF_CODEC_SET_PARAMS(pAEnc, params);

	hndIdx[3] = _pFF_AVCODEC_CREATE(&pADec);
	params = cparams[3];
	_pFF_CODEC_SET_PARAMS(pADec, params);
	//
	ret = _pFF_CODEC_INIT(pVEnc, vEncType);
	ret = _pFF_CODEC_INIT(pAEnc, encType);
	ret = _pFF_CODEC_INIT(pVDec, vDecType);
	ret = _pFF_CODEC_INIT(pADec, decType);


	for (int i = 0; i < 300; i++)
	{
		//if(i)
		{
			//			time0 = timeGetTime();
			ret = _pFF_CODEC_CODEDED(pVEnc, vInBuf, vInLen, vOutBuf, vOutLen, &frame_type, vEncType);//
			///ret = _pFF_RTP_PACKAGE(vOutBuf[0], ret, vInBuf[0], rtpSize);
			
			int oSize = 0;
			ret = _pFF_RTP_UNPACKAGE(vInBuf[0], rtpSize, ret, 0, vOutBuf[0], &oSize);
			if (ret)
			{
				ret = _pFF_CODEC_CODEDED(pVDec, vOutBuf, ret, vInBuf, vInLen, &frame_type, vDecType);
				
			}
			
		}
		ret = _pFF_CODEC_CODEDED(pAEnc, NULL, aInLen, outBuf, aOutLen, &frame_type, encType);
		
		if (ret)
		{
			ret = _pFF_CODEC_CODEDED(pADec, outBuf, ret, inBuf, aInLen, &frame_type, decType);
			
			if (ret < 0)
				exit(1);
		}
		
	}
	_pFF_WRITE_TAIL(pVEnc, vEncType);
	_pFF_WRITE_TAIL(pAEnc, encType);
	
#if 0
	for (int k = 0; k < 4; k++)
	{
		if (hndIdx[k] >= 0)
		{
			ret = _pFF_AVCODEC_DELETE((void *)&(hndIdx[k]));
		}
	}
	//
	_pFF_FACTORY_DELETE(NULL);
#endif
	/*if (logfp)
	{
		fclose(logfp);
	}
	logfp = NULL;*/
	//LogClose();
	//
	return ret;
}
#if 1
FILE *fp = NULL;
int failcount = 0;
int highthreld = 0;
long long lasttickcount = 0;
bool isready = false;
void TransTestInit()
{
	if (fp)
	{
		return;
	}

	char achTemp[MAX_PATH];
	char path[MAX_PATH];
	char* pPos = NULL;
	char cLogId[128] = "";
#ifdef _WIN32
	_pFF_GET_TIIME(cLogId);
#else
	FF_GET_TIIME(cLogTime);
#endif
#ifdef _WIN32
	GetModuleFileNameA(NULL, achTemp, sizeof(achTemp));
	pPos = strrchr(achTemp, '\\');
	*pPos = '\0';
	strcpy(path, achTemp);
	strcat(path, "\\log\\translog");
	strcat(path, cLogId);
#elif defined(__ANDROID__)
	strcpy(path, "/sdcard/translog");
	strcat(path, cLogId);
#elif defined(WEBRTC_IOS) || defined(WEBRTC_MAC) || defined(IOS)
	char dirName[256] = "translog";
	strcat(dirName, cLogId);
	char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
	strcpy(path, pName);
	free(pName);
#else
	strcpy(path, "translog");
	strcat(path, cLogId);
#endif
	char ctimstamp[128] = "";

	strcat(path, ".txt");

	fp = fopen(path, "wb");
	if (fp)
	{
		fputs("start trans log: \n", fp);	 fflush(fp);
	}
}
#endif
static void OnErrorCallback(void *hndId, unsigned int err_code)
{
#if 1
	{
		long long cur = GetTickCount64();
		long long step = cur - lasttickcount;
		if (!fp)
		{
			TransTestInit();
		}
		if (err_code == 1200) 
		{
			isready = true;
		}
		else if (err_code == 1201 && isready) 
		{
			failcount++;
			if (fp)
			{
				char cLogTime[128] = "";
#ifdef _WIN32
				_pFF_GET_TIIME(cLogTime);
#else
				FF_GET_TIIME(cLogTime);
#endif
				fprintf(fp, "date:%s \t failcount= %d \n", cLogTime,failcount);	fflush(fp);
			}
			std::cout << "-----------------:" << failcount << std::endl;
		}
		else 
		{
			//status = true;
		}
		if (step > 1000 && isready)
		{
			highthreld++;
			if (fp)
			{
				char cLogTime[128] = "";
#ifdef _WIN32
				_pFF_GET_TIIME(cLogTime);
#else
				FF_GET_TIIME(cLogTime);
#endif
				fprintf(fp, "date:%s \t\t\t\t highthreld= %d \n", cLogTime, highthreld);	fflush(fp);
			}
			std::cout << "&&&&&&&&&55555555555555555555555:" << step << std::endl;
		}
		lasttickcount = cur;
	}
#endif
	//printf("gxh err_code= %d #############################################\n", err_code);
}
#ifdef LFDEMO
static int RtpUpload(void *hnd, int cmd, int channel, char *buf, int len)
{
	printf("");
	return 0;
}
#if 0
void DeleteJson(void *jsonObject)
{
	_pIDeleteJson(jsonObject);
}
int ReadMember(void *jsonObject, char *path, int *value, char *contex)
{
	return _pIReadMember(jsonObject, path, value, contex);
}
int AddMember(FILE *fp, void *jsonObject, char *path, int value, char *contex, int flag)
{
	return _pIAddMember(fp, jsonObject, path, value, contex, flag);
}
int SaveJson(FILE **fp, void **jsonObject)
{
	return _pISaveJson(fp, jsonObject);
}
void *openJSFile(int processId, void **jsonHnd, void **m_fp)
{
	FILE *fp = NULL;
	//void *jsonHnd = NULL;
	/*if (loadFlag < 0)
	{
		loadFlag = LoadAPI("avengine_dll.dll");
	}*/
	//fp = fopen("MultStreamConfig.txt", "rb+");//plane_704x576
	//if (!fp)
	//{
	//	fp = fopen("MultStreamConfig.txt", "ab+");//plane_704x576
	//fputs("start gxh log: \n", fp);	 fflush(fp);
	//		jsonHnd = _pICreatJson((void *)fp);
	*m_fp = _pICreatJson(processId, jsonHnd, "lfserver");
	SaveJson((FILE **)m_fp, jsonHnd);
	//IAddMember(fp, (void *)&root, "0.codecParams.noexit1.noexit2", 6, NULL, 1);
	//IAddMember(fp, (void *)&root, "0.codecParams.noexit1.noexit3", 6, NULL, 1);
	/*if (fp)
	{
	fclose(fp);
	}*/
	//}
	//	return jsonHnd;
	return (void *)*m_fp;
}
void WriteJS(void *m_jsonHnd)
{
	char cIP[32] = "103.41.143.104";
	int port = 8080;
	char cUrl[128] = "http://103.41.143.104:8080/download/sdp/%s.sdp?token=98765";
	char cToke[32] = "98765";

	
	AddMember((FILE *)NULL, m_jsonHnd, "base.serverIp", NULL, cIP, 0);
	AddMember((FILE *)NULL, m_jsonHnd, "base.serverPort", port, NULL, 0);
	AddMember((FILE *)NULL, m_jsonHnd, "base.url", NULL, cUrl, 0);
	AddMember((FILE *)NULL, m_jsonHnd, "base.toke", NULL, cToke, 0);
}
void ReadJS(void **m_jsonHnd, void **m_fp)
{
	char cIP[32] = "103.41.143.104";
	int port = 8080;
	char cUrl[128] = "http://103.41.143.104:8080/download/sdp/%s.sdp?token=98765";
	char cToke[32] = "98765";

	int processId = 0;
	//*m_fp = (FILE *)
	openJSFile(processId, m_jsonHnd, m_fp);
	ReadMember(m_jsonHnd, "base.serverIp", NULL, g_cIP);
	ReadMember(m_jsonHnd, "base.serverPort", &g_port, NULL);
	ReadMember(m_jsonHnd, "base.url", NULL, g_cUrl);
	ReadMember(m_jsonHnd, "base.toke", NULL, g_cToke);
	
}
#endif
int lfDemo(void *wind0, void *wind1)
{
	void *g_player = NULL;
	void *m_jsonHnd = NULL;
	void *m_fp = NULL;
	//int ret = _pICreatePlayerSdkObj(&g_player, wind1);// LoadAPI("avengine_dll.dll");
	int ret = _pICreatePlayerSdkObj(wind1, 640, 480, 1, 0);
	_pRegisterOnErrorCallback(NULL, (void*)&OnErrorCallback);
	char *app_id = "101";
	//char *stream_id = "siyu";// 
	//char *stream_id = "stream_alias_301789501_999999";//90158777_
	//char *stream_id = "stream_alias_90158777_399976";//
	//char *stream_id = "stream_alias_380668442_98154986";//
	char *stream_id = "stream_alias_380668271_5519";
	//char *stream_id = "stream_alias_380668445_98154990";
	//char *stream_id = "stream_alias_380668301_98154981";// "stream_alias_90158777_399976";//"stream_alias_380668271_5519";// "stream_alias_90158777_399976";// "stream_alias_380668445_98154990";// "stream_alias_301787865_520";
	char *token_id = "98765";
	bool is_test = true;// false;
	if (g_streamId)
	{
		stream_id = g_streamId;
	}
	//ReadJS(&m_jsonHnd, &m_fp);
	//_pIStartPlayerSdk(g_player, app_id, stream_id, token_id);
#if 0
	void *audioBank = NULL;
	int cmd = kAudio;
	long long streamId = 0;
	_pICreateFecSndObj((void**) &audioBank, cmd, streamId, (void *)&RtpUpload, NULL);
	_pISetFec(audioBank, streamId, 4, 4, 3, 2, 500, 80, false);
	void *videoBank = NULL;
	cmd = kVideo;
	_pICreateFecSndObj((void**) &videoBank, cmd, streamId, (void *)&RtpUpload, NULL);
	_pISetFec(videoBank, streamId, 4, 4, 3, 2, 500, 80, false);
	char buf[1500] = "";
	int size = 100;
	_pISndFecPushData(videoBank, buf, size);
	_pISndFecPushData(audioBank, buf, size);
	Sleep(1000000);
#endif
	_pIStartPlayerSdk(app_id, stream_id, token_id, is_test);
#ifdef _WIN32
	int j = 0, baseCnt = 1000000;// 100;// 00;// 10000;
	do
	{
		Sleep(100);
		//usleep(2000000);

	} while (j++ < baseCnt);
#endif
	//ret = _pIDeletePlayerSdkObj(g_player);
	_pIStopPlayerSdk();
	ret = _pIDeletePlayerSdkObj();
	g_player = NULL;
	return ret;
}
#endif
void RenewParams(char *srcParams, char *dstParams)
{
	char newParams0[1024] = "";
	//char newParams1[1024] = "";
	char *newParams1 = dstParams;
	strcpy(newParams0, srcParams);
	strcpy(newParams1, srcParams);
	const char * str1 = newParams0;// cparams[1];
	const char * str2 = "out_flag";
	char *pos = (char *)strstr(str1, str2);
	//char cId[16] = "";
	//sprintf(cId, "%lld", streamId);
	//char newText[128] = "\"streamId\":\"";
	//strcat(newText, cId);
	//strcat(newText, "\",\"in_filename\":\"\",");
	char newText[128] = "\"in_filename\":\"";
	//rtmp://127.0.0.1/live/live-0
	//http://fwss.xiu.youku.com/live/f/v1/000000000000000000000000153AFFE5?token=98765
	//http://fwss.xiu.youku.com/live/f/v1/000000000000000000000000155A47F8?token=98765
	char newTexta[128] = "rtmp://30.96.194.13/live/live1-0";
	///char newTexta[128] = "http://fwss.xiu.youku.com/live/f/v1/000000000000000000000000155DEF6E?token=98765";
	//char newTexta[128] = "http://30.96.180.95:8090/file.flv";
	//char newTexta[128] = "http://30.96.176.97:8090/stream.flv";

	strcat(newText, newTexta);
	strcat(newText, "\",");
	/*
	char newTexta[128] = "";
	sprintf(newTexta, "\"width\":\"%d\",\"height\":\"%d\",", winWidth, winHeight);
	strcat(newText,newTexta);
	*/
	int offset = (int)(pos - str1);
	strcpy(&newParams1[offset - 1], newText);
	strcpy(&newParams1[offset - 1 + strlen(newText)], &pos[-1]);
}
int IIAVETestMain2(void *wind0, void *wind1)
{
	void *pHandle = NULL;
	int ret = 0;
	int EncGroupIdx = -1;
	int DecGroupIdx = -1;
	int EncWinIdx = -1;
	int DecWinIdx = -1;
	long long streamId = 1;
#ifdef LFDEMO
	ret = lfDemo(wind0, wind1);
	return ret;
#endif
	ret = _pIIICreateAVETestObj(&pHandle);
	//_pRegisterOnErrorCallback(NULL, (void*)&OnErrorCallback);

	//_pIIISetFec(pHandle, 4, 4, 3, 2, 500, 80, true);
//	_pIIISetFec(pHandle, 4, 4, 3, 2, 500, 80, false);

	char(deviceName[MAX_CAP_LISTS])[128] = {};
	char (avDev[3])[128] = {};
	int devType = dev_player;//dev_cam = 0, dev_recoder, dev_player
	int winIdx = -1;
	ret = _pIIIGetCapDevList(pHandle, deviceName, -1);
#if 0
	ret = _pIIIGetDev(pHandle, deviceName, devType);//dev_player
	strcpy(avDev[dev_player], deviceName[0]);
	winIdx = _pIIIAddExtWin(pHandle, -1, wind1, 640, 480, 0, 0, 320, 240);
	_pIIISetStretchMode(pHandle, kStretchToInsideEdge);
	//ret = _pIITStartAVCodec(pHandle, streamId, wind1, cparams[1], NULL, NULL, avDev, kDec);//dec
	ret = _pIITStartAVCodec(pHandle, streamId, (void *)&winIdx, cparams[1], NULL, NULL, avDev, kDecoder);//dec
#endif

#if 0
	_pIIISetStretchMode(pHandle, kStretchToInsideEdge);
	devType = dev_recoder;
	ret = _pIIIGetDev(pHandle, deviceName, devType);//dev_recoder
	strcpy(avDev[dev_recoder], deviceName[0]);
	devType = dev_cam;
	ret = _pIIIGetDev(pHandle, deviceName, devType);//dev_cam
	strcpy(avDev[dev_cam], deviceName[0]);//
	//strcpy(avDev[dev_cam], deviceName[1]);//
	//strcpy(avDev[dev_cam], deviceName[2]);//
	EncWinIdx = winIdx = _pIIIAddExtWin(pHandle, -1, wind0, WINWIDTH, WINHEIGHT, WINWIDTH / 2, WINHEIGHT / 2, WINWIDTH / 2, WINHEIGHT / 2);
	int localIdx = winIdx;
	//ret = _pIITStartAVCodec(pHandle, streamId, wind0, cparams[0], NULL, NULL, avDev, kEnc);//enc
	EncGroupIdx = _pIITStartAVCodec(pHandle, streamId, (void *)&winIdx, cparams[0], NULL, NULL, avDev, kEncoder);//enc
#endif
	//Sleep(1000);
#if 1
	ret = _pIIIGetDev(pHandle, deviceName, devType);//dev_player
	strcpy(avDev[dev_player], deviceName[0]);
	DecWinIdx = winIdx = _pIIIAddExtWin(pHandle, -1, wind0, WINWIDTH, WINHEIGHT, 0, 0, WINWIDTH / 2, WINHEIGHT / 2);
	_pIIISetStretchMode(pHandle, kStretchToInsideEdge);
	//ret = _pIITStartAVCodec(pHandle, streamId, wind1, cparams[1], NULL, NULL, avDev, kDec);//dec
	char newParams1[1024] = "";
	RenewParams(cparams[1], newParams1);
	DecGroupIdx = _pIITStartAVCodec(pHandle, streamId, (void *)&winIdx, newParams1, NULL, NULL, avDev, kDecoder,0,0,NULL);//dec
#endif
	//
	int type = kAudio | kDecoder;//kAudio = 1, kVideo = 2, kEncoder = 4, kDecoder
	void *group0 = _pIIIGetGroup(pHandle, type, streamId);
	type = kVideo | kDecoder;
	void *group1 = _pIIIGetGroup(pHandle, type, streamId);
	AVGroup *gpAudio = (AVGroup *)group0;
	AVGroup *gpVideo = (AVGroup *)group0;
#if 1
	//Sleep(4000);
	streamId = 2;
	ret = _pIIIGetDev(pHandle, deviceName, devType);//dev_player
	strcpy(avDev[dev_player], deviceName[0]);
	//winIdx = _pIIIAddExtWin(pHandle, -1, wind0, WINWIDTH, WINHEIGHT, WINWIDTH / 2, 0, WINWIDTH / 2, WINHEIGHT / 2);
	winIdx = _pIIIAddExtWin(pHandle, -1, wind1, WINWIDTH, WINHEIGHT, WINWIDTH / 2, 0, WINWIDTH / 2, WINHEIGHT / 2);
	_pIIISetStretchMode(pHandle, kStretchToInsideEdge);
	//ret = _pIITStartAVCodec(pHandle, streamId, wind1, cparams[1], NULL, NULL, avDev, kDec);//dec
	char newParams2[1024] = "";
	RenewParams(cparams[5], newParams2);
	ret = _pIITStartAVCodec(pHandle, streamId, (void *)&winIdx, newParams2, NULL, NULL, avDev, kDecoder,0,0,NULL);//dec
#endif
#if 0
	streamId = 1;
	_pIIISetStretchMode(pHandle, kStretchToInsideEdge);
	devType = dev_recoder;
	ret = _pIIIGetDev(pHandle, deviceName, devType);//dev_recoder
	strcpy(avDev[dev_recoder], deviceName[0]);
	devType = dev_cam;
	ret = _pIIIGetDev(pHandle, deviceName, devType);//dev_cam
	strcpy(avDev[dev_cam], deviceName[0]);//
	//strcpy(avDev[dev_cam], deviceName[1]);//
	EncWinIdx = winIdx = _pIIIAddExtWin(pHandle, -1, wind0, WINWIDTH, WINHEIGHT, WINWIDTH / 2, WINHEIGHT / 2, WINWIDTH / 2, WINHEIGHT / 2);
	int localIdx = winIdx;
	//ret = _pIITStartAVCodec(pHandle, streamId, wind0, cparams[0], NULL, NULL, avDev, kEnc);//enc
	EncGroupIdx = _pIITStartAVCodec(pHandle, streamId, (void *)&winIdx, cparams[0], NULL, NULL, avDev, kEncoder);//enc
#endif
///	IIIPlayAV(pHandle, data, len, gpAudio->ADChanId, kAudio);
///	IIIPlayAV(pHandle, data, len, gpVideo->VDChanId, kVideo);
	Sleep(20000);
	//Sleep(10000000);
	//winIdx = _pIIIAddExtWin(pHandle, winIdx, wind1, 1280, 720, 0, 0, 1280 / 2, 720 / 2);
	//winIdx = _pIIIAddExtWin(pHandle, DecWinIdx, wind1, WINWIDTH, WINHEIGHT, 0, WINHEIGHT / 2, WINWIDTH / 2, WINHEIGHT / 2);
//	winIdx = _pIIIAddExtWin(pHandle, DecWinIdx, wind0, 1920/2, 1080/2, 0, 1080 / 4, 1920 / 4, 1080 / 4);
	//winIdx = _pIIIAddExtWin(pHandle, EncWinIdx, wind1, WINWIDTH, WINHEIGHT, 0, WINHEIGHT / 2, WINWIDTH / 2, WINHEIGHT / 2);
//	Sleep(5000000000);
//	int iStreamId = 2;
//	_pIIIWriteParams(pHandle, "addtion.serverIp", "10.155.11.159", NULL, EncGroupIdx);
//	_pIIIWriteParams(pHandle, "addtion.streamId", NULL, iStreamId, EncGroupIdx);
//	_pIIIReadParams(pHandle, "addtion.wxh", "640x480", NULL, EncGroupIdx);
	
#ifdef _WIN32__
	int j = 0, baseCnt = 100000;// 20;// 100;
	Sleep(2000);
///	_pIIIRenewUrl(pHandle, streamId, "rtmp://127.0.0.1/live/live-0", 0, 0, kAVMux);
	do
	{
		Sleep(100);
		//usleep(2000000);
		//if (0)
		if (!(j % 200) && j)
		{
			if (!(j % 400))
			{
				printf("_pIIIChangeCam =================\n");
				ret = _pIIIAddExtWin(pHandle, localIdx, wind0, 640, 480, 320, 240, 320, 240);
				ret = _pIIIChangeCam(pHandle, streamId, (void *)&localIdx);// NULL);
				_pIIIRenewUrl(pHandle, streamId, "rtmp://127.0.0.1/live/live-0", 0, 0, kAVMux);
			}
			else
			{
				///_pIIIRenewUrl(pHandle, streamId, "http://fwss.xiu.youku.com/live/f/v1/000000000000000000000000153BE8DB?token=98765", 0, 0, kAVMux);
			}
		}

	} while (j++ < baseCnt);
#if 1
	int loopn = 10;
	do
	{
		//winIdx = _pIIIAddExtWin(pHandle, wind0, 640, 480, 320, 240, 320, 240);
		ret = _pIIIChangeCam(pHandle, 1, NULL);
		j = 0, baseCnt = 100;
		do
		{
			Sleep(100);
			//usleep(2000000);

		} while (j++ < baseCnt);
	} while (loopn--);
	
#endif
#endif
	//ret = _pIITStopAVCodec(pHandle, streamId, kEncoder);//enc
	streamId = 1;
	ret = _pIITStopAVCodec(pHandle, streamId, kDecoder);//dec
	Sleep(5000);

	streamId = 2;
	ret = _pIITStopAVCodec(pHandle, streamId, kDecoder);//dec
	ret = _pIITStopAVCodec(pHandle, streamId, kDecoder);//dec
	ret = _pIITStopAVCodec(pHandle, streamId, kEncoder);//enc
	ret = _pIIIDeleteAVETestObj(pHandle);
//#ifdef _WIN32
//	j = 0, baseCnt = 100;
//	do
//	{
//		Sleep(100);
//		//usleep(2000000);
//
//	} while (j++ < baseCnt);
//#endif
	return ret;
}
#endif

#ifndef MFC_DEMO

#ifndef AVDLL
void main(int argc, char** argv)
{
	int cnt = 0;
	int width = WINWIDTH;// 1280;// 640;
	int height = WINHEIGHT;// 720;// 480;
	//char *testchar = CONCAT_HELPER(ROOT_DIR, "testing/gtest.lib");
	//printf("dll test %s \n", testchar);
	printf("dll test %d \n", cnt);
	if (argc != 3)
	{
		printf("no input params: argc = %d \n", argc);
		printf("argv[0] = %s \n", argv[0]);
		printf("argv[1] = %s \n", argv[1]);
		//Sleep(10000);
		//return;
	}
	else
	{
		g_streamId = (char *)argv[2];
		printf("g_streamId = %s \n", g_streamId);
	}
#ifdef DEBUG_FFMPEG
	int ret0 = LoadDll("ffcodec.dll");
	if (ret0 < 0)
	{
		printf("load ffcodec.dll fail \n");
	}
#endif
	int ret = LoadAPI("avengine_dll.dll");
	if (ret >= 0)
	{
		//FF_TEST_MAIN2(0);
		//
		//
		int winNum = 2;// 1;
		void *winHnd = _pICreateMyWind(winNum, width, height);
		void *_window1 = _pIGetWind(winHnd, 0);
		void *_window2 = _pIGetWind(winHnd, 1);
		do
		{
			IIAVETestMain2((void *)_window1, (void *)_window2);
			//IIAVETestMain2((void *)_window1, (void *)_window1);
			printf("cnt = %d \n",cnt);
			cnt++;
		} while (cnt < 1);
		
		_pIDeleteMyWind(winHnd);
		printf("run dll is ok! \n");
	}
	
}
#else

#include "webrtc/base/avcodec.h"

AVCodec::AVCodec(void *window)
:_loadFlag(-1)
, _window(window)
, _winHnd(NULL)
, _pHandle(NULL)
, _num(0)
, _streamNum(0)
, _startEncoder(false)
, _logf(NULL)
{
}
AVCodec::~AVCodec()
{
	Release();
}
void AVCodec::DeleteJson(void *jsonObject)
{
	_pIDeleteJson(jsonObject);
}
int AVCodec::ReadMember(void *jsonObject, char *path, int *value, char *contex)
{
	return _pIReadMember(jsonObject, path, value, contex);
}
int AVCodec::AddMember(FILE *fp, void *jsonObject, char *path, int value, char *contex, int flag)
{
	return _pIAddMember(fp, jsonObject, path, value, contex, flag);
}
int AVCodec::SaveJson(FILE **fp, void **jsonObject)
{
	return _pISaveJson(fp, jsonObject);
}
void *AVCodec::openJSFile(int processId, void **jsonHnd)
{
	FILE *fp = NULL;
	//void *jsonHnd = NULL;
	if (_loadFlag < 0)
	{
		_loadFlag = LoadAPI("avengine_dll.dll");
	}
	//fp = fopen("MultStreamConfig.txt", "rb+");//plane_704x576
	//if (!fp)
	//{
	//	fp = fopen("MultStreamConfig.txt", "ab+");//plane_704x576
	//fputs("start gxh log: \n", fp);	 fflush(fp);
	//		jsonHnd = _pICreatJson((void *)fp);
	fp = (FILE *)_pICreatJson(processId, jsonHnd);
	//IAddMember(fp, (void *)&root, "0.codecParams.noexit1.noexit2", 6, NULL, 1);
	//IAddMember(fp, (void *)&root, "0.codecParams.noexit1.noexit3", 6, NULL, 1);
	/*if (fp)
	{
	fclose(fp);
	}*/
	//}
	//	return jsonHnd;
	return (void *)fp;
}
int AVCodec::GetPlayer(char(deviceName[10])[128], int devType)
{
	int ret = 0;
	if (_loadFlag < 0)
	{
		_loadFlag = LoadAPI("avengine_dll.dll");
	}
	if (_loadFlag >= 0)
	{
		if (!_pHandle)
		{
			ret = _pIIICreateAVETestObj(&_pHandle);
			//_pRegisterOnErrorCallback(NULL, (void*)&OnErrorCallback);
		}
		if (_pHandle)
		{
			//char(deviceName[10])[128] = {};
			char(avDev[3])[128] = {};
			//int devType = dev_player;
			//devType = dev_player;
			ret = _pIIIGetDev(_pHandle, deviceName, devType);
		}
	}
	return ret;
}
int AVCodec::StartDecoder(void *cb, void *obj, long long streamId, int winWidth, int winHeight, int orgX, int orgY, int showWidth, int showHeight, char *cPlayer)
{
	int ret = 0;
	int devType = dev_player;
	int winIdx = -1;
	//
	char newParams0[1024] = "";
	char newParams1[1024] = "";
	strcpy(newParams0, cparams[5]);
	strcpy(newParams1, cparams[5]);
	const char * str1 = newParams0;// cparams[1];
	const char * str2 = "out_flag";
	char *pos = (char *)strstr(str1, str2);
	char cId[16] = "";
	_itoa(streamId, cId, 10);
	char newText[128] = "\"streamId\":\"";
	strcat(newText, cId);
	strcat(newText, "\",\"in_filename\":\"\",");

	int offset = (int)(pos - str1);
	strcpy(&newParams1[offset - 1], newText);
	strcpy(&newParams1[offset - 1 + strlen(newText)], &pos[-1]);

	if (_loadFlag < 0)
	{
		_loadFlag = LoadAPI("avengine_dll.dll");
	}
	if (_loadFlag >= 0)
	{
		if (!_window)
		{
			_winHnd = _pICreateMyWind(1, winWidth, winHeight);
			_window = _pIGetWind(_winHnd, 0);
		}
		if (_window)
		{
			if (!_pHandle)
			{
				ret = _pIIICreateAVETestObj(&_pHandle);
				//_pRegisterOnErrorCallback(NULL, (void*)&OnErrorCallback);
			}
			if (_pHandle)
			{
				char(avDev[3])[128] = {};
				//devType = dev_player;
				//ret = _pIIIGetDev(pHandle, deviceName, devType);//dev_player
				strcpy(avDev[dev_player], cPlayer);// deviceName[0]);
				winIdx = _pIIIAddExtWin(_pHandle, -1, _window, winWidth, winHeight, orgX, orgY, showWidth, showHeight);
				_pIIISetStretchMode(_pHandle, kStretchToInsideEdge);
				//ret = _pIITStartAVCodec(pHandle, streamId, wind1, cparams[1], NULL, NULL, avDev, kDec);//dec
				ret = _pIITStartAVCodec(_pHandle, streamId, (void *)&winIdx, newParams1, cb, obj, avDev, kDecoder);//dec

				CodecContext *newCodec = new CodecContext;
				newCodec->streamId = streamId;
				newCodec->EncOrDec = kDecoder;
				_codecList.push_back(newCodec);
				_streamNum++;
			}
		}
	}
	return ret;

}
int AVCodec::StartEncoder(void *cb, void *obj, long long streamId, int winWidth, int winHeight, int orgX, int orgY, int showWidth, int showHeight, char(avDev[3])[128])
{
	int ret = 0;
	int winIdx = -1;
	///
	char newParams0[1024] = "";
	char newParams1[1024] = "";
	strcpy(newParams0, cparams[4]);
	strcpy(newParams1, cparams[4]);
	const char * str1 = newParams0;// cparams[1];
	const char * str2 = "out_flag";
	char *pos = (char *)strstr(str1, str2);
	char cId[16] = "";
	_itoa(streamId, cId, 10);
	char newText[128] = "\"streamId\":\"";
	strcat(newText, cId);
	strcat(newText, "\",\"out_streamname\":\"\",");

	int offset = (int)(pos - str1);
	strcpy(&newParams1[offset - 1], newText);
	strcpy(&newParams1[offset - 1 + strlen(newText)], &pos[-1]);
	//int devType = dev_recoder;
	//char(deviceName[10])[128] = {};
	//char(avDev[3])[128] = {};
	if (_loadFlag < 0)
	{
		_loadFlag = LoadAPI("avengine_dll.dll");
	}
	if (_loadFlag >= 0)
	{
		if (!_window)
		{
			_winHnd = _pICreateMyWind(1, winWidth, winHeight);
			_window = _pIGetWind(_winHnd, 0);
		}
		if (_window)
		{
			if (!_pHandle)
			{
				ret = _pIIICreateAVETestObj(&_pHandle);
				//_pRegisterOnErrorCallback(NULL, (void*)&OnErrorCallback);
			}
			if (_pHandle)
			{
//				ret = _pIIIGetDev(_pHandle, deviceName, devType);//dev_recoder
//				strcpy(avDev[dev_recoder], deviceName[0]);
//				devType = dev_cam;
//				ret = _pIIIGetDev(_pHandle, deviceName, devType);//dev_cam
//				strcpy(avDev[dev_cam], deviceName[0]);//
				//strcpy(avDev[dev_cam], deviceName[1]);//
				winIdx = _pIIIAddExtWin(_pHandle, -1, _window, winWidth, winHeight, orgX, orgY, showWidth, showHeight);
				//ret = _pIITStartAVCodec(pHandle, streamId, wind0, cparams[0], NULL, NULL, avDev, kEnc);//enc
				ret = _pIITStartAVCodec(_pHandle, streamId, (void *)&winIdx, newParams1, cb, obj, avDev, kEncoder);//enc

				CodecContext *newCodec = new CodecContext;
				newCodec->streamId = streamId;
				newCodec->EncOrDec = kEncoder;
				_codecList.push_back(newCodec);
			}
		}
	}

	return ret;
}
int AVCodec::RtpVideoDecode(int cmd, int channel, long long streamId, char *buf, int len) 
{
	int type = kVideo | kDecoder;
#if defined(WIN32) || defined(WIN64)
	void *group0 = _pIIIGetGroup(_pHandle, type, streamId);
#else
	void *group0 = IIIGetGroup(pHandle_, type, streamId_);
#endif
	AVGroup *gpVideo = (AVGroup *)group0;
#if defined(WIN32) || defined(WIN64)
	int ret = _pIIIPlayAV(_pHandle, (void*)buf, len, gpVideo->VDChanId, kVideo);
#else
	int ret = IIIPlayAV(pHandle_, (void*)buf, len, gpVideo->VDChanId, kVideo);
#endif
	return ret;
}

int AVCodec::RtpAudioDecode(int cmd, int channel, long long streamId, char *buf, int len) 
{
	int type = kAudio | kDecoder;//kAudio = 1, kVideo = 2, kEncoder = 4, kDecoder
#if defined(WIN32) || defined(WIN64)
	void *group0 = _pIIIGetGroup(_pHandle, type, streamId);
#else
	void *group0 = IIIGetGroup(_pHandle, type, streamId_);
#endif
	if(!group0)
	{
		return -1;
	}
	AVGroup *gpAudio = (AVGroup *)group0;
#if defined(WIN32) || defined(WIN64)
	int ret = _pIIIPlayAV(_pHandle, (void*)buf, len, gpAudio->ADChanId, kAudio);
#else
	int ret = IIIPlayAV(_pHandle, (void*)buf, len, gpAudio->ADChanId, kAudio);
#endif
	return ret;
}
void AVCodec::Release()
{
	int ret = 0;
	if (_pHandle)
	{
		_pFF_FACTORY_STOP();
		for (std::list<CodecContext*>::iterator it = _codecList.begin(); it != _codecList.end(); ++it)
		{
			CodecContext *newCodec = *it;
			long long streamId = newCodec->streamId;// (*it)->streamId;
			int EncOrDec = newCodec->EncOrDec;
			ret = _pIITStopAVCodec(_pHandle, streamId, EncOrDec);
			//delete *it;
			delete newCodec;
		}
		_codecList.clear();
		if (_pHandle)
		{
			ret = _pIIIDeleteAVETestObj(_pHandle);
			_pHandle = NULL;
		}
	}
	if (_winHnd)
	{
		_pIDeleteMyWind(_winHnd);
		_winHnd = NULL;
	}
}
#endif

#else
//A动态库创建则由A动态库释放
//在编译动态库和编译可执行程序中增加编译选项/MD,则可共同操作同一文件指针；

static int loadFlag = -1;
void *_winHnd = NULL;
//void *_window = NULL;
void *pHandle = NULL;
int g_num = 0;
int g_streamNum = 0;
int g_startEncoder = 0;
static FILE *g_logf = NULL;
void DeleteJson(void *jsonObject)
{
	_pIDeleteJson(jsonObject);
}
int ReadMember(void *jsonObject, char *path, int *value, char *contex)
{
	return _pIReadMember(jsonObject, path, value, contex);
}
int AddMember(FILE *fp, void *jsonObject, char *path, int value, char *contex, int flag)
{
	return _pIAddMember(fp, jsonObject, path, value, contex, flag);
}
int SaveJson(FILE **fp, void **jsonObject)
{
	return _pISaveJson(fp, jsonObject);
}
void *openJSFile(int processId, void **jsonHnd)
{
	FILE *fp = NULL;
	//void *jsonHnd = NULL;
	if (loadFlag < 0)
	{
		loadFlag = LoadAPI("avengine_dll.dll");
	}
	//fp = fopen("MultStreamConfig.txt", "rb+");//plane_704x576
	//if (!fp)
	//{
	//	fp = fopen("MultStreamConfig.txt", "ab+");//plane_704x576
		//fputs("start gxh log: \n", fp);	 fflush(fp);
//		jsonHnd = _pICreatJson((void *)fp);
	fp = (FILE *)_pICreatJson(processId, jsonHnd);
		//IAddMember(fp, (void *)&root, "0.codecParams.noexit1.noexit2", 6, NULL, 1);
		//IAddMember(fp, (void *)&root, "0.codecParams.noexit1.noexit3", 6, NULL, 1);
		/*if (fp)
		{
			fclose(fp);
		}*/
	//}
//	return jsonHnd;
	return (void *)fp;
}
int GetPlayer(char(deviceName[10])[128])
{
	int ret = 0;
	if (loadFlag < 0)
	{
		loadFlag = LoadAPI("avengine_dll.dll");
	}
	if (loadFlag >= 0)
	{
		if (!pHandle)
		{
			ret = _pIIICreateAVETestObj(&pHandle);
			//_pRegisterOnErrorCallback(NULL, (void*)&OnErrorCallback);
		}
		if (pHandle)
		{
			//char(deviceName[10])[128] = {};
			char(avDev[3])[128] = {};
			int devType = dev_player;
			devType = dev_player;
			ret = _pIIIGetDev(pHandle, deviceName, devType);
		}
	}
	return ret;
}
void test_main(void *_window, int showWidth0, int showHeight0, int orgX, int orgY, int streamNum0, char *sIP, bool startServer, bool startEncoder, int width, int height, char *cPlayer)
{
	int streamNum = streamNum0 + startEncoder;
	int winWidth = width;// 640;
	int winHeight = height;// 480;
	int i = 0, j = 0, cnt = 0;
	for (i = 1; i < 8; i++)
	{
		if (i*i >= streamNum)
		{
			break;
		}
	}
	int n = i;
	int showWidth = winWidth / n;
	int showHeight = winHeight / n;
	showWidth = showWidth0 > 0 ? showWidth0 : showWidth;
	showHeight = showHeight0 > 0 ? showHeight0 : showHeight;
	g_num = n * n;
	g_streamNum = streamNum0;
	g_startEncoder = startEncoder;
	if (loadFlag < 0)
	{
		loadFlag = LoadAPI("avengine_dll.dll");
	}
	if (loadFlag >= 0)
	{
		if (!_window)
		{
			_winHnd = _pICreateMyWind(1, winWidth, winHeight);
			_window = _pIGetWind(_winHnd, 0);
		}
		if (_window)
		{
			int ret = 0;
			long long streamId = 1;
			if (!pHandle)
			{
				ret = _pIIICreateAVETestObj(&pHandle);
				//_pRegisterOnErrorCallback(NULL, (void*)&OnErrorCallback);
			}
			if (pHandle)
			{
				char(deviceName[10])[128] = {};
				char(avDev[3])[128] = {};
				int devType = dev_player;//dev_cam = 0, dev_recoder, dev_player
				int winIdx = -1;
				for (i = 0; i < n; i++)
				{
					for (j = 0; j < n; j++)
					{
						//dec
						if ((i*n + j) < streamNum0)
						{
							if (streamId == 44)
							{
								printf("");
								//Sleep(1000);
							}
							char newParams0[1024] = "";
							char newParams1[1024] = "";
							strcpy(newParams0, cparams[5]);
							strcpy(newParams1, cparams[5]);
							const char * str1 = newParams0;// cparams[1];
							const char * str2 = "out_flag";
							char *pos = (char *)strstr(str1, str2);
							char cId[16] = "";
							_itoa(streamId, cId, 10);
							char newText[128] = "\"streamId\":\"";
							strcat(newText, cId);
							strcat(newText, "\",\"in_filename\":\"");
							strcat(newText, "rtmp://");
							strcat(newText, sIP);
							strcat(newText, "/live/live");
							strcat(newText, cId);
							strcat(newText, "-0\",");
							int offset = (int)(pos - str1);
							strcpy(&newParams1[offset - 1], newText);
							strcpy(&newParams1[offset - 1 + strlen(newText)], &pos[-1]);

							//
							//devType = dev_player;
							//ret = _pIIIGetDev(pHandle, deviceName, devType);//dev_player
							strcpy(avDev[dev_player], cPlayer);// deviceName[0]);
							winIdx = _pIIIAddExtWin(pHandle, -1, _window, winWidth, winHeight, orgX + j*showWidth, orgY + i*showHeight, showWidth, showHeight);
							_pIIISetStretchMode(pHandle, kStretchToInsideEdge);
							//ret = _pIITStartAVCodec(pHandle, streamId, wind1, cparams[1], NULL, NULL, avDev, kDec);//dec
							ret = _pIITStartAVCodec(pHandle, streamId, (void *)&winIdx, newParams1, NULL, NULL, avDev, kDecoder);//dec
										
						}
						//Sleep(1000);
						//enc
						if (startEncoder && streamId == 1)
						{
							char newParams0[1024] = "";
							char newParams1[1024] = "";
							strcpy(newParams0, cparams[4]);
							strcpy(newParams1, cparams[4]);
							const char * str1 = newParams0;// cparams[1];
							const char * str2 = "out_flag";
							char *pos = (char *)strstr(str1, str2);
							char cId[16] = "";
							_itoa(streamId, cId, 10);
							char newText[128] = "\"streamId\":\"";
							strcat(newText, cId);
							strcat(newText, "\",\"out_streamname\":\"");
							strcat(newText, "rtmp://");
							strcat(newText, sIP);
							strcat(newText, "/live/live");
							strcat(newText, cId);
							strcat(newText, "\",");//
							int offset = (int)(pos - str1);
							strcpy(&newParams1[offset - 1], newText);
							strcpy(&newParams1[offset - 1 + strlen(newText)], &pos[-1]);
							//\"out_streamname\":\"rtmp://127.0.0.1/live/live\",\
															//
							devType = dev_recoder;
							ret = _pIIIGetDev(pHandle, deviceName, devType);//dev_recoder
							strcpy(avDev[dev_recoder], deviceName[0]);
							devType = dev_cam;
							ret = _pIIIGetDev(pHandle, deviceName, devType);//dev_cam
							strcpy(avDev[dev_cam], deviceName[0]);//
							//strcpy(avDev[dev_cam], deviceName[1]);//
							winIdx = _pIIIAddExtWin(pHandle, -1, _window, winWidth, winHeight, (n - 1)*showWidth, (n - 1)*showHeight, showWidth, showHeight);
							//ret = _pIITStartAVCodec(pHandle, streamId, wind0, cparams[0], NULL, NULL, avDev, kEnc);//enc
							ret = _pIITStartAVCodec(pHandle, streamId, (void *)&winIdx, newParams1, NULL, NULL, avDev, kEncoder);//enc
						
						}
						streamId++;
					}
				}			
			}
		}
	}
}
void test_delete()
{
	int ret = 0;
	long long streamId = 1;
	_pFF_FACTORY_STOP();
	for (int i = 0; i < g_streamNum; i++)
	{
		ret = _pIITStopAVCodec(pHandle, streamId, kDecoder);//dec
		if (g_startEncoder && streamId == 1)
		{
			ret = _pIITStopAVCodec(pHandle, streamId, kEncoder);//enc
		}
		streamId++;
	}
	
	ret = _pIIIDeleteAVETestObj(pHandle);
	_pIDeleteMyWind(_winHnd);
}

#ifdef AVDLL
#include "AVAPI.h"
void IAVMain(int processId, void *wHnd, int argc, char* argv[])
{
#else
//$(TargetExt)
void main(int argc, char* argv[])
{
	int processId = 1;
#endif
	void *m_fp = NULL;
	void *m_jsonHnd = NULL;
	int bStartEncoder = 0;
	int bStartServer = 0;
	int iStreamNumber = 1;
	std::string sServerIP;
	int iWidth = 640;
	int iHeight = 480;
	int showWidth = 640;
	int showHeight = 480;
	int orgX = 0;
	int orgY = 0;
	char cServerIP[32] = "10.155.11.3";
	char cPlayer[128] = "0";
	int streamId = 1;
	char serverIp[32] = "";
#if 1
	m_fp = (FILE *)openJSFile(processId, &m_jsonHnd);
	ReadMember(m_jsonHnd, "base.streamNum", &iStreamNumber, NULL);

	ReadMember(m_jsonHnd, "base.streamId", &streamId, NULL);

	ReadMember(m_jsonHnd, "base.serverIp", NULL, serverIp);	sServerIP = serverIp;
	ReadMember(m_jsonHnd, "base.width", &iWidth, NULL);
	ReadMember(m_jsonHnd, "base.height", &iHeight, NULL);
	ReadMember(m_jsonHnd, "base.showWidth", &showWidth, NULL);
	ReadMember(m_jsonHnd, "base.showHeight", &showHeight, NULL);
	ReadMember(m_jsonHnd, "base.orgX", &orgX, NULL);
	ReadMember(m_jsonHnd, "base.orgY", &orgY, NULL);
	int value = 0;
	ReadMember(m_jsonHnd, "base.aplydev", NULL, cPlayer);
	ReadMember(m_jsonHnd, "base.startServer", &value, NULL);	bStartServer = value ? TRUE : FALSE;
	ReadMember(m_jsonHnd, "base.startEncoder", &value, NULL);	bStartEncoder = value ? TRUE : FALSE;
#endif
	//showWidth 320 showHeight 240 orgX 0 orgY 0 streamNum 1 streamId 1 serverIp 10.155.11.3 width 640 height 480 aplydev "player (Realtek High Definition Audio)"
	//showWidth 320 showHeight 240 orgX 0 orgY 0  streamNum 1 streamId 1 serverIp 10.155.11.3 width 640 height 480 aplydev "player (Realtek High Definition Audio)"
	for (int i = 0; i < argc; i++)
	{
		if (!strcmp(argv[i], "streamNum"))
		{
			i++;
			sscanf(argv[i], "%d", &iStreamNumber);
		}
		else if (!strcmp(argv[i], "streamId"))
		{
			i++;
			sscanf(argv[i], "%d", &streamId);
		}
		else if (!strcmp(argv[i], "serverIp"))
		{
			i++;
			sscanf(argv[i], "%s", cServerIP);
			strcpy(cServerIP, argv[i]);
		}
		else if (!strcmp(argv[i], "width"))
		{
			i++;
			sscanf(argv[i], "%d", &iWidth);
		}
		else if (!strcmp(argv[i], "height"))
		{
			i++;
			sscanf(argv[i], "%d", &iHeight);
		}
		else if (!strcmp(argv[i], "showWidth"))
		{
			i++;
			sscanf(argv[i], "%d", &showWidth);
		}
		else if (!strcmp(argv[i], "showHeight"))
		{
			i++;
			sscanf(argv[i], "%d", &showHeight);
		}
		else if (!strcmp(argv[i], "orgX"))
		{
			i++;
			sscanf(argv[i], "%d", &orgX);
		}
		else if (!strcmp(argv[i], "orgY"))
		{
			i++;
			sscanf(argv[i], "%d", &orgY);
		}
		else if (!strcmp(argv[i], "aplydev"))
		{
			i++;
			sscanf(argv[i], "%s", cPlayer);
			strcpy(cPlayer, argv[i]);
		}
		else if (!strcmp(argv[i], "startServer"))
		{
			i++;
			sscanf(argv[i], "%d", &bStartServer);
		}
		else if (!strcmp(argv[i], "startEncoder"))
		{
			i++;
			sscanf(argv[i], "%d", &bStartEncoder);
		}
	}
	if (!g_logf)
	{
		char logpath[128] = "log/avtest-";
		char cProcessId[16] = "1";
		sprintf(cProcessId, "%d", processId);
		strcat(logpath, cProcessId);
		strcat(logpath, ".txt");
		g_logf = fopen(logpath, "wb");
	}
	if (g_logf)
	{
		fprintf(g_logf, "IAVMain processId = %d \n", *((int *)&processId));	fflush(g_logf);
		fprintf(g_logf, "IAVMain argc = %d \n", *((int *)&argc));	fflush(g_logf);
		fprintf(g_logf, "IAVMain cPlayer = %s \n", cPlayer);	fflush(g_logf);
	}
	
	//

	int iPlayer = -1;
	if(strlen(cPlayer) < 4)
	{
		iPlayer = atoi(cPlayer);
	}
	char(deviceName[10])[128] = {};
	int ret = GetPlayer(deviceName);
	for (int i = 0; i < ret; i++)
	{
		if (i == iPlayer)
		{
			AddMember((FILE *)NULL, m_jsonHnd, "base.aplydev", NULL, deviceName[i], 0);
			strcpy(cPlayer, deviceName[i]);
		}
	}

	FILE *g_config = NULL;
	char text[256] = "";
	if (!g_config)
	{
		char configPath[128] = "MonitorConfig.txt";
		g_config = fopen(configPath, "rb+");
	}
	if (g_config)
	{
		fseek(g_config, 0, SEEK_END);
		int size = ftell(g_config);
		rewind(g_config);
		fread(text, sizeof(char), size, g_config);
		text[size] = '\0';
		char cDevNum[16] = "";
		itoa(ret, cDevNum, 10);
		//text[5] = cDevNum[0];
		//
		char cStreamNumber[16] = "";
		int m = sscanf(text, "%s%s%s%s%d%d", cStreamNumber, cPlayer, cDevNum, cServerIP, &iWidth, &iHeight);
		itoa(ret, cDevNum, 10);
		char text2[256] = "";
		sprintf(text2, "%s %s %s %s %d %d ", cStreamNumber, cPlayer, cDevNum, cServerIP, iWidth, iHeight);
		//
		rewind(g_config);
		fprintf(g_config, "%s", text2);//fflush(g_config);
		fclose(g_config);
	}

	//
	AddMember((FILE *)NULL, m_jsonHnd, "base.streamNum", iStreamNumber, NULL, 0);
	//int streamId = 1;
	AddMember((FILE *)NULL, m_jsonHnd, "base.streamId", streamId, NULL, 0);
	AddMember((FILE *)NULL, m_jsonHnd, "base.serverIp", NULL, cServerIP, 0);
	AddMember((FILE *)NULL, m_jsonHnd, "base.width", iWidth, NULL, 0);
	AddMember((FILE *)NULL, m_jsonHnd, "base.height", iHeight, NULL, 0);
	//int value = 0;
	//ReadMember(m_jsonHnd, "base.aplydev", &value, NULL);
	value = bStartServer;
	AddMember((FILE *)NULL, m_jsonHnd, "base.startServer", value, NULL, 0);
	value = bStartEncoder;
	AddMember((FILE *)NULL, m_jsonHnd, "base.startEncoder", value, NULL, 0);
	SaveJson((FILE **)&m_fp, &m_jsonHnd);
	if (g_logf) { fprintf(g_logf, "IAVMain argc2 = %d \n", *((int *)&argc));	fflush(g_logf); }
	//test_main(NULL, showWidth, showHeight, orgX, orgY, iStreamNumber, (char *)cServerIP, bStartServer, bStartEncoder, iWidth, iHeight, cPlayer);
	test_main(wHnd, showWidth, showHeight, orgX, orgY, iStreamNumber, (char *)cServerIP, bStartServer, bStartEncoder, iWidth, iHeight, cPlayer);
	if (g_logf) { fprintf(g_logf, "IAVMain argc3 = %d \n", *((int *)&argc));	fflush(g_logf); }
#ifdef AVDLL
	do
	{
		Sleep(100);
	} while (1);
#endif
}
//==============================================
int IGetPlayer(char(deviceName[10])[128])
{
	return GetPlayer(deviceName);
}
void *ICreateWin(int winWidth, int winHeight)
{
	
	void *winHnd = _pICreateMyWind(1, winWidth, winHeight);
	void *window = _pIGetWind(winHnd, 0);
	return window;
}
void IAVDelete()
{
	test_delete();
}
#endif