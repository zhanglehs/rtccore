#include "webrtc/modules/video_coding/codecs/h264/main/interface/h264.h"

#include "webrtc/avengine/source/avengine_util.h"
#include "webrtc/modules/av_coding/codecs/ffmpeg/main/source/ffmpeg_factory.h"

H264Decoder::H264Decoder() :
  _decodedImage(),
  _width(0),
  _height(0),
  _inited(false),
  _decodeCompleteCallback(NULL) {
  _pDecHandle = NULL;
  _cb = NULL;
  _chanId = -1;
  _should_decode = false;
  _pH264_critsect = webrtc::CriticalSectionWrapper::CreateCriticalSection();
  _frame_combiner = new RtpPacket2Frame;
}

H264Decoder::~H264Decoder() {
  Release();
  if (_pH264_critsect) {
    delete _pH264_critsect;
    _pH264_critsect = NULL;
  }
  delete _frame_combiner;
}

int
H264Decoder::H264Dec(const webrtc::EncodedImage& inputImage) {
  if (_pDecHandle == NULL && inputImage._length <= 0) {
    return WEBRTC_VIDEO_CIDEC_DECODE_FAIL;
  }
  _frame_combiner->ParseRtpPackets((const char *)inputImage._buffer, inputImage._length);
  if (_frame_combiner->GetFrameData() == NULL || _frame_combiner->GetFrameLen() <= 0) {
    _should_decode = false;
    return WEBRTC_VIDEO_CIDEC_DECODE_FAIL;
  }

  bool is_keyframe = _frame_combiner->IsKeyframe();
  bool frame_complete = _frame_combiner->IsFrameComplete();
  int lost_pkt = _frame_combiner->MissingPacketCount();
  int lastest_seqnum = _frame_combiner->GetLatestSeqNum();
  bool is_continuous = _frame_combiner->IsContinuousWithLast();

  if (!frame_complete) {
    _should_decode = false;
  }
  else if (is_keyframe) {
    _should_decode = true;
  }
  else if (!is_continuous) {
    _should_decode = false;
  }
  lfrtcGlobalConfig *params = GetGlobalConfig();
  if (params && !params->is_lostpacketStrategy) {
    _should_decode = true;
  }
  if (!_should_decode) {
    return WEBRTC_VIDEO_CIDEC_DECODE_FAIL;
  }

  unsigned char *decoded_buf[3];
  decoded_buf[0] = _decodedImage.buffer(webrtc::kYPlane);
  decoded_buf[1] = _decodedImage.buffer(webrtc::kUPlane);
  decoded_buf[2] = _decodedImage.buffer(webrtc::kVPlane);
  int ret = _pDecHandle->Process((unsigned char*)_frame_combiner->GetFrameData(), _frame_combiner->GetFrameLen(), decoded_buf, _width, _height);

  if (ret == lfRtcVideoDecoder::VIDEO_DECODE_SUCCESS_ON_SIZE) {
    _width = _pDecHandle->GetWidth();
    _height = _pDecHandle->GetHeight();
    ExportNotifyMessage(_object, LFRTC_VIDEO_GET_RESOLUTION, _width, _height);
    _decodedImage.Reset();
    int half_width = (_width + 1) / 2;
    _decodedImage.CreateEmptyFrame(_width, _height,
      _width, half_width, half_width);
    decoded_buf[0] = _decodedImage.buffer(webrtc::kYPlane);
    decoded_buf[1] = _decodedImage.buffer(webrtc::kUPlane);
    decoded_buf[2] = _decodedImage.buffer(webrtc::kVPlane);
    memcpy(decoded_buf[0], _pDecHandle->GetExtraDecodeData(0), _width * _height);
    memcpy(decoded_buf[1], _pDecHandle->GetExtraDecodeData(1), _width * _height/4);
    memcpy(decoded_buf[2], _pDecHandle->GetExtraDecodeData(2), _width * _height/4);
  }
  bool got_picture = ret == lfRtcVideoDecoder::VIDEO_DECODE_SUCCESS || ret == lfRtcVideoDecoder::VIDEO_DECODE_SUCCESS_ON_SIZE;
  _should_decode = (ret >= 0);
  int frame_type = _pDecHandle->GetFrameType();

  AVENGINE_TRC("H264Dec, last_seq=%d, timestamp=%u, lostpacket=%d, frame_type=%d", lastest_seqnum, inputImage._timeStamp, lost_pkt, frame_type);

  if (!got_picture) {
    return WEBRTC_VIDEO_CODEC_OK;
  }

  //if (params && params->is_yuvDump)  {
  //  static int decode_frame_count = 0;
  //  decode_frame_count++;
  //
  //  char temp_name[1024];
  //  if (_should_decode) {
  //    sprintf(temp_name, "D://rtp_test//yuv//ffmpegfailed//count_%d_timestamp_%u_last_seq_num_%d_frametype_%d_lost_pkt_%d_%dx%d.yuv",
  //      decode_frame_count, inputImage._timeStamp, lastest_seqnum, frame_type, lost_pkt, _width, _height);
  //  }
  //  else {
  //    if (frame_complete) {
  //      sprintf(temp_name, "D://rtp_test//yuv//success//lostpacket//count_%d_timestamp_%u_last_seq_num_%d_frametype_%d_lost_pkt_%d_%dx%d.yuv",
  //        decode_frame_count, inputImage._timeStamp, lastest_seqnum, frame_type, lost_pkt, _width, _height);
  //    }
  //    else {
  //      sprintf(temp_name, "D://rtp_test//yuv//success//perfect//count_%d_timestamp_%u_last_seq_num_%d_frametype_%d_lost_pkt_%d_%dx%d.yuv",
  //        decode_frame_count, inputImage._timeStamp, lastest_seqnum, frame_type, lost_pkt, _width, _height);
  //    }
  //  }
  //  FILE *debug_fp = fopen(temp_name, "wb");
  //  if (!debug_fp) {
  //    system("md D:\\rtp_test\\yuv\\success\\perfect");
  //    system("md D:\\rtp_test\\yuv\\success\\lostpacket");
  //    system("md D:\\rtp_test\\yuv\\ffmpegfailed");
  //    debug_fp = fopen(temp_name, "wb");
  //  }
  //  if (debug_fp) {
  //    fwrite(decoded_buf[0], _width * _height, 1, debug_fp);
  //    fwrite(decoded_buf[1], (_width >> 1) * (_height >> 1), 1, debug_fp);
  //    fwrite(decoded_buf[2], (_width >> 1) * (_height >> 1), 1, debug_fp);
  //    fclose(debug_fp);
  //  }
  //}

  if (got_picture) {
    if (_cb) {
      ((T_lfrtcDecodedVideoCb)_cb)(_object, (char **)decoded_buf, klfrtcVideoI420, _width, _height);
    }
    ExportNotifyMessage(_object, LFRTC_VIDEO_DECODE_SUCCESS, (int)is_keyframe, 0);

    _decodedImage.set_timestamp(inputImage._timeStamp);
    _decodeCompleteCallback->Decoded(_decodedImage);

    return WEBRTC_VIDEO_CODEC_OK;
  }
  else {
    ExportNotifyMessage(_object, LFRTC_VIDEO_DECODE_FAILED, (int)is_keyframe, 0);
    return WEBRTC_VIDEO_CIDEC_DECODE_FAIL;
  }
}

