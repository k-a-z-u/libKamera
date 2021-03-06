#ifndef K_YXX_YXX_H
#define K_YXX_YXX_H

#include "../WebcamImage.h"

namespace K {

	/** convert from Yxx (xx-bit grey-scale) to Y08 */
	static void convertYxxToY08(const int numBits, const WebcamImage& src, WebcamImage& dst) {

		debug("ImageConverter", "converting Y" << numBits << " -> Y08");

		const uint32_t w = src.getWidth();
		const uint32_t h = src.getHeight();

		const uint8_t* srcBuffer = src.getData();
		dst.ensureSpace(w*h);
		uint8_t* dstBuffer = dst.getData();

		// translate each pixel
		for (uint32_t y = 0; y < h; ++y) {
			for (uint32_t x = 0; x < w; ++x) {

				const uint32_t srcIdx = (x + y*w) * 2;
				const uint32_t dstIdx = (x + y*w);

				// 16 bit src value (highest bits are unused)
				const uint16_t g16 = ((uint16_t)srcBuffer[srcIdx+0] << 0) | ((uint16_t)srcBuffer[srcIdx+1] << 8);

				// convert to 8 bit grey
				const uint8_t  g8 = g16 >> (numBits - 8);

				dstBuffer[ dstIdx ] = g8;

			}
		}

		// set
		dst.setParameters( w, h, PixelFormat(V4L2_PIX_FMT_GREY), (w*h) );

	}

}

#endif // K_YXX_YXX_H
