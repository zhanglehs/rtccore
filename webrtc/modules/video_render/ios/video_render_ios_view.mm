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

#include "webrtc/avengine/interface/avengine_api.h"

using namespace webrtc;

@implementation VideoRenderIosView {
  EAGLContext* _context;
  rtc::scoped_ptr<webrtc::OpenGles20> _gles_renderer20;
  int _frameBufferWidth;
  int _frameBufferHeight;
  unsigned int _defaultFrameBuffer;
  unsigned int _colorRenderBuffer;
  bool _pause_render;
}

@synthesize context = context_;

+ (Class)layerClass {
  return [CAEAGLLayer class];
}

- (id)initWithCoder:(NSCoder*)coder {
  // init super class
  self = [super initWithCoder:coder];
  if (self) {
    [self myinit];
  }
  return self;
}

- (id)init {
  // init super class
  self = [super init];
  if (self) {
    [self myinit];
  }
  return self;
}

- (id)initWithFrame:(CGRect)frame {
  // init super class
  self = [super initWithFrame:frame];
  if (self) {
    [self myinit];
  }
  return self;
}

- (void)myinit {
  if (!_gles_renderer20) {
    _gles_renderer20.reset(new OpenGles20());
    _pause_render = false;
    UIApplicationState state = [UIApplication sharedApplication].applicationState;
    if(UIApplicationStateActive != state) {
      _pause_render = true;
    }
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillEnterForeground) name:UIApplicationWillEnterForegroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive) name:UIApplicationDidBecomeActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive) name:UIApplicationWillResignActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidEnterBackground) name:UIApplicationDidEnterBackgroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillTerminate) name:UIApplicationWillTerminateNotification object:nil];
  }
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

  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (NSString*)description {
  return [NSString stringWithFormat:
          @"A WebRTC implemented subclass of UIView."
          "+Class method is overwritten, along with custom methods"];
}

- (BOOL)createContext {
  if (_context) {
    return YES;
  }
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
    if (_pause_render) {
        return NO;
    }
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
  if (_pause_render) {
    return NO;
  }
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

- (void)applicationWillEnterForeground {
}

- (void)applicationDidBecomeActive {
  _pause_render = false;
}

- (void)applicationWillResignActive {
  _pause_render = true;
}

- (void)applicationDidEnterBackground {
}

- (void)applicationWillTerminate {
}

@end

UIView *lfrtcCreateIosRenderView() {
  VideoRenderIosView *renderView = [[VideoRenderIosView alloc] init];
  // 如果去掉该行，则画面是上下颠倒的，估计是opengl渲染时参数有误。
  // 这里将画面上下反转过来。其实应修改opengl渲染参数的，或许还能提高效率
  renderView.transform = CGAffineTransformScale(renderView.transform, 1.0f, -1.0f);
  return renderView;
}