int
H264Decoder::InitDecode(const webrtc::VideoCodec* codecSettings,
  int /*numberOfCores */) {
  if (codecSettings == NULL/* || codecSettings->width < 1
    || codecSettings->height < 1*/) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  _cb = codecSettings->cb;
  _object = codecSettings->object;
  _chanId = codecSettings->chanId;

  webrtc::CriticalSectionScoped cs(_pH264_critsect);
  lfrtcVideoDecodeParams decoded_params;
  memset(&decoded_params, 0, sizeof(decoded_params));
  _pDecHandle = new lfRtcVideoDecoder();
  if (_pDecHandle->Open(&decoded_params) < 0) {
    delete _pDecHandle;
    _pDecHandle = NULL;
  }
  _inited = true;
  return WEBRTC_VIDEO_CODEC_OK;
}

int
H264Decoder::Decode(const webrtc::EncodedImage& inputImage,
  bool missingFrames,
  const webrtc::RTPFragmentationHeader* fragmentation,
  const webrtc::CodecSpecificInfo* codecSpecificInfo,
  int64_t renderTimeMs) {
  if (inputImage._buffer == NULL || inputImage._length <= 0) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
  if (_decodeCompleteCallback == NULL) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  //if (inputImage._completeFrame == false) {
  //  return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  //}
  if (!_inited) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }

  if (_width > 0 && _height > 0) {
    int half_width = (_width + 1) / 2;
    _decodedImage.CreateEmptyFrame(_width, _height,
      _width, half_width, half_width);
  }

  webrtc::CriticalSectionScoped cs(_pH264_critsect);
  if (_pDecHandle == NULL) {
    return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
  }

  return H264Dec(inputImage);
}

