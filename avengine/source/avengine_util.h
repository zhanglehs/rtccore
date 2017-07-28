// edit by zhangle
#ifndef AVENGINE_SOURCE_AVENGINE_UTIL_H_
#define AVENGINE_SOURCE_AVENGINE_UTIL_H_

#include "webrtc/avengine/interface/avengine_api.h"

extern T_lfrtcNotifyCb g_NotifyCallback;
extern T_lfrtcReportCb g_ReportCallback;
extern T_lfrtcLogCb g_LogCallback;
extern T_lfrtcDecodedVideoCb g_DecodedVideoCallback;
extern T_lfrtcDecodedAudioCb g_DecodedAudioCallback;
extern T_lfrtcEncodedVideoCb g_EncodedVideoCallback;
extern T_lfrtcEncodedAudioCb g_EncodedAudioCallback;
extern lfrtcGlobalConfig *g_global_config;

void ExportNotifyMessage(void *playerCtx, unsigned int msgid, int wParam, int lParam);
void SendAvengineReport(void *playerCtx, const char* msg);
void LogAvenginePrintf(int level, const char *file, int line_num, const char *format, ...);
lfrtcGlobalConfig *GetGlobalConfig();

#define AVENGINE_TRC(...) LogAvenginePrintf(AVENGINE_LOG_LEVEL_TRC, __FILE__, __LINE__, __VA_ARGS__);
#define AVENGINE_DBG(...) LogAvenginePrintf(AVENGINE_LOG_LEVEL_DBG, __FILE__, __LINE__, __VA_ARGS__);
#define AVENGINE_INF(...) LogAvenginePrintf(AVENGINE_LOG_LEVEL_INF, __FILE__, __LINE__, __VA_ARGS__);
#define AVENGINE_WRN(...) LogAvenginePrintf(AVENGINE_LOG_LEVEL_WRN, __FILE__, __LINE__, __VA_ARGS__);
#define AVENGINE_ERR(...) LogAvenginePrintf(AVENGINE_LOG_LEVEL_ERR, __FILE__, __LINE__, __VA_ARGS__);

#endif
