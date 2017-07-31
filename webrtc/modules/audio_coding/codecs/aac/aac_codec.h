#ifndef AAC_CODEC_H
#define AAC_CODEC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AacEncoder AacEncoder;
typedef struct AacDecoder AacDecoder;

AacEncoder *aac_encoder_create(const char *cparams, int Fs, int channels, int application, int &nb_samples_per_channel, int *error);
int aac_encode(AacEncoder *st, const short *pcm, int frame_size, unsigned char *data, int max_data_bytes);
void aac_encoder_destroy(AacEncoder *st);
AacDecoder *aac_decoder_create(const char *cparams, int *error);
int aac_decode(AacDecoder *st, const unsigned char *data, int len, short *pcm, int frame_size, int decode_fec);
void aac_decoder_destroy(AacDecoder *st);

#ifdef __cplusplus
}
#endif

#endif
