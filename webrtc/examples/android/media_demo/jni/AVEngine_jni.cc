/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// This file contains JNI for the video engine interfaces.
// The native functions are found using jni's auto discovery.

#include "webrtc/avengine/android/jni/AVEngine_jni.h"
#include "webrtc/modules/av_coding/codecs/ffmpeg/main/interface/ffmpeg_api.h"
#include <map>
#include <string>

#include "webrtc/examples/android/media_demo/jni/jni_helpers.h"


// Macro for native functions that can be found by way of jni-auto discovery.
// Note extern "C" is needed for "discovery" of native methods to work.
#if 0
#define JOWW(rettype, name)                                             \
  extern "C" rettype JNIEXPORT JNICALL Java_org_webrtc_avengine_##name
#else
#define JOWW(rettype, name)                                             \
  extern "C" rettype JNIEXPORT JNICALL Java_org_webrtc_webrtcdemo_##name
#endif
namespace {

#if 0
static JavaVM* g_vm = NULL;
static ClassReferenceHolder* g_class_reference_holder = NULL;

jclass GetClass(const char* name) {
  CHECK(g_class_reference_holder, "Class reference holder NULL");
  return g_class_reference_holder->GetClass(name);
}
#endif
// C(++) description of a camera. This class is created by Java native calls
// and associated with the CameraDesc Java class. The Java class is used in the
// Java code but it is just a thin wrapper of the C(++) class that contain the
// actual information. The information is stored in C(++) as it is used to
// call video engine APIs.
/*
struct CameraDesc {
  // The name and id corresponds to ViECapture's |device_nameUTF8| and
  // |unique_idUTF8|.
  char name[64];
  char unique_id[64];
};
*/
// C++ callback class that can be used to register for callbacks from the
// video engine. It further propagates the callbacks to
// VideoDecodeEncodeObserver.java interface. The memory associated with this
// class is managed globally by the VideoEngineData class when registering and
// unregistering VideoDecodeEncodeObserver.java to receive callbacks.

/*
template<typename T>
void ReleaseSubApi(T instance) {
  CHECK(instance->Release() == 0, "failed to release instance")
}
*/
class AVEngineData {
 public:
  AVEngineData()
      : _windows0(NULL),_windows1(NULL)
  {
    
  }

  ~AVEngineData()
  {
    
  }
  void *_windows0;
  void *_windows1;
 private:
  
};
AVEngineData* GetAVEngineData(JNIEnv* jni, jobject j_av) {
    jclass j_av_class = jni->GetObjectClass(j_av);
    jfieldID native_av_id = jni->GetFieldID(j_av_class, "nativeAVEngine", "J");
    jlong j_p = jni->GetLongField(j_av, native_av_id);
    return reinterpret_cast<AVEngineData*>(j_p);
}
}  // namespace
/*
namespace webrtc_examples {

static const char* g_classes[] = {
  "org/webrtc/webrtcdemo/CameraDesc",
  "org/webrtc/webrtcdemo/RtcpStatistics",
  "org/webrtc/webrtcdemo/VideoCodecInst",
  "org/webrtc/webrtcdemo/VideoDecodeEncodeObserver",
  "org/webrtc/webrtcdemo/MediaCodecVideoDecoder"};

void SetVieDeviceObjects(JavaVM* vm) {
  CHECK(vm, "Trying to register NULL vm");
  CHECK(!g_vm, "Trying to re-register vm");
  g_vm = vm;
  webrtc::AttachThreadScoped ats(g_vm);
  JNIEnv* jni = ats.env();
  g_class_reference_holder = new ClassReferenceHolder(
      jni, g_classes, arraysize(g_classes));
}

void ClearVieDeviceObjects() {
  CHECK(g_vm, "Clearing vm without it being set");
  {
    webrtc::AttachThreadScoped ats(g_vm);
    g_class_reference_holder->FreeReferences(ats.env());
  }
  g_vm = NULL;
  delete g_class_reference_holder;
  g_class_reference_holder = NULL;
}

}  // namespace webrtc_examples
*/
JOWW(jlong, AVEngine_create)(JNIEnv* jni, jclass) {
  AVEngineData* av_data = new AVEngineData();
  //LogInit(0);
  return jlongFromPointer(av_data);
}

JOWW(jint, AVEngine_avtestmain)(JNIEnv* jni, jobject j_av, jobject gl_surface0, jobject gl_surface1) {
  AVEngineData* av_data = GetAVEngineData(jni, j_av);
    av_data->_windows0 = gl_surface0;
    av_data->_windows1 = gl_surface1;
    return 0;//IIAVETestMain(gl_surface0, gl_surface1);
}
JOWW(jint, AVEngine_avtestffmpeg)(JNIEnv* jni, jobject j_av) {
    //AVEngineData* av_data = GetAVEngineData(jni, j_av);

    return 0;//FF_TEST_MAIN(0);
}


