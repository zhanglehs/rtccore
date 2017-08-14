#ifndef WEBRTC_MODULES_VIDEO_CODING_CODECS_I420_H_
#define WEBRTC_MODULES_VIDEO_CODING_CODECS_I420_H_

#include "webrtc/modules/video_coding/codecs/interface/video_codec_interface.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"

#ifndef _WIN32
typedef int64_t __int64;
#endif

class RtpPacket2Frame;
class lfRtcVideoDecoder;
class lfRtcVideoEncoder;

class H264Decoder : public webrtc::VideoDecoder {
public:
  H264Decoder();
  virtual ~H264Decoder();

  // Initialize the decoder.
  // The user must notify the codec of width and height values.
  //
  // Return value         :  WEBRTC_VIDEO_CODEC_OK.
  //                        <0 - Errors
  int InitDecode(const webrtc::VideoCodec* codecSettings,
    int /*numberOfCores*/) override;

  // Decode encoded image (as a part of a video stream). The decoded image
  // will be returned to the user through the decode complete callback.
  //
  // Input:
  //          - inputImage        : Encoded image to be decoded
  //          - missingFrames     : True if one or more frames have been lost
  //                                since the previous decode call.
  //          - codecSpecificInfo : pointer to specific codec data
  //          - renderTimeMs      : Render time in Ms
  //
  // Return value                 : WEBRTC_VIDEO_CODEC_OK if OK
  //                                 <0 - Error
  int Decode(const webrtc::EncodedImage& inputImage,
    bool missingFrames,
    const webrtc::RTPFragmentationHeader* fragmentation,
    const webrtc::CodecSpecificInfo* codecSpecificInfo,
    int64_t renderTimeMs) override;

  // Register a decode complete callback object.
  //
  // Input:
  //          - callback         : Callback object which handles decoded images.
  //
  // Return value                : WEBRTC_VIDEO_CODEC_OK if OK, < 0 otherwise.
  int RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback) override;

  // Free decoder memory.
  //
  // Return value                : WEBRTC_VIDEO_CODEC_OK if OK.
  //                                  <0 - Error
  int Release() override;

  // Reset decoder state and prepare for a new call.
  //
  // Return value         :  WEBRTC_VIDEO_CODEC_OK.
  //                          <0 - Error
  int Reset() override { return WEBRTC_VIDEO_CODEC_OK; }

private:
  int H264Dec(const webrtc::EncodedImage& inputImage);

  webrtc::I420VideoFrame _decodedImage;
  int _width;
  int _height;
  bool _inited;
  webrtc::DecodedImageCallback* _decodeCompleteCallback;
  lfRtcVideoDecoder *_pDecHandle;
  webrtc::CriticalSectionWrapper* _pH264_critsect;
  void *_cb;
  void *_object;
  long long _chanId;
  RtpPacket2Frame *_frame_combiner;
  bool _should_decode;
}; // End of WebRtcH264Decoder class.

class H264Encoder : public webrtc::VideoEncoder {
public:

  H264Encoder();

  virtual ~H264Encoder();

  // Initialize the encoder with the information from the VideoCodec.
  //
  // Input:
  //          - codecSettings     : Codec settings.
  //          - numberOfCores     : Number of cores available for the encoder.
  //          - maxPayloadSize    : The maximum size each payload is allowed
  //                                to have. Usually MTU - overhead.
  //
  // Return value                 : WEBRTC_VIDEO_CODEC_OK if OK.
  //                                <0 - Error
  int InitEncode(const webrtc::VideoCodec* codecSettings,
    int /*numberOfCores*/,
    size_t /*maxPayloadSize*/) override;

  // "Encode" an I420 image (as a part of a video stream). The encoded image
  // will be returned to the user via the encode complete callback.
  //
  // Input:
  //          - inputImage        : Image to be encoded.
  //          - codecSpecificInfo : Pointer to codec specific data.
  //          - frameType         : Frame type to be sent (Key /Delta).
  //
  // Return value                 : WEBRTC_VIDEO_CODEC_OK if OK.
  //                                <0 - Error
  int Encode(const webrtc::I420VideoFrame& inputImage,
    const webrtc::CodecSpecificInfo* /*codecSpecificInfo*/,
    const std::vector<webrtc::VideoFrameType>* /*frame_types*/) override;

  // Register an encode complete callback object.
  //
  // Input:
  //          - callback         : Callback object which handles encoded images.
  //
  // Return value                : WEBRTC_VIDEO_CODEC_OK if OK, < 0 otherwise.
  int RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback) override;

  // Free encoder memory.
  //
  // Return value                : WEBRTC_VIDEO_CODEC_OK if OK, < 0 otherwise.
  int Release();

  int SetRates(uint32_t newBitRate, uint32_t frameRate) override
  {
    return WEBRTC_VIDEO_CODEC_OK;
  }

  int SetChannelParameters(uint32_t packetLoss, int64_t rtt) override
  {
    return WEBRTC_VIDEO_CODEC_OK;
  }

  int CodecConfigParameters(uint8_t* buffer, int size) override
  {
    return WEBRTC_VIDEO_CODEC_OK;
  }

private:
  bool _inited;
  webrtc::EncodedImage _encodedImage;
  webrtc::EncodedImageCallback* _encodedCompleteCallback;
  void *_cb;
  void *_object;
  long long _chanId;
  webrtc::CriticalSectionWrapper* _pH264_critsect;
  lfRtcVideoEncoder *_EncHandle;
  int _width;
  int _height;

public:
  char _EncParams[PARAMS_SIZE];
}; // end of WebRtcI420DEncoder class

#endif // WEBRTC_MODULES_VIDEO_CODING_CODECS_I420_H_
