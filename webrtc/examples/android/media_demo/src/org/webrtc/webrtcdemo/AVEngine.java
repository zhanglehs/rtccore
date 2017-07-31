/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

//package org.webrtc.avengine;
package org.webrtc.webrtcdemo;

import android.view.SurfaceView;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceHolder;
import android.view.WindowManager;

public class AVEngine {
  private final long nativeAVEngine;

  // Keep in sync (including this comment) with webrtc/common_types.h:TraceLevel
  public AVEngine() {
    nativeAVEngine = create();
  }

  // API comments can be found in VideoEngine's native APIs. Not all native
  // APIs are available.
  private static native long create();
  //public native int avtestmain(SurfaceHolder win0, SurfaceHolder win1);
  public native int avtestmain(SurfaceView win0, SurfaceView win1);
  public native int avtestffmpeg();
}