int
H264Decoder::RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback) {
  _decodeCompleteCallback = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

int
H264Decoder::Release() {
  _inited = false;
  webrtc::CriticalSectionScoped cs(_pH264_critsect);
  if (_pDecHandle) {
    delete _pDecHandle;
    _pDecHandle = NULL;
  }
  _decodedImage.Reset();
  return WEBRTC_VIDEO_CODEC_OK;
}

//////////////////////////////////////////////////////////////////////////

H264Encoder::H264Encoder() {
  _inited = false;
  _encodedCompleteCallback = NULL;
  _cb = NULL;
  _object = NULL;
  _chanId = -1;
  _pH264_critsect = webrtc::CriticalSectionWrapper::CreateCriticalSection();
  _EncHandle = NULL;
  _width = 0;
  _height = 0;
}

H264Encoder::~H264Encoder() {
  _inited = false;
  Release();

  if (_pH264_critsect) {
    delete _pH264_critsect;
    _pH264_critsect = NULL;
  }
}

int H264Encoder::Release() {
  webrtc::CriticalSectionScoped cs(_pH264_critsect);
  if (_EncHandle) {
    delete _EncHandle;
    _EncHandle = NULL;
  }

  if (_encodedImage._buffer != NULL) {
    delete[] _encodedImage._buffer;
    _encodedImage._buffer = NULL;
  }

  _inited = false;
  return WEBRTC_VIDEO_CODEC_OK;
}

int H264Encoder::InitEncode(const webrtc::VideoCodec* codecSettings,
  int /*numberOfCores*/,
  size_t /*maxPayloadSize */) {
  if (codecSettings == NULL) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
  if (codecSettings->width < 1 || codecSettings->height < 1
    || strlen(codecSettings->codecSpecific.H264.params) > sizeof(_EncParams)-1) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  webrtc::CriticalSectionScoped cs(_pH264_critsect);

  _cb = codecSettings->cb;
  _object = codecSettings->object;
  _chanId = codecSettings->chanId;
  //strcpy(_EncParams, codecSettings->codecSpecific.H264.params);

  _inited = true;
  return WEBRTC_VIDEO_CODEC_OK;
}

int H264Encoder::Encode(const webrtc::I420VideoFrame& inputImage,
  const webrtc::CodecSpecificInfo* codecSpecificInfo,
  const std::vector<webrtc::VideoFrameType>* /*frame_types*/) {
  if (!_inited) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (_encodedCompleteCallback == NULL) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (inputImage.width() <= 0 || inputImage.height() <= 0) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  webrtc::CriticalSectionScoped cs(_pH264_critsect);

  if (_width != inputImage.width() || _height != inputImage.height()) {
    _width = inputImage.width();
    _height = inputImage.height();
    if (_encodedImage._buffer) {
      delete[] _encodedImage._buffer;
      _encodedImage._buffer = NULL;
    }
    _encodedImage._size = _width * _height * 3/2;  // I420
    _encodedImage._buffer = new uint8_t[_encodedImage._size];
    
    if (_EncHandle) {
      delete _EncHandle;
      _EncHandle = NULL;
    }
    // TODO: zhangle, magic number
    lfrtcVideoEncodeParams encode_params;
    memset(&encode_params, 0, sizeof(encode_params));
    encode_params.width = _width;
    encode_params.height = _height;
    encode_params.fps = 15;
    encode_params.bitrate = 800 * 1024;
    encode_params.gop_size = encode_params.fps;
    _EncHandle = new lfRtcVideoEncoder();
    if (_EncHandle->Open(&encode_params) < 0) {
      delete _EncHandle;
      _EncHandle = NULL;
    }
  }
  if (_EncHandle == NULL) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }

  unsigned char *inBuf[3];
  inBuf[0] = (unsigned char*)inputImage.buffer(webrtc::kYPlane);
  inBuf[1] = (unsigned char*)inputImage.buffer(webrtc::kUPlane);
  inBuf[2] = (unsigned char*)inputImage.buffer(webrtc::kVPlane);
  int inLinesize[3];
  inLinesize[0] = inputImage.stride(webrtc::kYPlane);
  inLinesize[1] = inputImage.stride(webrtc::kUPlane);
  inLinesize[2] = inputImage.stride(webrtc::kVPlane);
  //char *outBuf[3] = { (char*)_encodedImage._buffer, NULL, NULL };
  //int encodedLen = _encodedImage._size;
  //int *outLen[3] = { &encodedLen, NULL, NULL };
  //lfrtcEncodeVideo(_EncHandle, inBuf, inLinesize, outBuf, outLen, &keyframe);
  _EncHandle->Process(inBuf, inLinesize);
  int encodedLen = _EncHandle->GetEncodeLen();
  if (encodedLen > 0) {
    memcpy(_encodedImage._buffer, _EncHandle->GetEncodeData(), encodedLen);
    _encodedImage._length = encodedLen;
    _encodedImage._timeStamp = inputImage.timestamp();
    // TODO: zhangle
    _encodedImage._frameType = webrtc::VideoFrameType::kSkipFrame;
    if (_encodedCompleteCallback) {
      webrtc::RTPFragmentationHeader fragment;
      _encodedCompleteCallback->Encoded(_encodedImage, codecSpecificInfo, &fragment);
    }
    if (_cb) {
      ((T_lfrtcEncodedVideoCb)_cb)(_object, (char*)_encodedImage._buffer, encodedLen, inputImage.timestamp());
    }
  }
  return WEBRTC_VIDEO_CODEC_OK;
}

int
H264Encoder::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback) {
  _encodedCompleteCallback = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}
