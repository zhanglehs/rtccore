// edit by zhangle
#include "webrtc/avengine/source/avengine_util.h"

#include <stdio.h>
#include <stdarg.h>

T_lfrtcNotifyCb g_NotifyCallback = NULL;
T_lfrtcReportCb g_ReportCallback = NULL;
T_lfrtcLogCb g_LogCallback = NULL;
T_lfrtcDecodedVideoCb g_DecodedVideoCallback = NULL;
T_lfrtcDecodedAudioCb g_DecodedAudioCallback = NULL;
T_lfrtcEncodedVideoCb g_EncodedVideoCallback = NULL;
T_lfrtcEncodedAudioCb g_EncodedAudioCallback = NULL;
lfrtcGlobalConfig *g_global_config = NULL;

void ExportNotifyMessage(void *playerCtx, unsigned int msgid, int wParam, int lParam) {
  if (g_NotifyCallback) {
    g_NotifyCallback(playerCtx, msgid, wParam, lParam);
  }
}

void SendAvengineReport(void *playerCtx, const char* msg) {
  if (g_ReportCallback) {
    g_ReportCallback(playerCtx, msg);
  }
}

void LogAvenginePrintf(int level, const char *file, int line_num, const char *format, ...) {
  if (g_LogCallback == NULL) {
    return;
  }
  va_list args;
  va_start(args, format);
  g_LogCallback(level, file, line_num, format, args);
  va_end(args);
}

lfrtcGlobalConfig *GetGlobalConfig() {
  return g_global_config;
}

void xxx_hack_link_bug_xxx() {
  lfrtcSetRtpData(0, 0, 0);
}
