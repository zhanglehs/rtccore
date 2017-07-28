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
      'target_name': 'neteq_test_support',
      'type': 'static_library',
      'dependencies': [
        'neteq',
        'PCM16B',
        'neteq_unittest_tools',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/third_party/gflags/gflags.gyp:gflags',
      ],
      'sources': [
        'tools/neteq_external_decoder_test.cc',
        'tools/neteq_external_decoder_test.h',
        'tools/neteq_performance_test.cc',
        'tools/neteq_performance_test.h',
        'tools/neteq_quality_test.cc',
        'tools/neteq_quality_test.h',
      ],
    }, # neteq_test_support

    {
     'target_name': 'neteq_test_tools',
      # Collection of useful functions used in other tests.
      'type': 'static_library',
      'variables': {
        # Expects RTP packets without payloads when enabled.
        'neteq_dummy_rtp%': 0,
      },
      'dependencies': [
        'G711',
        'G722',
        'PCM16B',
        'iLBC',
        'iSAC',
        'CNG',
        '<(webrtc_root)/common.gyp:webrtc_common',
        '<(DEPTH)/testing/gtest.gyp:gtest',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'interface',
          'test',
          '<(webrtc_root)',
        ],
      },
      'defines': [
      ],
      'include_dirs': [
        'interface',
        'test',
        '<(webrtc_root)',
      ],
      'sources': [
        'test/NETEQTEST_DummyRTPpacket.cc',
        'test/NETEQTEST_DummyRTPpacket.h',
        'test/NETEQTEST_RTPpacket.cc',
        'test/NETEQTEST_RTPpacket.h',
      ],
      # Disable warnings to enable Win64 build, issue 1323.
      'msvs_disabled_warnings': [
        4267,  # size_t to int truncation.
      ],
    },
  ], # targets
}
