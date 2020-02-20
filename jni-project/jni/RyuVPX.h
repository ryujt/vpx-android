#ifndef _RyuVPX_H_
#define	_RyuVPX_H_


#define _Error_General -1;
#define _Error_Allocate_Image -2
#define _Error_Getting_Config -3
#define _Error_Init_VideoCodec -4;


typedef struct _RyuVPX {
	unsigned char errorCode;
    struct vpx_codec_enc_cfg cfgEnc;
	struct vpx_codec_dec_cfg cfgDec;
    vpx_codec_ctx_t codec;
	vpx_image_t	img;
} RyuVPX;


#pragma pack(push,1)

typedef struct _ARGB {
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;
} ARGB;

#pragma pack(pop)


#endif /* _RyuVPX_H_ */
