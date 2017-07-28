# Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'targets': [
    {
      'target_name': 'transient_suppression_test',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/third_party/gflags/gflags.gyp:gflags',
        '<(webrtc_root)/test/test.gyp:test_support',
        '<(webrtc_root)/modules/modules.gyp:audio_processing',
      ],
      'sources': [
        'transient/transient_suppression_test.cc',
        'transient/file_utils.cc',
        'transient/file_utils.h',
      ],
    }, # transient_suppression_test
    {
      'target_name': 'nonlinear_beamformer_test',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/third_party/gflags/gflags.gyp:gflags',
        '<(webrtc_root)/modules/modules.gyp:audio_processing',
      ],
      'sources': [
        'beamformer/nonlinear_beamformer_test.cc',
        'beamformer/pcm_utils.cc',
        'beamformer/pcm_utils.h',
      ],
    }, # nonlinear_beamformer_test
  ],
  'conditions': [
    ['enable_protobuf==1', {
      'targets': [
        {
          'target_name': 'audioproc_unittest_proto',
          'type': 'static_library',
          'sources': [ 'test/unittest.proto', ],
          'variables': {
            'proto_in_dir': 'test',
            # Workaround to protect against gyp's pathname relativization when
            # this file is included by modules.gyp.
            'proto_out_protected': 'webrtc/audio_processing',
            'proto_out_dir': '<(proto_out_protected)',
          },
          'includes': [ '../../build/protoc.gypi', ],
        },
      ],
    }],
  ],
}
