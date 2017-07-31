/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "webrtc/modules/audio_coding/codecs/aac/interface/audio_encoder_aac.h"

#include "webrtc/base/checks.h"
#include "webrtc/common_types.h"
#include "webrtc/modules/audio_coding/codecs/aac/interface/aac_interface.h"
#include "webrtc/avengine/source/avengine_util.h"
#ifdef _WIN32
#pragma warning(disable:4189)
#endif
namespace webrtc {

namespace {

const int kMinBitrateBps = 500;
const int kMaxBitrateBps = 512000;

// TODO(tlegrand): Remove this code when we have proper APIs to set the
// complexity at a higher level.
#if defined(WEBRTC_ANDROID) || defined(WEBRTC_IOS) || defined(WEBRTC_ARCH_ARM)
// If we are on Android, iOS and/or ARM, use a lower complexity setting as
// default, to save encoder complexity.
const int kDefaultComplexity = 5;
#else
const int kDefaultComplexity = 9;
#endif

// We always encode at 48 kHz.
const int kSampleRateHz = 48000;

int16_t ClampInt16(size_t x) {
  return static_cast<int16_t>(
    std::min(x, static_cast<size_t>(std::numeric_limits<int16_t>::max())));
}

}  // namespace

AudioEncoderAac::Config::Config()
: frame_size_ms(20),
  num_channels(1),
  payload_type(120),
  application(kVoip),
  bitrate_bps(64000),
  fec_enabled(false),
  max_playback_rate_hz(48000),
  complexity(kDefaultComplexity),
  dtx_enabled(false) {
  channelid = -1;
  cb = NULL;
  object = NULL;
}

bool AudioEncoderAac::Config::IsOk() const {
  if (num_channels != 1 && num_channels != 2)
    return false;
  if (bitrate_bps < kMinBitrateBps || bitrate_bps > kMaxBitrateBps)
    return false;
  if (complexity < 0 || complexity > 10)
    return false;
  if (dtx_enabled && application != kVoip)
    return false;
  return true;
}

AudioEncoderAac::AudioEncoderAac(const Config& config)
: num_10ms_frames_per_packet_((config.frame_size_ms / 10)),
  num_channels_(config.num_channels),
  payload_type_(config.payload_type),
  application_(config.application),
  bitrate_bps_(config.bitrate_bps),
  dtx_enabled_(config.dtx_enabled),
  packet_loss_rate_(0.0),
  samples_per_10ms_frame_((kSampleRateHz / 100) * num_channels_),
  buf_data_len_(0),
  last_timestamp_in_buffer_(0) {
  channelid_ = config.channelid;
  cb_ = config.cb;
  object_ = config.object;
  config.IsOk();
  strcpy(params, config.params);
  inst_ = NULL;
  WebRtcAac_EncoderCreate(&inst_, num_channels_, nb_samples_per_channel_, params);
  input_buffer_.resize(nb_samples_per_channel_ * 4 * 2);
  SetTargetBitrate(config.bitrate_bps);
  if (config.fec_enabled) {
    WebRtcAac_EnableFec(inst_);
  }
  else {
    WebRtcAac_DisableFec(inst_);
  }
  WebRtcAac_SetMaxPlaybackRate(inst_, config.max_playback_rate_hz);
  WebRtcAac_SetComplexity(inst_, config.complexity);
  if (config.dtx_enabled) {
    WebRtcAac_EnableDtx(inst_);
  }
  else {
    WebRtcAac_DisableDtx(inst_);
  }
}

AudioEncoderAac::~AudioEncoderAac() {
  WebRtcAac_EncoderFree(inst_);
}

int AudioEncoderAac::SampleRateHz() const {
  return kSampleRateHz;
}

int AudioEncoderAac::NumChannels() const {
  return num_channels_;
}

size_t AudioEncoderAac::MaxEncodedBytes() const {
  // Calculate the number of bytes we expect the encoder to produce,
  // then multiply by two to give a wide margin for error.
  int frame_size_ms = num_10ms_frames_per_packet_ * 10;
  int bytes_per_millisecond = bitrate_bps_ / (1000 * 8) + 1;
  size_t approx_encoded_bytes =
    static_cast<size_t>(frame_size_ms * bytes_per_millisecond);
  return 2 * approx_encoded_bytes;
}

int AudioEncoderAac::Num10MsFramesInNextPacket() const {
  return num_10ms_frames_per_packet_;
}

int AudioEncoderAac::Max10MsFramesInAPacket() const {
  return num_10ms_frames_per_packet_;
}

void AudioEncoderAac::SetTargetBitrate(int bits_per_second) {
  bitrate_bps_ = std::max(std::min(bits_per_second, kMaxBitrateBps),
    kMinBitrateBps);
  WebRtcAac_SetBitRate(inst_, bitrate_bps_);
}

void AudioEncoderAac::SetProjectedPacketLossRate(double fraction) {
  DCHECK_GE(fraction, 0.0);
  DCHECK_LE(fraction, 1.0);
  // Optimize the loss rate to configure Aac. Basically, optimized loss rate is
  // the input loss rate rounded down to various levels, because a robustly good
  // audio quality is achieved by lowering the packet loss down.
  // Additionally, to prevent toggling, margins are used, i.e., when jumping to
  // a loss rate from below, a higher threshold is used than jumping to the same
  // level from above.
  const double kPacketLossRate20 = 0.20;
  const double kPacketLossRate10 = 0.10;
  const double kPacketLossRate5 = 0.05;
  const double kPacketLossRate1 = 0.01;
  const double kLossRate20Margin = 0.02;
  const double kLossRate10Margin = 0.01;
  const double kLossRate5Margin = 0.01;
  double opt_loss_rate;
  if (fraction >=
    kPacketLossRate20 +
    kLossRate20Margin *
    (kPacketLossRate20 - packet_loss_rate_ > 0 ? 1 : -1)) {
    opt_loss_rate = kPacketLossRate20;
  }
  else if (fraction >=
    kPacketLossRate10 +
    kLossRate10Margin *
    (kPacketLossRate10 - packet_loss_rate_ > 0 ? 1 : -1)) {
    opt_loss_rate = kPacketLossRate10;
  }
  else if (fraction >=
    kPacketLossRate5 +
    kLossRate5Margin *
    (kPacketLossRate5 - packet_loss_rate_ > 0 ? 1 : -1)) {
    opt_loss_rate = kPacketLossRate5;
  }
  else if (fraction >= kPacketLossRate1) {
    opt_loss_rate = kPacketLossRate1;
  }
  else {
    opt_loss_rate = 0;
  }

  if (packet_loss_rate_ != opt_loss_rate) {
    WebRtcAac_SetPacketLossRate(inst_, static_cast<int32_t>(opt_loss_rate * 100 + .5));
    packet_loss_rate_ = opt_loss_rate;
  }
}

AudioEncoder::EncodedInfo AudioEncoderAac::EncodeInternal(
  uint32_t rtp_timestamp,
  const int16_t* audio,
  size_t max_encoded_bytes,
  uint8_t* encoded) {
  char *buf = &input_buffer_[0];
  int frameBytes = (nb_samples_per_channel_ << 2);
  if (rtp_timestamp - last_timestamp_in_buffer_ > (uint32_t)frameBytes) {
    AVENGINE_INF("audio capture interval is too large");
    buf_data_len_ = 0;
  }
  last_timestamp_in_buffer_ = rtp_timestamp;

  int size = samples_per_10ms_frame_ * sizeof(int16_t);
  // TODO: zhangle, divide 4 means 16bits and channels=2
  rtp_timestamp -= buf_data_len_ / 4;
  memcpy(&buf[buf_data_len_], audio, size);
  buf_data_len_ += size;

  if (buf_data_len_ < frameBytes) {
    return EncodedInfo();
  }
  int16_t status = WebRtcAac_Encode(
    inst_,
    (const short *)buf,
    (frameBytes >> 2),
    ClampInt16(max_encoded_bytes),
    encoded);
  buf_data_len_ -= frameBytes;
  if (buf_data_len_ > 0) {
    memmove(buf, &buf[frameBytes], buf_data_len_);
  }

  if (status > 0 && cb_) {
    ((T_lfrtcEncodedAudioCb)cb_)(object_, (char*)encoded, status, rtp_timestamp);
  }

  EncodedInfo info;
  info.encoded_bytes = status;
  info.encoded_timestamp = rtp_timestamp;
  info.payload_type = payload_type_;
  info.send_even_if_empty = true;  // Allows Aac to send empty packets.
  info.speech = (status > 0);
  return info;
}

}  // namespace webrtc
