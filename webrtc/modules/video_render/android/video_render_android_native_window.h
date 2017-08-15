#ifndef WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_ANDROID_VIDEO_RENDER_ANDROID_NATIVE_WINDOW_H_
#define WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_ANDROID_VIDEO_RENDER_ANDROID_NATIVE_WINDOW_H_

#include <jni.h>
#include <android/native_window_jni.h>

struct NativeWindowFrame {
  int width; /**< Read-only */
  int height; /**< Read-only */
  unsigned int format; /**< Read-only */
  int planes; /**< Read-only */
  unsigned short *strides; /**< in bytes, Read-only */
  unsigned char **pixels; /**< Read-write */

  NativeWindowFrame() {
    width = 0;
    height = 0;
    format = 0x32315659;   // 默认为YUV格式, 0x32315659 = SDL_FOURCC('Y', 'V', '1', '2')
    planes = 0;
    strides = NULL;
    pixels = NULL;
  }
};

class NativeWindowAdapter {
public:
  NativeWindowAdapter();
  ~NativeWindowAdapter();

  int init(JNIEnv* env, void *surface);

  void stop();

  int render(const NativeWindowFrame *frame);

private:
  ANativeWindow *m_native_window;
};


#endif  // WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_ANDROID_VIDEO_RENDER_ANDROID_NATIVEWINDOW_H_
