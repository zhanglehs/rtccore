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

#include "webrtc/modules/video_render/ios/video_render_ios_channel.h"

using namespace webrtc;

VideoRenderIosChannel::VideoRenderIosChannel(VideoRenderIosView* view)
    : view_(view),
      current_frame_(new I420VideoFrame()),
      buffer_is_updated_(false) {
    _firstUpdated = true;
    _userdata = NULL;
  }

VideoRenderIosChannel::~VideoRenderIosChannel() { delete current_frame_; }

int32_t VideoRenderIosChannel::RenderFrame(const uint32_t stream_id,
                                           I420VideoFrame& video_frame) {
  video_frame.set_render_time_ms(0);

  current_frame_->CopyFrame(video_frame);
  buffer_is_updated_ = true;

  return 0;
}

void VideoRenderIosChannel::SetUserdata(void* data) {
  _userdata = data;
}

void* VideoRenderIosChannel::GetUserdata() {
  return _userdata;
}

bool VideoRenderIosChannel::RenderOffScreenBuffer() {
  _firstUpdated = false;
  if (![view_ renderFrame:current_frame_]) {
    return false;
  }

  buffer_is_updated_ = false;
  return true;
}

int VideoRenderIosChannel::IsUpdated(bool& isUpdated, bool& firstUpdated) {
  isUpdated = buffer_is_updated_;
  firstUpdated = buffer_is_updated_ && _firstUpdated;
  return 0;
}

int VideoRenderIosChannel::SetStreamSettings(const float z_order,
                                             const float left,
                                             const float top,
                                             const float right,
                                             const float bottom) {
  if (![view_ setCoordinatesForZOrder:z_order
                                 Left:left
                                  Top:bottom
                                Right:right
                               Bottom:top]) {

    return -1;
  }

  return 0;
}
