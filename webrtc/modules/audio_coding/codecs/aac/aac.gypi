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
      'target_name': 'webrtc_aac',
      'type': 'static_library',
      'dependencies': [
        'audio_encoder_interface',
      ],
      'include_dirs': [
        '<(webrtc_root)',
      ],
      'sources': [
        'interface/aac_interface.h',
        'interface/audio_encoder_aac.h',
        'aac_codec.h',
        'aac_codec.cc',
		'aac_interface.cc',
        'audio_encoder_aac.cc',		
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
