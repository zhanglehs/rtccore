# Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'targets': [
    {
      'target_name': 'webrtc_ffmpeg',
      'type': 'static_library',
      'dependencies': [
        '<(webrtc_root)/system_wrappers/system_wrappers.gyp:system_wrappers',
      ],

      'include_dirs': [
        '../../../../../../../third_party/ffmpeg/include',
      ],

      'sources': [
        'ffmpeg_factory.h',
        'ffmpeg_factory.cc',
        'ffmpeg_resample.cc',
        'ffmpeg_resample.h',
        'rtp_enc_h264.cc',
      ],

      'conditions': [
        ['OS=="ios"', {
          'sources': [
            'file_ios.m',
          ],
        }],
      ],

      'variables': {
        'clang_warning_flags': [
          '-Wno-incompatible-pointer-types',
          '-Wno-pointer-sign',
          '-Wno-unused-function',
          '-Wno-unused-variable',
          '-Wno-write-strings',
          '-Wno-format-security',
          '-Wno-unused-private-field',
          '-Wno-newline-eof',
          '-Wno-unused-value',
          '-Wno-sign-compare',
          '-Wno-deprecated-declarations',
          '-Wno-missing-field-initializers',
          '-Wno-invalid-source-encoding',
        ],
      },

      'cflags': [
        '-Wno-error',
      ],

      'msvs_disabled_warnings': [
        4189,
        4800,
        4101,
        4309,
        4169,
        4309,
        4018,
        4005,
        4289,
        4805,
        4706,
        4804,
        4189,
      ],
    },
  ],
  'conditions': [
    ['OS=="win"', {
      'msvs_settings': {
        'VCCLCompilerTool': {
          'WarningLevel': '3',
          'DisableSpecificWarnings': ['4189','4996'],
          'WarnAsError': 'false',
        },
      }, # msvs_settings
    }],
  ], # conditions
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
