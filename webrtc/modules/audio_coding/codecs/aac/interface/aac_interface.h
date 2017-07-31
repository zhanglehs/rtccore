/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_AUDIO_CODING_CODECS_AAC_INTERFACE_AAC_INTERFACE_H_
#define WEBRTC_MODULES_AUDIO_CODING_CODECS_AAC_INTERFACE_AAC_INTERFACE_H_

#include "webrtc/typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
	typedef int64_t __int64;
#endif


// Opaque wrapper types for the codec state.
typedef struct WebRtcAacEncInst AacEncInst;
typedef struct WebRtcAacDecInst AacDecInst;

/****************************************************************************
 * WebRtcAac_EncoderCreate(...)
 *
 * This function create an Aac encoder.
 *
 * Input:
 *      - channels           : number of channels.
 *      - application        : 0 - VOIP applications.
 *                                 Favor speech intelligibility.
 *                             1 - Audio applications.
 *                                 Favor faithfulness to the original input.
 *
 * Output:
 *      - inst               : a pointer to Encoder context that is created
 *                             if success.
 *
 * Return value              : 0 - Success
 *                            -1 - Error
 */
int16_t WebRtcAac_EncoderCreate(AacEncInst** inst,
  int32_t channels, int &nb_samples_per_channel,
								 char *params);

int16_t WebRtcAac_EncoderFree(AacEncInst* inst);

/****************************************************************************
 * WebRtcAac_Encode(...)
 *
 * This function encodes audio as a series of Aac frames and inserts
 * it into a packet. Input buffer can be any length.
 *
 * Input:
 *      - inst                  : Encoder context
 *      - audio_in              : Input speech data buffer
 *      - samples               : Samples per channel in audio_in
 *      - length_encoded_buffer : Output buffer size
 *
 * Output:
 *      - encoded               : Output compressed data buffer
 *
 * Return value                 : >=0 - Length (in bytes) of coded data
 *                                -1 - Error
 */
int16_t WebRtcAac_Encode(AacEncInst* inst,
                          const int16_t* audio_in,
                          int16_t samples,
                          int16_t length_encoded_buffer,
                          uint8_t* encoded);

/****************************************************************************
 * WebRtcAac_SetBitRate(...)
 *
 * This function adjusts the target bitrate of the encoder.
 *
 * Input:
 *      - inst               : Encoder context
 *      - rate               : New target bitrate
 *
 * Return value              :  0 - Success
 *                             -1 - Error
 */
int16_t WebRtcAac_SetBitRate(AacEncInst* inst, int32_t rate);

/****************************************************************************
 * WebRtcAac_SetPacketLossRate(...)
 *
 * This function configures the encoder's expected packet loss percentage.
 *
 * Input:
 *      - inst               : Encoder context
 *      - loss_rate          : loss percentage in the range 0-100, inclusive.
 * Return value              :  0 - Success
 *                             -1 - Error
 */
int16_t WebRtcAac_SetPacketLossRate(AacEncInst* inst, int32_t loss_rate);

/****************************************************************************
 * WebRtcAac_SetMaxPlaybackRate(...)
 *
 * Configures the maximum playback rate for encoding. Due to hardware
 * limitations, the receiver may render audio up to a playback rate. Aac
 * encoder can use this information to optimize for network usage and encoding
 * complexity. This will affect the audio bandwidth in the coded audio. However,
 * the input/output sample rate is not affected.
 *
 * Input:
 *      - inst               : Encoder context
 *      - frequency_hz       : Maximum playback rate in Hz.
 *                             This parameter can take any value. The relation
 *                             between the value and the Aac internal mode is
 *                             as following:
 *                             frequency_hz <= 8000           narrow band
 *                             8000 < frequency_hz <= 12000   medium band
 *                             12000 < frequency_hz <= 16000  wide band
 *                             16000 < frequency_hz <= 24000  super wide band
 *                             frequency_hz > 24000           full band
 * Return value              :  0 - Success
 *                             -1 - Error
 */
int16_t WebRtcAac_SetMaxPlaybackRate(AacEncInst* inst, int32_t frequency_hz);

/* TODO(minyue): Check whether an API to check the FEC and the packet loss rate
 * is needed. It might not be very useful since there are not many use cases and
 * the caller can always maintain the states. */

/****************************************************************************
 * WebRtcAac_EnableFec()
 *
 * This function enables FEC for encoding.
 *
 * Input:
 *      - inst               : Encoder context
 *
 * Return value              :  0 - Success
 *                             -1 - Error
 */
int16_t WebRtcAac_EnableFec(AacEncInst* inst);

/****************************************************************************
 * WebRtcAac_DisableFec()
 *
 * This function disables FEC for encoding.
 *
 * Input:
 *      - inst               : Encoder context
 *
 * Return value              :  0 - Success
 *                             -1 - Error
 */
int16_t WebRtcAac_DisableFec(AacEncInst* inst);

/****************************************************************************
 * WebRtcAac_EnableDtx()
 *
 * This function enables Aac internal DTX for encoding.
 *
 * Input:
 *      - inst               : Encoder context
 *
 * Return value              :  0 - Success
 *                             -1 - Error
 */
int16_t WebRtcAac_EnableDtx(AacEncInst* inst);

/****************************************************************************
 * WebRtcAac_DisableDtx()
 *
 * This function disables Aac internal DTX for encoding.
 *
 * Input:
 *      - inst               : Encoder context
 *
 * Return value              :  0 - Success
 *                             -1 - Error
 */
