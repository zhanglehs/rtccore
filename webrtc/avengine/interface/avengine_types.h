// edit by zhangle
#ifndef AVENGINE_SOURCE_AVENGINE_TYPES_H_
#define AVENGINE_SOURCE_AVENGINE_TYPES_H_

struct lfrtcGlobalConfig {
  unsigned int is_lostpacketStrategy;// 1 to screen  0 no
  unsigned int IlostpacketToScreen; // <= number of packet int I frame to screen
  unsigned int PlostpacketToScreen; // <= number of packet int P frame to screen
  unsigned int is_yuvDump;          // yuv dump
  unsigned int playout_delay_ms;
  unsigned int android_render_mode; // 0: software, 1: java gles, 2: native_window, 3: GLSurfaceView
  lfrtcGlobalConfig() {
    is_lostpacketStrategy = 1;
    IlostpacketToScreen = 0;
    PlostpacketToScreen = 0;
    is_yuvDump = 0;
    playout_delay_ms = 200;
    android_render_mode = 0;
  }
};

struct lfrtcDevice {
  char szDeviceName[256];
  char szDeviceID[256];
};

struct lfrtcCameraCapability {
  unsigned int width;
  unsigned int height;
  unsigned int maxFPS;
  lfrtcCameraCapability() {
    width = 0;
    height = 0;
    maxFPS = 0;
  }
};

struct lfrtcCaptureConfig {
  char audio_deviceid[256];
  int audio_frequence;
  char video_deviceid[256];
  int video_capture_width;
  int video_capture_height;
  lfrtcCaptureConfig() {
    audio_deviceid[0] = 0;
    audio_frequence = 48000;
    video_deviceid[0] = 0;
    video_capture_width = 640;
    video_capture_height = 480;
  }
};

struct lfrtcEncodeConfig {
  int video_encode_width;
  int video_encode_height;
  int video_max_fps;
  int video_gop;
  int video_mtu_size;
  int video_bitrate;
  lfrtcEncodeConfig() {
    video_encode_width = 360;
    video_encode_height = 640;
    video_max_fps = 15;
    video_gop = 15;
    video_mtu_size = 1200;
    video_bitrate = 800 * 1024;
  }
};

enum lfrtcRawVideoType {
  klfrtcVideoI420 = 0,
  klfrtcVideoARGB = 5,
  klfrtcVideoRGB24 = 6
};

typedef void(*T_lfrtcDecodedVideoCb)(void *playerCtx, char* data[3], lfrtcRawVideoType type, int width, int height);
typedef void(*T_lfrtcDecodedAudioCb)(void *playerCtx, char *buf, int len);
typedef void(*T_lfrtcPreviewVideoCb)(void *captureCtx, char* data[3], lfrtcRawVideoType type, int width, int height);

#endif
