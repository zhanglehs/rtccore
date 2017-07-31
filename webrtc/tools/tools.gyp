# Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'includes': [
    '../build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'video_quality_analysis',
      'type': 'static_library',
      'dependencies': [
        '<(webrtc_root)/common_video/common_video.gyp:common_video',
      ],
      'export_dependent_settings': [
        '<(webrtc_root)/common_video/common_video.gyp:common_video',
      ],
      'sources': [
        'frame_analyzer/video_quality_analysis.h',
        'frame_analyzer/video_quality_analysis.cc',
      ],
    }, # video_quality_analysis
    {
      'target_name': 'frame_editing_lib',
      'type': 'static_library',
      'dependencies': [
        '<(webrtc_root)/common_video/common_video.gyp:common_video',
      ],
      'sources': [
        'frame_editing/frame_editing_lib.cc',
        'frame_editing/frame_editing_lib.h',
      ],
      # Disable warnings to enable Win64 build, issue 1323.
      'msvs_disabled_warnings': [
        4267,  # size_t to int truncation.
      ],
    }, # frame_editing_lib
  ],
  'conditions': [
    ['include_tests==1', {
      'targets' : [
        {
          'target_name': 'agc_manager',
          'type': 'static_library',
          'dependencies': [
            '<(webrtc_root)/common_audio/common_audio.gyp:common_audio',
            '<(webrtc_root)/modules/modules.gyp:audio_processing',
            '<(webrtc_root)/voice_engine/voice_engine.gyp:voice_engine',
          ],
          'sources': [
            'agc/agc_manager.cc',
            'agc/agc_manager.h',
          ],
        },
        {
          'target_name': 'agc_test_utils',
          'type': 'static_library',
          'sources': [
            'agc/test_utils.cc',
            'agc/test_utils.h',
          ],
        },
        {
          'target_name': 'tools_unittests',
          'type': '<(gtest_target_type)',
          'dependencies': [
            'frame_editing_lib',
            'video_quality_analysis',
            '<(webrtc_root)/tools/internal_tools.gyp:command_line_parser',
            '<(webrtc_root)/test/test.gyp:test_support_main',
            '<(DEPTH)/testing/gtest.gyp:gtest',
          ],
          'sources': [
            'simple_command_line_parser_unittest.cc',
            'frame_editing/frame_editing_unittest.cc',
            'frame_analyzer/video_quality_analysis_unittest.cc',
          ],
          # Disable warnings to enable Win64 build, issue 1323.
          'msvs_disabled_warnings': [
            4267,  # size_t to int truncation.
          ],
          'conditions': [
            ['OS=="android"', {
              'dependencies': [
                '<(DEPTH)/testing/android/native_test.gyp:native_test_native_code',
              ],
            }],
          ],
        }, # tools_unittests
      ], # targets
      'conditions': [
        ['OS=="android"', {
          'targets': [
            {
              'target_name': 'tools_unittests_apk_target',
              'type': 'none',
              'dependencies': [
                '<(apk_tests_path):tools_unittests_apk',
              ],
            },
          ],
        }],
        ['test_isolation_mode != "noop"', {
          'targets': [
            {
              'target_name': 'tools_unittests_run',
              'type': 'none',
              'dependencies': [
                'tools_unittests',
              ],
              'includes': [
                '../build/isolate.gypi',
              ],
              'sources': [
                'tools_unittests.isolate',
              ],
            },
          ],
        }],
      ],
    }], # include_tests
  ], # conditions
}