int16_t WebRtcAac_DisableDtx(AacEncInst* inst);

/*
 * WebRtcAac_SetComplexity(...)
 *
 * This function adjusts the computational complexity. The effect is the same as
 * calling the complexity setting of Aac as an Aac encoder related CTL.
 *
 * Input:
 *      - inst               : Encoder context
 *      - complexity         : New target complexity (0-10, inclusive)
 *
 * Return value              :  0 - Success
 *                             -1 - Error
 */
int16_t WebRtcAac_SetComplexity(AacEncInst* inst, int32_t complexity);

int16_t WebRtcAac_DecoderCreate(AacDecInst** inst, int channels, int chanId, void *cb, void *object, const char *cparams);
int16_t WebRtcAac_DecoderFree(AacDecInst* inst);

/****************************************************************************
 * WebRtcAac_DecoderChannels(...)
 *
 * This function returns the number of channels created for Aac decoder.
 */
int WebRtcAac_DecoderChannels(AacDecInst* inst);

/****************************************************************************
 * WebRtcAac_DecoderInit(...)
 *
 * This function resets state of the decoder.
 *
 * Input:
 *      - inst               : Decoder context
 *
 * Return value              :  0 - Success
 *                             -1 - Error
 */
int16_t WebRtcAac_DecoderInit(AacDecInst* inst);

/****************************************************************************
 * WebRtcAac_Decode(...)
 *
 * This function decodes an Aac packet into one or more audio frames at the
 * ACM interface's sampling rate (32 kHz).
 *
 * Input:
 *      - inst               : Decoder context
 *      - encoded            : Encoded data
 *      - encoded_bytes      : Bytes in encoded vector
 *
 * Output:
 *      - decoded            : The decoded vector
 *      - audio_type         : 1 normal, 2 CNG (for Aac it should
 *                             always return 1 since we're not using Aac's
 *                             built-in DTX/CNG scheme)
 *
 * Return value              : >0 - Samples per channel in decoded vector
 *                             -1 - Error
 */
int16_t WebRtcAac_Decode(AacDecInst* inst, const uint8_t* encoded,
                          int16_t encoded_bytes, int16_t* decoded,
                          int16_t* audio_type);

/****************************************************************************
 * WebRtcAac_DecodePlc(...)
 *
 * This function processes PLC for opus frame(s).
 * Input:
 *        - inst                  : Decoder context
 *        - number_of_lost_frames : Number of PLC frames to produce
 *
 * Output:
 *        - decoded               : The decoded vector
 *
 * Return value                   : >0 - number of samples in decoded PLC vector
 *                                  -1 - Error
 */
int16_t WebRtcAac_DecodePlc(AacDecInst* inst, int16_t* decoded,
                             int16_t number_of_lost_frames);

/****************************************************************************
 * WebRtcAac_DecodeFec(...)
 *
 * This function decodes the FEC data from an Aac packet into one or more audio
 * frames at the ACM interface's sampling rate (32 kHz).
 *
 * Input:
 *      - inst               : Decoder context
 *      - encoded            : Encoded data
 *      - encoded_bytes      : Bytes in encoded vector
 *
 * Output:
 *      - decoded            : The decoded vector (previous frame)
 *
 * Return value              : >0 - Samples per channel in decoded vector
 *                              0 - No FEC data in the packet
 *                             -1 - Error
 */
int16_t WebRtcAac_DecodeFec(AacDecInst* inst, const uint8_t* encoded,
                             int16_t encoded_bytes, int16_t* decoded,
                             int16_t* audio_type);

/****************************************************************************
 * WebRtcAac_DurationEst(...)
 *
 * This function calculates the duration of an opus packet.
 * Input:
 *        - inst                 : Decoder context
 *        - payload              : Encoded data pointer
 *        - payload_length_bytes : Bytes of encoded data
 *
 * Return value                  : The duration of the packet, in samples per
 *                                 channel.
 */
int WebRtcAac_DurationEst(AacDecInst* inst,
                           const uint8_t* payload,
                           int payload_length_bytes);

/* TODO(minyue): Check whether it is needed to add a decoder context to the
 * arguments, like WebRtcAac_DurationEst(...). In fact, the packet itself tells
 * the duration. The decoder context in WebRtcAac_DurationEst(...) is not used.
 * So it may be advisable to remove it from WebRtcAac_DurationEst(...). */

/****************************************************************************
 * WebRtcAac_FecDurationEst(...)
 *
 * This function calculates the duration of the FEC data within an opus packet.
 * Input:
 *        - payload              : Encoded data pointer
 *        - payload_length_bytes : Bytes of encoded data
 *
 * Return value                  : >0 - The duration of the FEC data in the
 *                                 packet in samples per channel.
 *                                  0 - No FEC data in the packet.
 */
int WebRtcAac_FecDurationEst(AacDecInst* inst,
                             const uint8_t* payload,
                             int payload_length_bytes);

/****************************************************************************
 * WebRtcAac_PacketHasFec(...)
 *
 * This function detects if an opus packet has FEC.
 * Input:
 *        - payload              : Encoded data pointer
 *        - payload_length_bytes : Bytes of encoded data
 *
 * Return value                  : 0 - the packet does NOT contain FEC.
 *                                 1 - the packet contains FEC.
 */
int WebRtcAac_PacketHasFec(const uint8_t* payload,
                            int payload_length_bytes);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // WEBRTC_MODULES_AUDIO_CODING_CODECS_OPUS_INTERFACE_OPUS_INTERFACE_H_
