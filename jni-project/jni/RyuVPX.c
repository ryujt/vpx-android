#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>


#include "vpx/vpx_codec.h"
#include "vpx/vpx_encoder.h"
#include "vpx/vpx_decoder.h"
#include "vpx/vp8cx.h"
#include "vpx/vp8dx.h"
#include "vpx/vpx_image.h"


#include "RyuVPX.h"
#include "yuvTools.h"


#define interfaceEnc (vpx_codec_vp8_cx())
#define interfaceDec (vpx_codec_vp8_dx())


const int _PixelSize = 4;


JNIEXPORT jint Java_ryulib_VideoZip_VPX_OpenEncoder(JNIEnv* env,
		jclass clazz, jint width, jint height, jint bitRate, int fps, int gop)
{
	RyuVPX *pHandle = (RyuVPX *) malloc(sizeof(RyuVPX));
	
	pHandle->errorCode = 0;

	if (!vpx_img_alloc(&pHandle->img, VPX_IMG_FMT_I420 , width, height, 1)) {
		pHandle->errorCode = _Error_Allocate_Image;
		goto EXIT;
	}

	if (vpx_codec_enc_config_default(interfaceEnc, &pHandle->cfgEnc, 0)) {
		pHandle->errorCode = _Error_Getting_Config;
		goto EXIT;
	}

	pHandle->cfgEnc.g_w = width;
	pHandle->cfgEnc.g_h = height;
	pHandle->cfgEnc.rc_target_bitrate = bitRate;

	if (0 != bitRate) {
		pHandle->cfgEnc.rc_target_bitrate = bitRate;
	} else {
		pHandle->cfgEnc.rc_target_bitrate = width * height * pHandle->cfgEnc.rc_target_bitrate  / pHandle->cfgEnc.g_w / pHandle->cfgEnc.g_h;    
	}

	if (0 != gop) {
		pHandle->cfgEnc.kf_max_dist = gop;
	} 

	if (-1 == gop) {
		pHandle->cfgEnc.kf_mode = VPX_KF_DISABLED;
	}

	if (vpx_codec_enc_init(&pHandle->codec, interfaceEnc, &pHandle->cfgEnc, 0)) {
		pHandle->errorCode = _Error_Init_VideoCodec;
		goto EXIT;
	}

EXIT:

	return pHandle;
}


JNIEXPORT void Java_ryulib_VideoZip_VPX_CloseEncoder(JNIEnv* env,
		jclass clazz, jint handle)
{
	RyuVPX *pHandle = (RyuVPX *) handle;

	// TODO: 초기화 되었는 지 플래그를 둘 것인지 등 고민
//	if (0 != pHandle->img) vpx_img_free(&pHandle->img);
//	if (0 != pHandle->codec) vpx_codec_destroy(&pHandle->codec);

	free(handle);
}


JNIEXPORT jint Java_ryulib_VideoZip_VPX_EncodeBitmap(JNIEnv* env,
		jclass clazz, jint handle, jobject bitmap, jbyteArray buffer, jint bufferSize, jint deadline)
{
	RyuVPX *pHandle = (RyuVPX *) handle;

	jbyte *pByteBuffer = (*env)->GetByteArrayElements(env, buffer, 0);

	int packet_size = 0;
	int frame_cnt = 0;
	int flags = 0;

	unsigned long ulDeadline = VPX_DL_GOOD_QUALITY;

	switch (deadline) {
		case 0: ulDeadline = VPX_DL_REALTIME; break;
		case 1: ulDeadline = VPX_DL_GOOD_QUALITY; break;
		case 2: ulDeadline = VPX_DL_BEST_QUALITY; break;
	}

	void *pixelBitmap;
	if (AndroidBitmap_lockPixels(env, bitmap, &pixelBitmap) >= 0) {
		RGBtoYUV420((unsigned char*) pixelBitmap, pHandle->img.planes[0], pHandle->cfgEnc.g_w, pHandle->cfgEnc.g_h, _PixelSize);
		int encodeResult = vpx_codec_encode(&pHandle->codec, &pHandle->img, frame_cnt, 1, flags, ulDeadline);

		AndroidBitmap_unlockPixels(env, bitmap);

		if (encodeResult) goto EXIT;
	}

	const vpx_codec_cx_pkt_t *pPacket;
	vpx_codec_iter_t iter = NULL;
	unsigned char *pFrame = (unsigned char *) pByteBuffer;
	int *pFrameSize;

	while ( (pPacket = (vpx_codec_get_cx_data(&pHandle->codec, &iter))) ) {
		if ((packet_size + sizeof(int) + pPacket->data.frame.sz) >= bufferSize) goto EXIT;

		switch (pPacket->kind) {
			case VPX_CODEC_CX_FRAME_PKT: {
				pFrameSize = (int *) pFrame;
				*pFrameSize = pPacket->data.frame.sz;
				pFrame = pFrame + sizeof(int);

				memcpy(pFrame, pPacket->data.frame.buf, pPacket->data.frame.sz);
				pFrame = pFrame + pPacket->data.frame.sz;

				packet_size = packet_size + sizeof(int) + pPacket->data.frame.sz;
			} break;

			default: break;
		}
	}

EXIT:
	(*env)->ReleaseByteArrayElements(env, buffer, pByteBuffer, 0);

	return packet_size;
}


