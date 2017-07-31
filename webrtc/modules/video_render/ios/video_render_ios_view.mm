/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

#include "webrtc/modules/video_render/ios/video_render_ios_view.h"
#include "webrtc/system_wrappers/interface/trace.h"
#include "webrtc/modules/video_render/ios/video_render_ios_impl.h"

#include "webrtc/avengine/interface/ios_video_render_api.h"

using namespace webrtc;

@implementation VideoRenderIosView {
  EAGLContext* _context;
  rtc::scoped_ptr<webrtc::OpenGles20> _gles_renderer20;
  int _frameBufferWidth;
  int _frameBufferHeight;
  unsigned int _defaultFrameBuffer;
  unsigned int _colorRenderBuffer;
}

@synthesize context = context_;

+ (Class)layerClass {
  return [CAEAGLLayer class];
}

- (id)initWithCoder:(NSCoder*)coder {
  // init super class
  self = [super initWithCoder:coder];
  if (self) {
    _gles_renderer20.reset(new OpenGles20());
  }
  return self;
}

- (id)init {
  // init super class
  self = [super init];
  if (self) {
    _gles_renderer20.reset(new OpenGles20());
  }
  return self;
}

- (id)initWithFrame:(CGRect)frame {
  // init super class
  self = [super initWithFrame:frame];
  if (self) {
    _gles_renderer20.reset(new OpenGles20());
  }
  return self;
}

- (void)dealloc {
  if (_defaultFrameBuffer) {
    glDeleteFramebuffers(1, &_defaultFrameBuffer);
    _defaultFrameBuffer = 0;
  }

  if (_colorRenderBuffer) {
    glDeleteRenderbuffers(1, &_colorRenderBuffer);
    _colorRenderBuffer = 0;
  }

  [EAGLContext setCurrentContext:nil];
}

- (NSString*)description {
  return [NSString stringWithFormat:
          @"A WebRTC implemented subclass of UIView."
          "+Class method is overwritten, along with custom methods"];
}

- (BOOL)createContext {
  // create OpenGLES context from self layer class
  CAEAGLLayer* eagl_layer = (CAEAGLLayer*)self.layer;
  eagl_layer.opaque = YES;
  eagl_layer.drawableProperties =
      [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO],
          kEAGLDrawablePropertyRetainedBacking,
          kEAGLColorFormatRGBA8,
          kEAGLDrawablePropertyColorFormat,
          nil];
  _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

  if (!_context) {
    return NO;
  }

  if (![EAGLContext setCurrentContext:_context]) {
    return NO;
  }

  // generates and binds the OpenGLES buffers
  glGenFramebuffers(1, &_defaultFrameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, _defaultFrameBuffer);

  // Create color render buffer and allocate backing store.
  glGenRenderbuffers(1, &_colorRenderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderBuffer);
  [_context renderbufferStorage:GL_RENDERBUFFER
                   fromDrawable:(CAEAGLLayer*)self.layer];
  glGetRenderbufferParameteriv(
      GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_frameBufferWidth);
  glGetRenderbufferParameteriv(
      GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_frameBufferHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                            GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER,
                            _colorRenderBuffer);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    return NO;
  }

  // set the frame buffer
  glBindFramebuffer(GL_FRAMEBUFFER, _defaultFrameBuffer);
  glViewport(0, 0, self.frame.size.width, self.frame.size.height);

  return _gles_renderer20->Setup([self bounds].size.width,
                                 [self bounds].size.height);
}

- (BOOL)presentFramebuffer {
  if (![_context presentRenderbuffer:GL_RENDERBUFFER]) {
    WEBRTC_TRACE(kTraceWarning,
                 kTraceVideoRenderer,
                 0,
                 "%s:%d [context present_renderbuffer] "
                 "returned false",
                 __FUNCTION__,
                 __LINE__);
  }
  return YES;
}

- (BOOL)renderFrame:(I420VideoFrame*)frameToRender {
  if (![EAGLContext setCurrentContext:_context]) {
    return NO;
  }

  return _gles_renderer20->Render(*frameToRender);
}

- (BOOL)setCoordinatesForZOrder:(const float)zOrder
                           Left:(const float)left
                            Top:(const float)top
                          Right:(const float)right
                         Bottom:(const float)bottom {
  return _gles_renderer20->SetCoordinates(zOrder, left, top, right, bottom);
}

@end

void *lfrtcCreateIosVideoRender(int idx, CGRect frame0, float scale0, UIView *view, void **rendHnd) {
  int orgx = frame0.origin.x * scale0;
  int orgy = frame0.origin.y * scale0;
  int width = frame0.size.width * scale0;
  int height = frame0.size.height * scale0;
  CGRect frame = CGRectMake(orgx, orgy, width, height);
  VideoRenderIosView *renderView = [[VideoRenderIosView alloc] initWithFrame:frame];
  IVideoRender* _ptrRenderer = NULL;

  webrtc::VideoRenderIosImpl* ptrRenderer = new webrtc::VideoRenderIosImpl(idx, (__bridge void *)renderView, NO);
  if (ptrRenderer) {
    _ptrRenderer = reinterpret_cast<IVideoRender*>(ptrRenderer);
    if (_ptrRenderer->Init() != -1) {
    } else {
      int ret = -1;
      NSLog(@"CREATE_VIDEO_RENDER_OBJ: %d ",ret);
      return (__bridge void *)renderView;
      return nil;
    }
  }

  float scale = 1.0 / scale0;
  float offsetx = frame0.size.width * scale;
  float offsety = frame0.size.height * scale;
  if (view == NULL) {
    offsetx = frame0.size.width;
    offsety = frame0.size.height;
    ((UIView *)renderView).transform = CGAffineTransformScale(((UIView *)renderView).transform, 1.0 * scale, -1.0 * scale);
    ((UIView *)renderView).transform = CGAffineTransformTranslate(((UIView *)renderView).transform, -offsetx, offsety);
  }
  else {
    view.transform = CGAffineTransformScale(view.transform, 1.0 * scale, -1.0 * scale);
    view.transform = CGAffineTransformTranslate(view.transform, -offsetx, -offsety);
  }

  if (rendHnd) {
    *rendHnd = _ptrRenderer;
  }
  return (__bridge void *)renderView;
}

void lfrtcDestroyIosVideoRender(void *rendHnd) {
  if (rendHnd) {
    IVideoRender *ptrRenderer = reinterpret_cast<IVideoRender*>(rendHnd);
    delete ptrRenderer;
  }
}

void CREATE_VIDEO_RENDER_PAUSE(void *hnd, bool flag) {
  webrtc::VideoRenderIosImpl* ptrRenderer = reinterpret_cast<webrtc::VideoRenderIosImpl*>(hnd);
  if(ptrRenderer) {
    printf("CREATE_VIDEO_RENDER_PAUSE:%d\n",flag);
    ptrRenderer->PauseRender(flag);
  }
}

/*void *CREATE_VIDEO_RENDER_OBJ(CGRect frame) {
  VideoRenderIosView *renderView = [[VideoRenderIosView alloc] initWithFrame:frame];
  return (__bridge void *)renderView;
}

int CREATE_VIDEO_RENDER_INIT(void *hnd) {
  VideoRenderIosView *renderView = (__bridge VideoRenderIosView *)hnd;
  webrtc::VideoRenderIosImpl* ptrRenderer = new webrtc::VideoRenderIosImpl(0, (__bridge void *)renderView, NO);
  if (ptrRenderer->Init() != -1) {
  } else {
    return 1;
  }
  return 0;
}

*/
