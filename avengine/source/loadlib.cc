#include "webrtc/avengine/interface/avengine_api.h"
#include "webrtc/avengine/interface/ffmpeg_api.h"

#ifdef _WIN32

#define CONCAT_HELPER(text,line)	text##line
#define FFMPEG_DIR "../../third_party/ffmpeg/windows/"

#if 0
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/avcodec.lib"))
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/avutil.lib"))
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/avformat.lib"))
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/swresample.lib"))
#else
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/libavcodec.a"))
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/libavutil.a"))
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/libavformat.a"))
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/libswresample.a"))
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/fdk-aac.lib"))
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/libx264.a"))
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/libgcc.a"))
#pragma comment(lib, CONCAT_HELPER(FFMPEG_DIR, "lib/libmingwex.a"))
#endif

#endif//_WIN32
