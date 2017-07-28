/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "webrtc/modules/audio_coding/codecs/aac/interface/aac_interface.h"

#include "webrtc/avengine/source/avengine_util.h"
#include "webrtc/modules/audio_coding/codecs/aac/aac_codec.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>

#define AUDIO_SAMPLE_RATE 48000

enum {
  /* Maximum supported frame size in WebRTC is 60 ms. */
	kWebRtcAacMaxEncodeFrameSizeMs = 60,

	/* The format allows up to 120 ms frames. Since we don't control the other
	* side, we must allow for packets of that size. NetEq is currently limited
	* to 60 ms on the receive side. */
	kWebRtcAacMaxDecodeFrameSizeMs = 120,

	/* Maximum sample count per channel is 48 kHz * maximum frame size in
	* milliseconds. */
	kWebRtcAacMaxFrameSizePerChannel = 8192,// 48 * kWebRtcAacMaxDecodeFrameSizeMs,

	/* Default frame size, 20 ms @ 48 kHz, in samples (for one channel). */
	kWebRtcAacDefaultFrameSize = 2048,// 960,
};

struct WebRtcAacEncInst {
  AacEncoder* encoder;
};

struct WebRtcAacDecInst {
  AacDecoder* decoder;
  int prev_decoded_samples;
  void *cb;
  void *object;
  int chanId;
  int channels;
};

int16_t WebRtcAac_EncoderCreate(AacEncInst** inst,
  int32_t channels, int &nb_samples_per_channel,
								                char *cparams) {
  if (inst != NULL) {
    AacEncInst* state = (AacEncInst*)calloc(1, sizeof(AacEncInst));
    int application = (channels == 1) ? 0 : 1;
    if (state) {
      int aac_app;
      switch (application) {
      case 0:
        aac_app = 1;// OPUS_APPLICATION_VOIP;
        break;
      case 1:
        aac_app = 2;// OPUS_APPLICATION_AUDIO;
        break;
      default:
        free(state);
        return -1;
      }

      int error = 0;
      int sampleRate = AUDIO_SAMPLE_RATE;
      state->encoder = aac_encoder_create(cparams, sampleRate, channels, aac_app, nb_samples_per_channel, &error);
      if (error == 0 && state->encoder != NULL) {
        *inst = state;
        return 0;
      }
      free(state);
    }
  }
  return -1;
}

int16_t WebRtcAac_EncoderFree(AacEncInst* inst) {
  if (inst) {
    aac_encoder_destroy(inst->encoder);
    free(inst);
    return 0;
  } else {
    return -1;
  }
}

int16_t WebRtcAac_Encode(AacEncInst* inst,
                         const int16_t* audio_in,
                         int16_t samples,
                         int16_t length_encoded_buffer,
                         uint8_t* encoded) {
  int res;
  short* audio = (short*)audio_in;
  unsigned char* coded = encoded;
  if (samples > AUDIO_SAMPLE_RATE / 1000 * kWebRtcAacMaxEncodeFrameSizeMs) {
    return 0;
  }
  if (inst == NULL) {
    return 0;
  }

  res = aac_encode(inst->encoder, audio, samples, coded,
	  length_encoded_buffer);
  if (res > 0) {
	  return res;
  }
  return 0;
}

int16_t WebRtcAac_SetBitRate(AacEncInst* inst, int32_t rate) {
	return 0;
}

int16_t WebRtcAac_SetPacketLossRate(AacEncInst* inst, int32_t loss_rate) {
	return 0;
}

int16_t WebRtcAac_SetMaxPlaybackRate(AacEncInst* inst, int32_t frequency_hz) {
  return 0;
}

int16_t WebRtcAac_EnableFec(AacEncInst* inst) {
	return 0;
}

int16_t WebRtcAac_DisableFec(AacEncInst* inst) {
	return 0;
}

int16_t WebRtcAac_EnableDtx(AacEncInst* inst) {
  return 0;
}

int16_t WebRtcAac_DisableDtx(AacEncInst* inst) {
	return 0;
}

int16_t WebRtcAac_SetComplexity(AacEncInst* inst, int32_t complexity) {
	return 0;
}

