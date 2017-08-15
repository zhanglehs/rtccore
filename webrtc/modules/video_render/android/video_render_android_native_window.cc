#include "webrtc/modules/video_render/android/video_render_android_native_window.h"

extern "C" {
#include "libavutil/imgutils.h"
}

namespace {

#ifndef IJKMIN
#define IJKMIN(a, b)    ((a) < (b) ? (a) : (b))
#endif

#ifndef IJKALIGN
#define IJKALIGN(x, align) ((( x ) + (align) - 1) / (align) * (align))
#endif

#define SDL_LIL_ENDIAN  1234
#define SDL_BIG_ENDIAN  4321

#include <endian.h>
#define SDL_BYTEORDER  __BYTE_ORDER

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#   define SDL_FOURCC(a, b, c, d) \
  (((uint32_t)a) | (((uint32_t)b) << 8) | (((uint32_t)c) << 16) | (((uint32_t)d) << 24))
#else
#   define SDL_FOURCC(a, b, c, d) \
  (((uint32_t)d) | (((uint32_t)c) << 8) | (((uint32_t)b) << 16) | (((uint32_t)a) << 24))
#endif

  /*-
  *  http://www.webartz.com/fourcc/indexyuv.htm
  *  http://www.neuro.sfc.keio.ac.jp/~aly/polygon/info/color-space-faq.html
  *  http://www.fourcc.org/yuv.php
  */

  // YUV formats
#define SDL_FCC_YV12    SDL_FOURCC('Y', 'V', '1', '2')  /**< bpp=12, Planar mode: Y + V + U  (3 planes) */
#define SDL_FCC_IYUV    SDL_FOURCC('I', 'Y', 'U', 'V')  /**< bpp=12, Planar mode: Y + U + V  (3 planes) */
#define SDL_FCC_I420    SDL_FOURCC('I', '4', '2', '0')  /**< bpp=12, Planar mode: Y + U + V  (3 planes) */
#define SDL_FCC_I444P10LE   SDL_FOURCC('I', '4', 'A', 'L')

#define SDL_FCC_YUV2    SDL_FOURCC('Y', 'U', 'V', '2')  /**< bpp=16, Packed mode: Y0+U0+Y1+V0 (1 plane) */
#define SDL_FCC_UYVY    SDL_FOURCC('U', 'Y', 'V', 'Y')  /**< bpp=16, Packed mode: U0+Y0+V0+Y1 (1 plane) */
#define SDL_FCC_YVYU    SDL_FOURCC('Y', 'V', 'Y', 'U')  /**< bpp=16, Packed mode: Y0+V0+Y1+U0 (1 plane) */

#define SDL_FCC_NV12    SDL_FOURCC('N', 'V', '1', '2')

  // RGB formats
#define SDL_FCC_RV16    SDL_FOURCC('R', 'V', '1', '6')    /**< bpp=16, RGB565 */
#define SDL_FCC_RV24    SDL_FOURCC('R', 'V', '2', '4')    /**< bpp=24, RGB888 */
#define SDL_FCC_RV32    SDL_FOURCC('R', 'V', '3', '2')    /**< bpp=32, RGBX8888 */

  // opaque formats
#define SDL_FCC__AMC    SDL_FOURCC('_', 'A', 'M', 'C')    /**< Android MediaCodec */
#define SDL_FCC__VTB    SDL_FOURCC('_', 'V', 'T', 'B')    /**< iOS VideoToolbox */
#define SDL_FCC__GLES2  SDL_FOURCC('_', 'E', 'S', '2')    /**< let Vout choose format */

  // undefine
#define SDL_FCC_UNDF    SDL_FOURCC('U', 'N', 'D', 'F')    /**< undefined */

  enum {
    HAL_PIXEL_FORMAT_RGBA_8888 = 1,
    HAL_PIXEL_FORMAT_RGBX_8888 = 2,
    HAL_PIXEL_FORMAT_RGB_888 = 3,
    HAL_PIXEL_FORMAT_RGB_565 = 4,
    HAL_PIXEL_FORMAT_BGRA_8888 = 5,
    HAL_PIXEL_FORMAT_RGBA_5551 = 6,
    HAL_PIXEL_FORMAT_RGBA_4444 = 7,

    /* 0x8 - 0xFF range unavailable */
    /* 0x100 - 0x1FF HAL implement */
    HAL_PIXEL_FORMAT_YV12 = 0x32315659, // YCrCb 4:2:0 Planar

    HAL_PIXEL_FORMAT_RAW_SENSOR = 0x20,
    HAL_PIXEL_FORMAT_BLOB = 0x21,
    HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED = 0x22,

    /* Legacy formats (deprecated), used by ImageFormat.java */
    HAL_PIXEL_FORMAT_YCbCr_422_SP = 0x10, // NV16
    HAL_PIXEL_FORMAT_YCrCb_420_SP = 0x11, // NV21
    HAL_PIXEL_FORMAT_YCbCr_422_I = 0x14, // YUY2
  };

