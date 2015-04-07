#ifndef K_YUV420_RGB24_H
#define K_YUV420_RGB24_H

#include "YUV.h"
#include "../WebcamImage.h"

namespace K {

	/** convert YUV420 -> RGB24 */
	static void convertYUV420toRGB24(const WebcamImage& src, WebcamImage& dst) {

		debug("ImageConverter", "converting YUV420 -> RGB24");

		const uint32_t w = src.getWidth();
		const uint32_t h = src.getHeight();

		const uint8_t* srcBuffer = src.getData();
		dst.ensureSpace(w*h*3);
		uint8_t* dstBuffer = dst.getData();

		// calculate U and V offset within srcData
		const uint32_t offsetU = (w*h);					// start of U part
		const uint32_t offsetV = (w*h) + (w*h/4);			// start of V part

		// translate each pixel
		for (uint32_t y = 0; y < h; ++y) {
			for (uint32_t x = 0; x < w; ++x) {

				const uint32_t offsetY = (y*w + x);
				const uint32_t offsetUV = (y/2*w/2 + x/2);

				// get Y,U,V values
				const int32_t _y =	srcBuffer[ offsetY ];
				const int32_t _u =	srcBuffer[ offsetU + offsetUV ];
				const int32_t _v =	srcBuffer[ offsetV + offsetUV ];

				// convert
				YUVtoRGB(_y, _u, _v,   dstBuffer[offsetY*3+0], dstBuffer[offsetY*3+0], dstBuffer[offsetY*3+2] );

			}
		}

		// set
		dst.setParameters( src.getWidth(), src.getHeight(), PixelFormat(V4L2_PIX_FMT_RGB24), (w*h*3) );

	}

}

#endif // K_YUV420_RGB24_H