int16_t WebRtcAac_DecoderCreate(AacDecInst** inst, int channels, int chanId, void *cb, void *object, const char *cparams) {
  if (inst != NULL) {
    AacDecInst* state = (AacDecInst*)calloc(1, sizeof(AacDecInst));
    if (state == NULL) {
      return -1;
    }

    int error = 0;
    state->decoder = aac_decoder_create(cparams, &error);
    if (error == 0 && state->decoder != NULL) {
      state->channels = channels;
      state->prev_decoded_samples = kWebRtcAacDefaultFrameSize;
      state->cb = cb;
      state->object = object;
      state->chanId = chanId;
      *inst = state;
      return 0;
    }
    free(state);
  }
  return -1;
}

int16_t WebRtcAac_DecoderFree(AacDecInst* inst) {
  if (inst) {
    aac_decoder_destroy(inst->decoder);
    free(inst);
    return 0;
  } else {
    return -1;
  }
}

int WebRtcAac_DecoderChannels(AacDecInst* inst) {
  return inst->channels;
}

int16_t WebRtcAac_DecoderInit(AacDecInst* inst) {
	return 0;
}

/* For decoder to determine if it is to output speech or comfort noise. */
static int16_t DetermineAudioType(AacDecInst* inst, int16_t encoded_bytes) {
	return 0;
}

/* |frame_size| is set to maximum Aac frame size in the normal case, and
 * is set to the number of samples needed for PLC in case of losses.
 * It is up to the caller to make sure the value is correct. */
static int DecodeNative(AacDecInst* inst, const uint8_t* encoded,
                        int16_t encoded_bytes, int frame_size,
                        int16_t* decoded, int16_t* audio_type, int decode_fec) {
  int res = aac_decode(inst->decoder, encoded, encoded_bytes,
                       (int16_t*)decoded, frame_size, decode_fec);
  if (res <= 0) {
    ExportNotifyMessage(inst->object, LFRTC_AUDIO_DECODE_FAILED, 0, 0);
    return -1;
  }
  ExportNotifyMessage(inst->object, LFRTC_AUDIO_DECODE_SUCCESS, 0, 0);
  *audio_type = 0;// DetermineAudioType(inst, encoded_bytes);
  if (res > 0) {
	  return res >> 2;
  }
  return res;
}

int16_t WebRtcAac_Decode(AacDecInst* inst, const uint8_t* encoded,
                          int16_t encoded_bytes, int16_t* decoded,
                          int16_t* audio_type) {
  int decoded_samples;
  if (encoded_bytes == 0) {
    *audio_type = DetermineAudioType(inst, encoded_bytes);
    decoded_samples = WebRtcAac_DecodePlc(inst, decoded, 1);
  } else {
    decoded_samples = DecodeNative(inst,
                                   encoded,
                                   encoded_bytes,
                                   kWebRtcAacMaxFrameSizePerChannel,
                                   decoded,
                                   audio_type,
                                   0);
  }
  if (decoded_samples < 0) {
    return -1;
  }

  int16_t output_samples = decoded_samples;
  //if (inst->cb != NULL)
  //{
	 // char *dst[3];	dst[0] = (char *)decoded;
	 // inst->cb(inst->object, 0, inst->chanId, dst, output_samples << 2, 0);
	 // output_samples = -1;//0;
  //}
  if (inst->cb) {
    ((T_lfrtcDecodedAudioCb)inst->cb)(inst->object, (char *)decoded, output_samples << 2);
  }

  /* Update decoded sample memory, to be used by the PLC in case of losses. */
  inst->prev_decoded_samples = decoded_samples;

  return output_samples;
}

int16_t WebRtcAac_DecodePlc(AacDecInst* inst, int16_t* decoded,
                            int16_t number_of_lost_frames) {
	return -1;
}

int16_t WebRtcAac_DecodeFec(AacDecInst* inst, const uint8_t* encoded,
                            int16_t encoded_bytes, int16_t* decoded,
                            int16_t* audio_type) {
  return inst->prev_decoded_samples;
}

int WebRtcAac_DurationEst(AacDecInst* inst,
                           const uint8_t* payload,
                           int payload_length_bytes) {
  return inst->prev_decoded_samples;
}

int WebRtcAac_FecDurationEst(AacDecInst* inst,
                             const uint8_t* payload,
                             int payload_length_bytes) {
  return inst->prev_decoded_samples;
}

int WebRtcAac_PacketHasFec(const uint8_t* payload,
                           int payload_length_bytes) {
  return 0;
}