  static int android_render_yv12_on_yv12(ANativeWindow_Buffer *out_buffer, const NativeWindowFrame *overlay) {
    assert(overlay->format == SDL_FCC_YV12);
    assert(overlay->planes == 3);

    int min_height = IJKMIN(out_buffer->height, overlay->height);
    int dst_y_stride = out_buffer->stride;
    int dst_c_stride = IJKALIGN(out_buffer->stride / 2, 16);
    int dst_y_size = dst_y_stride * out_buffer->height;
    int dst_c_size = dst_c_stride * out_buffer->height / 2;

    uint8_t *dst_pixels_array[] = {
      (uint8_t *)(out_buffer->bits),
      (uint8_t *)(out_buffer->bits) + dst_y_size,
      (uint8_t *)(out_buffer->bits) + dst_y_size + dst_c_size,
    };
    int dst_line_height[] = { min_height, min_height / 2, min_height / 2 };
    int dst_line_size_array[] = { dst_y_stride, dst_c_stride, dst_c_stride };

    for (int i = 0; i < 3; ++i) {
      int dst_line_size = dst_line_size_array[i];
      int src_line_size = overlay->strides[i];
      int line_height = dst_line_height[i];
      uint8_t *dst_pixels = dst_pixels_array[i];
      const uint8_t *src_pixels = overlay->pixels[i];

      if (dst_line_size == src_line_size) {
        int plane_size = src_line_size * line_height;
        memcpy(dst_pixels, src_pixels, plane_size);
      }
      else {
        // TODO: 9 padding
        int bytewidth = IJKMIN(dst_line_size, src_line_size);
        av_image_copy_plane(dst_pixels, dst_line_size, src_pixels, src_line_size, bytewidth, line_height);
      }
    }

    return 0;
  }

  static int android_render_on_yv12(ANativeWindow_Buffer *out_buffer, const NativeWindowFrame *overlay) {
    assert(out_buffer);
    assert(overlay);
    switch (overlay->format) {
    case SDL_FCC_YV12:
      return android_render_yv12_on_yv12(out_buffer, overlay);
    }
    return -1;
  }

  static int android_render_rgb_on_rgb(ANativeWindow_Buffer *out_buffer, const NativeWindowFrame *overlay, int bpp) {
    assert(overlay->format == SDL_FCC_RV16);
    assert(overlay->planes == 1);

    int min_height = IJKMIN(out_buffer->height, overlay->height);
    int dst_stride = out_buffer->stride;
    int src_line_size = overlay->strides[0];
    int dst_line_size = dst_stride * bpp / 8;

    uint8_t *dst_pixels = (uint8_t *)(out_buffer->bits);
    const uint8_t *src_pixels = overlay->pixels[0];

    if (dst_line_size == src_line_size) {
      int plane_size = src_line_size * min_height;
      memcpy(dst_pixels, src_pixels, plane_size);
    }
    else {
      // TODO: 9 padding
      int bytewidth = IJKMIN(dst_line_size, src_line_size);
      av_image_copy_plane(dst_pixels, dst_line_size, src_pixels, src_line_size, bytewidth, min_height);
    }

    return 0;
  }

  static int android_render_rgb565_on_rgb565(ANativeWindow_Buffer *out_buffer, const NativeWindowFrame *overlay) {
    return android_render_rgb_on_rgb(out_buffer, overlay, 16);
  }

  static int android_render_on_rgb565(ANativeWindow_Buffer *out_buffer, const NativeWindowFrame *overlay) {
    assert(out_buffer);
    assert(overlay);
    switch (overlay->format) {
    case SDL_FCC_RV16:
       return android_render_rgb565_on_rgb565(out_buffer, overlay);
    }
    return -1;
  }

  static int android_render_rgb32_on_rgb8888(ANativeWindow_Buffer *out_buffer, const NativeWindowFrame *overlay) {
    return android_render_rgb_on_rgb(out_buffer, overlay, 32);
  }

  static int android_render_on_rgb8888(ANativeWindow_Buffer *out_buffer, const NativeWindowFrame *overlay) {
    assert(out_buffer);
    assert(overlay);
    switch (overlay->format) {
    case SDL_FCC_RV32:
      return android_render_rgb32_on_rgb8888(out_buffer, overlay);
    }
    return -1;
  }

  typedef struct AndroidHalFourccDescriptor {
    uint32_t fcc_or_hal;
    const char* name;
    int hal_format;
    int(*render)(ANativeWindow_Buffer *native_buffer, const NativeWindowFrame *overlay);
  } AndroidHalFourccDescriptor;