JNIEXPORT jint Java_ryulib_VideoZip_VPX_OpenDecoder(JNIEnv* env,
		jclass clazz, jint width, jint height)
{
	RyuVPX *pHandle = (RyuVPX *) malloc(sizeof(RyuVPX));

	pHandle->errorCode = 0;

	pHandle->cfgDec.w = width;
	pHandle->cfgDec.h = height;
	pHandle->cfgDec.threads = 16;

	int flags = 0;

	if (vpx_codec_dec_init(&pHandle->codec, interfaceDec, &pHandle->cfgDec, flags)) {
		pHandle->errorCode = _Error_Init_VideoCodec;
		goto EXIT;
	}

EXIT:

	return pHandle;
}


JNIEXPORT void Java_ryulib_VideoZip_VPX_CloseDecoder(JNIEnv* env,
		jclass clazz, jint handle)
{
	RyuVPX *pHandle = (RyuVPX *) handle;

	// TODO: 초기화 되었는 지 플래그를 둘 것인지 등 고민
//	if (0 != pHandle->codec) vpx_codec_destroy(&pHandle->codec);
//	if (0 != pHandle->pPacketSliceMerge) releasePacketSliceMerge(pHandle->pPacketSliceMerge);

	free(pHandle);
}


JNIEXPORT void Java_ryulib_VideoZip_VPX_InitDecoder(JNIEnv* env,
		jclass clazz, jint handle)
{
	RyuVPX *pHandle = (RyuVPX *) handle;

	int flags = 0;
	vpx_codec_dec_init(&pHandle->codec, interfaceDec, &pHandle->cfgDec, flags);
}


JNIEXPORT jint Java_ryulib_VideoZip_VPX_DecodeBitmap(JNIEnv* env,
		jclass clazz, jint handle, jobject bitmap, jbyteArray buffer, jint bufferSize)
{
	int result = 0;

	RyuVPX *pHandle = (RyuVPX *) handle;

	jbyte *pByteBuffer = (*env)->GetByteArrayElements(env, buffer, 0);

	unsigned char *pFrame = (unsigned char *) pByteBuffer;
	int *pFrameSize;
	int frameSize = 0;
	int count = 0;
	vpx_image_t *img;
	vpx_codec_iter_t iter;

	while (count < bufferSize) {
		pFrameSize = (int *) pFrame;
		frameSize = *pFrameSize;
		pFrame = pFrame + sizeof(int);

		if (vpx_codec_decode(&pHandle->codec, (unsigned char*) pFrame, frameSize, NULL, 0)) {
		goto EXIT;
		}
		pFrame = pFrame + frameSize;

		count = count + sizeof(int) + frameSize;

		iter = NULL;
		while((img = vpx_codec_get_frame(&pHandle->codec, &iter))) {
			void *pixelBitmap;
			if (AndroidBitmap_lockPixels(env, bitmap, &pixelBitmap) >= 0) {
				I420ToARGB(
					(unsigned char *) img->planes[0], img->stride[0],
					(unsigned char *) img->planes[1], img->stride[1],
					(unsigned char *) img->planes[2], img->stride[2],
					(unsigned char *) pixelBitmap,
					img->d_w * _PixelSize,
					img->d_w, img->d_h
				);

				AndroidBitmap_unlockPixels(env, bitmap);
			}

			result = 1;
		}
	}

EXIT:
	(*env)->ReleaseByteArrayElements(env, buffer, pByteBuffer, 0);

	return result;
}
