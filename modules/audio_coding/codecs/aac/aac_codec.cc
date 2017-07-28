#include "webrtc/modules/audio_coding/codecs/aac/aac_codec.h"

#include "webrtc/avengine/interface/ffmpeg_api.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct AacEncoder {
  void *pAEnc;
  webrtc::CriticalSectionWrapper* critsect;
};

struct AacDecoder {
  void *pADec;
  webrtc::CriticalSectionWrapper* critsect;
};

AacEncoder *aac_encoder_create(const char *cparams, int Fs, int channels, int application, int &nb_samples_per_channel, int *error) {
  AacEncoder *st = (AacEncoder *)calloc(1, sizeof(AacEncoder));
  st->critsect = webrtc::CriticalSectionWrapper::CreateCriticalSection();
  lfrtcAudioEncodeParams encode_params;
  memset(&encode_params, 0, sizeof(encode_params));
  // TODO: zhangle, 32 * 1024 is a magic data
  encode_params.bitrate = 32 * 1024;
  encode_params.channel_count = channels;
  encode_params.sample_rate = Fs;
  lfRtcAudioEncoder *enc = new lfRtcAudioEncoder();
  nb_samples_per_channel = 1024;
  if (enc->Open(&encode_params) < 0) {
    delete enc;
    enc = NULL;
  }
  if (enc) {
    nb_samples_per_channel = enc->GetFrameSamplesPerChannel();
    st->pAEnc = (void*)enc;
    *error = 0;
    return st;
  }
  else {
    *error = -1;
    free(st);
    return NULL;
  }
}

void aac_encoder_destroy(AacEncoder *st) {
  if (st) {
    st->critsect->Enter();
    delete (lfRtcAudioEncoder*)st->pAEnc;
    st->pAEnc = NULL;
    st->critsect->Leave();
    if (st->critsect) {
      delete st->critsect;
      st->critsect = NULL;
    }
    free(st);
  }
}

int aac_encode(AacEncoder *st, const short *pcm, int frame_size,
  unsigned char *data, int max_data_bytes) {
  unsigned char *inBuf[3] = { (unsigned char *)pcm, NULL, NULL };
  int inLen[3] = { frame_size << 2, 0, 0 };
  lfRtcAudioEncoder *enc = (lfRtcAudioEncoder*)st->pAEnc;
  enc->Process(inBuf, inLen);
  int outLen = enc->GetEncodeLen();
  if (outLen > 0 && outLen <= max_data_bytes) {
    memcpy(data, enc->GetEncodeData(), outLen);
    return outLen;
  }
  else {
    return 0;
  }
}

AacDecoder *aac_decoder_create(const char *cparams, int *error) {
  AacDecoder *st = (AacDecoder *)calloc(1, sizeof(AacDecoder));
  st->critsect = webrtc::CriticalSectionWrapper::CreateCriticalSection();
  lfrtcAudioDecodeParams decode_params;
  memset(&decode_params, 0, sizeof(decode_params));
  // TODO: zhangle, magic number
  decode_params.channel_count = 2;
  decode_params.sample_rate = 48000;
  lfRtcAudioDecoder *dec = new lfRtcAudioDecoder();
  if (dec->Open(&decode_params) < 0) {
    delete dec;
    dec = NULL;
  }
  if (dec) {
    st->pADec = (void*)dec;
    *error = 0;
    return st;
  }
  else {
    *error = -1;
    free(st);
    return NULL;
  }
}

int aac_decode(AacDecoder *st, const unsigned char *data,
  int len, short *pcm, int frame_size, int decode_fec) {
  lfRtcAudioDecoder *dec = (lfRtcAudioDecoder*)st->pADec;
  dec->Process((unsigned char*)data, len);
  int outLen = dec->GetDecodeLen();
  if (outLen > 0 && outLen <= frame_size) {
    memcpy(pcm, dec->GetDecodeData(), outLen);
    return outLen;
  }
  else {
    return 0;
  }
}

void aac_decoder_destroy(AacDecoder *st) {
  if (st) {
    st->critsect->Enter();
    delete (lfRtcAudioDecoder*)st->pADec;
    st->pADec = NULL;
    st->critsect->Leave();
    if (st->critsect) {
      delete st->critsect;
      st->critsect = NULL;
    }
    free(st);
  }
}
