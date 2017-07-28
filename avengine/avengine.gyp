# Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
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
      'target_name': 'avengine',
      'type': 'static_library',
      'dependencies': [
	    '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
		'<(webrtc_root)/video_engine/video_engine.gyp:video_engine_core',
		'<(webrtc_root)/modules/modules.gyp:video_capture_module_internal_impl',
		'<(webrtc_root)/modules/modules.gyp:video_render_module_internal_impl',
        '<(webrtc_root)/modules/modules.gyp:webrtc_ffmpeg',
        '<(webrtc_root)/modules/modules.gyp:webrtc_h264',
        '<(webrtc_root)/modules/modules.gyp:webrtc_aac',
        '<(webrtc_root)/system_wrappers/system_wrappers.gyp:field_trial_default',
        '<(webrtc_root)/system_wrappers/system_wrappers.gyp:metrics_default',
		'<(webrtc_root)/base/base.gyp:rtc_base',
      ],
	  'include_dirs': [
        '<(webrtc_root)',
      ],

	  'sources': [
		'source/avengine_internal.h',
		'source/avengine_internal.cc',
		'source/avengine_util.h',
		'source/avengine_util.cc',
	    'interface/avengine_api.h',
		'interface/avengine_types.h',
      ],

      'conditions': [
        ['OS=="ios"', {
          'sources': [
		    'source/avengine_api.cc',
          ],
        }],
        ['OS=="android"', {
          'sources': [
            'source/avengine_api.cc',
          ],
        }],
        ['OS=="win"', {
	       'msvs_settings': {
	         'VCCLCompilerTool': {
		       'DisableSpecificWarnings': ['4251','4996'],
		       'WarnAsError': 'false',
	         },
	         'VCLinkerTool': {
		       # 'ImageHasSafeExceptionHandlers': 0, # /SAFESEH:NO
		       'AdditionalLibraryDirectories': [],
	         },
	       }, # msvs_settings
	    }],
       ],   
       'variables': {
        'clang_warning_flags': [
          '-Wno-incompatible-pointer-types',
          '-Wno-pointer-sign',
          '-Wno-reorder',
          '-Wno-unused-function',
          '-Wno-unused-variable',
          '-Wno-unused-variable',
          '-Wno-writable-strings',
          '-Wno-format-security',
          '-Wno-unused-private-field',
          '-Wno-newline-eof',
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
       ],
    },

    {
      'target_name': 'avengine_dll',
      'type': 'shared_library',
      'dependencies': [
		'avengine',
      ],
      'sources': [
	    'interface/avengine_api.h',
		'interface/ffmpeg_api.h',
		'interface/ios_video_render_api.h',
		'source/loadlib.cc',
      ],
	  'msvs_settings': {
        'VCLinkerTool': {
          'ImageHasSafeExceptionHandlers': 'false',
        },
      },
      'variables': {
        'clang_warning_flags': [
          '-Wno-incompatible-pointer-types',
          '-Wno-pointer-sign',
          '-Wno-reorder',
          '-Wno-unused-function',
          '-Wno-unused-variable',
          '-Wno-unused-variable',
          '-Wno-writable-strings',
          '-Wno-format-security',
          '-Wno-unused-private-field',
          '-Wno-newline-eof',
          '-Wno-sign-compare',
          '-Wno-deprecated-declarations',
          '-Wno-missing-field-initializers',
          '-Wno-invalid-source-encoding',
         ],   
       },        
	  'msvs_disabled_warnings': [
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
       ],
	   'conditions': [
			['OS=="win"', {
				'msvs_settings': {
					'VCCLCompilerTool': {
						'DisableSpecificWarnings': ['4251','4996'],
						'WarnAsError': 'false',
					},
					'VCLinkerTool': {
						# 'ImageHasSafeExceptionHandlers': 0, # /SAFESEH:NO
						'AdditionalLibraryDirectories': [],
					},
				}, # msvs_settings
			}],
			['OS=="win"', {
				'sources': [
					'source/avengine_api.cc',
				],
			}],
		],
	}
  ],
}