  static AndroidHalFourccDescriptor g_hal_fcc_map[] = {
    // YV12
    { HAL_PIXEL_FORMAT_YV12, "HAL_YV12", HAL_PIXEL_FORMAT_YV12, android_render_on_yv12 },
    { SDL_FCC_YV12, "YV12", HAL_PIXEL_FORMAT_YV12, android_render_on_yv12 },

    // RGB565
    { HAL_PIXEL_FORMAT_RGB_565, "HAL_RGB_565", HAL_PIXEL_FORMAT_RGB_565, android_render_on_rgb565 },
    { SDL_FCC_RV16, "RV16", HAL_PIXEL_FORMAT_RGB_565, android_render_on_rgb565 },

    // RGB8888
    { HAL_PIXEL_FORMAT_RGBX_8888, "HAL_RGBX_8888", HAL_PIXEL_FORMAT_RGBX_8888, android_render_on_rgb8888 },
    { HAL_PIXEL_FORMAT_RGBA_8888, "HAL_RGBA_8888", HAL_PIXEL_FORMAT_RGBA_8888, android_render_on_rgb8888 },
    { HAL_PIXEL_FORMAT_BGRA_8888, "HAL_BGRA_8888", HAL_PIXEL_FORMAT_BGRA_8888, android_render_on_rgb8888 },
    { SDL_FCC_RV32, "RV32", HAL_PIXEL_FORMAT_RGBX_8888, android_render_on_rgb8888 },
  };

  AndroidHalFourccDescriptor *native_window_get_desc(int fourcc_or_hal) {
    for (int i = 0; i < sizeof(g_hal_fcc_map)/sizeof(g_hal_fcc_map[0]); ++i) {
      AndroidHalFourccDescriptor *desc = &g_hal_fcc_map[i];
      if ((int)desc->fcc_or_hal == fourcc_or_hal)
        return desc;
    }

    return NULL;
  }

  int SDL_Android_NativeWindow_display(ANativeWindow *native_window, const NativeWindowFrame *frame) {
    if (!native_window || !frame || frame->width <= 0 || frame->height <= 0) {
      return -1;
    } 
    
    int image_width = IJKALIGN(frame->width, 2);
    int image_height = IJKALIGN(frame->height, 2);

    AndroidHalFourccDescriptor *image_desc = native_window_get_desc(frame->format);
    if (image_desc == NULL) {
      __android_log_print("%s: unknown frame format: %u", __FUNCTION__, frame->format);
      return -1;
    }

    int window_format = ANativeWindow_getFormat(native_window);
    AndroidHalFourccDescriptor *window_desc = native_window_get_desc(window_format);
    if (window_desc == NULL || window_desc->hal_format != image_desc->hal_format) {
      int retval = ANativeWindow_setBuffersGeometry(native_window, image_width, image_height, image_desc->hal_format);
      if (retval < 0) {
        __android_log_print("%s: ANativeWindow_setBuffersGeometry failed, width=%d, height=%d, format=%d, retval=%d", __FUNCTION__, image_width, image_height, image_desc->hal_format, retval);
        return retval;
      }
      if (window_desc == NULL) {
        __android_log_print("%s: unknown window format: %d", __FUNCTION__, window_format);
        return -1;
      }
    }

    ANativeWindow_Buffer window_buffer;
    int retval = ANativeWindow_lock(native_window, &window_buffer, NULL);
    if (retval < 0) {
      __android_log_print("%s: ANativeWindow_lock failed, retval=%d", __FUNCTION__, retval);
      return retval;
    }
    if (window_buffer.width != image_width || window_buffer.height != image_height) {
      __android_log_print("%s: unexpected native window buffer (%p)(w:%d, h:%d, fmt:'%.4s'0x%x), expecting (w:%d, h:%d, fmt:'%.4s'0x%x)",
        __FUNCTION__, native_window,
        out_buffer.width, out_buffer.height, (char*)&window_buffer.format, window_buffer.format,
        image_width, image_height, (char*)&image_desc->format, image_desc->format);
      // TODO: 8 set all black
      ANativeWindow_unlockAndPost(native_window);
      ANativeWindow_setBuffersGeometry(native_window, image_width, image_height, image_desc->hal_format);
      return -1;
    }

    int render_ret = window_desc->render(&window_buffer, frame);
    if (render_ret < 0) {
      __android_log_print("%s: render failed, retval=%d", __FUNCTION__, render_ret);
      // TODO: 8 set all black
    }

    retval = ANativeWindow_unlockAndPost(native_window);
    if (retval < 0) {
      __android_log_print("%s: ANativeWindow_unlockAndPost failed, retval=%d", __FUNCTION__, retval);
      return retval;
    }

    return render_ret;
  }

}

NativeWindowAdapter::NativeWindowAdapter() {
  m_native_window = NULL;
}

NativeWindowAdapter::~NativeWindowAdapter() {
  stop();
}

int NativeWindowAdapter::init(JNIEnv* env, void *surface) {
  if (env && surface && m_native_window == NULL) {
    m_native_window = ANativeWindow_fromSurface(env, surface);
  }
  return m_native_window != NULL ? 0 : -1;
}

void NativeWindowAdapter::stop() {
  if (m_native_window) {
    ANativeWindow_release(m_native_window);
    m_native_window = NULL;
  }
}

int NativeWindowAdapter::render(const NativeWindowFrame *frame) {
  if (m_native_window == NULL) {
    return -1;
  }
  return SDL_Android_NativeWindow_display(m_native_window, frame);
}
