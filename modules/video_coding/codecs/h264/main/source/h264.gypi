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
      'target_name': 'webrtc_h264',
      'type': 'static_library',
      'dependencies': [
        '<(webrtc_root)/system_wrappers/system_wrappers.gyp:system_wrappers',
      ],
      'include_dirs': [
        '../interface',
        '../../../interface',
        '../../../../../../common_video/interface',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../interface',
          '../../../../../../common_video/interface',
        ],
      },
      'sources': [
        '../interface/h264.h',
        'h264.cc',
      ],
          'cflags': [
          '-Wno-error',
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
          '-Wno-unused-value','-Wno-sign-compare',
          '-Wno-deprecated-declarations',
          '-Wno-missing-field-initializers',
          '-Wno-invalid-source-encoding',
         ],   
       },        
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
