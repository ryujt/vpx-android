#include "yuvTools.h"

void RGBtoYUV420(const unsigned char* rgb, unsigned char* yuv,
		int srcFrameWidth, int srcFrameHeight, int pixelSize)
{
	unsigned int planeSize;
	unsigned int halfWidth;

	unsigned char *yplane;
	unsigned char *uplane;
	unsigned char *vplane;
	const unsigned char *rgbIndex;

	int x, y;
	unsigned char *yline;
	unsigned char *uline;
	unsigned char *vline;

	planeSize = srcFrameWidth * srcFrameHeight;
	halfWidth = srcFrameWidth >> 1;

	yplane = yuv;
	uplane = yuv + planeSize;
	vplane = yuv + planeSize + (planeSize >> 2);
	rgbIndex = rgb;

	for (y = 0; y < srcFrameHeight; y++) {
		yline = yplane + (y * srcFrameWidth);
		uline = uplane + ((y >> 1) * halfWidth);
		vline = vplane + ((y >> 1) * halfWidth);

		for (x = 0; x < (int) srcFrameWidth; x += 2) {
			rgbtoyuv(rgbIndex[0], rgbIndex[1], rgbIndex[2], *yline, *uline,	*vline);
			rgbIndex += pixelSize;
			yline++;
			rgbtoyuv(rgbIndex[0], rgbIndex[1], rgbIndex[2], *yline, *uline,	*vline);
			rgbIndex += pixelSize;
			yline++;
			uline++;
			vline++;
		}
	}
}

inline unsigned char Clip(int val) {
    if (val < 0) {
        return 0;
    } else if (val > 255) {
        return 255;
    }

    return val & 0xFF;
}

int I420ToARGB(const unsigned char *src_y, int src_stride_y,
                const unsigned char *src_u, int src_stride_u,
                const unsigned char *src_v, int src_stride_v,
                unsigned char *dst_frame, int dst_stride_frame,
                int width, int height) {

  if ((src_y == 0) || (src_u == 0) || (src_v == 0) || (dst_frame == 0)) {
    return 0;
  }

  // TODO(fbarchard): support inversion
  unsigned char* out = dst_frame;
  unsigned char* out2 = out + dst_stride_frame;
  int h, w;
  int tmp_r, tmp_g, tmp_b;
  const unsigned char *y1, *y2 ,*u, *v;
  y1 = src_y;
  y2 = y1 + src_stride_y;
  u = src_u;
  v = src_v;
  for (h = ((height + 1) >> 1); h > 0; h--){
    // 2 rows at a time, 2 y's at a time
    for (w = 0; w < ((width + 1) >> 1); w++){
      // Vertical and horizontal sub-sampling
      tmp_r = (int)((mapYc[y1[0]] + mapVcr[v[0]] + 128) >> 8);
      tmp_g = (int)((mapYc[y1[0]] + mapUcg[u[0]] + mapVcg[v[0]] + 128) >> 8);
      tmp_b = (int)((mapYc[y1[0]] + mapUcb[u[0]] + 128) >> 8);
      out[0] = Clip(tmp_b);
      out[1] = Clip(tmp_g);
      out[2] = Clip(tmp_r);

      tmp_r = (int)((mapYc[y1[1]] + mapVcr[v[0]] + 128) >> 8);
      tmp_g = (int)((mapYc[y1[1]] + mapUcg[u[0]] + mapVcg[v[0]] + 128) >> 8);
      tmp_b = (int)((mapYc[y1[1]] + mapUcb[u[0]] + 128) >> 8);
      out[4] = Clip(tmp_b);
      out[5] = Clip(tmp_g);
      out[6] = Clip(tmp_r);

      tmp_r = (int)((mapYc[y2[0]] + mapVcr[v[0]] + 128) >> 8);
      tmp_g = (int)((mapYc[y2[0]] + mapUcg[u[0]] + mapVcg[v[0]] + 128) >> 8);
      tmp_b = (int)((mapYc[y2[0]] + mapUcb[u[0]] + 128) >> 8);
      out2[0] = Clip(tmp_b);
      out2[1] = Clip(tmp_g);
      out2[2] = Clip(tmp_r);

      tmp_r = (int)((mapYc[y2[1]] + mapVcr[v[0]] + 128) >> 8);
      tmp_g = (int)((mapYc[y2[1]] + mapUcg[u[0]] + mapVcg[v[0]] + 128) >> 8);
      tmp_b = (int)((mapYc[y2[1]] + mapUcb[u[0]] + 128) >> 8);
      out2[4] = Clip(tmp_b);
      out2[5] = Clip(tmp_g);
      out2[6] = Clip(tmp_r);

      out += 8;
      out2 += 8;
      y1 += 2;
      y2 += 2;
      u++;
      v++;
    }
    y1 += 2 * src_stride_y - width;
    y2 += 2 * src_stride_y - width;
    u += src_stride_u - ((width + 1) >> 1);
    v += src_stride_v - ((width + 1) >> 1);
    out += dst_stride_frame;
    out2 += dst_stride_frame;
  }

  return 1;
}
