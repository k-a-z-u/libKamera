/*
 * YUYV_RGB24.h
 *
 *  Created on: Jun 14, 2014
 *      Author: kazu
 */

#ifndef K_YUYV_RGB24_H_
#define K_YUYV_RGB24_H_

#include "YUV.h"
#include "../WebcamImage.h"

namespace K {

	/** convert YUYV (YUV422) to RGB24 */
	static void convertYUYVtoRGB24(const WebcamImage& src, WebcamImage& dst) {

		const uint32_t w = src.getWidth();
		const uint32_t h = src.getHeight();

		const uint8_t* srcBuffer = src.getData();
		dst.ensureSpace(w*h*3);
		uint8_t* dstBuffer = dst.getData();

		// calculate U and V offset within srcData
		//uint32_t offsetU = (w*h);					// start of U part
		//uint32_t offsetV = (w*h) + (w*h/4);			// start of V part

		// translate each pixel
		for (uint32_t y = 0; y < h; ++y) {
			for (uint32_t x = 0; x < w; ++x) {

				const uint8_t Y  = srcBuffer[(x     + y*w)*2+0];
				const uint8_t Cb = srcBuffer[(x/2*4 + y*w*2)+1];
				const uint8_t Cr = srcBuffer[(x/2*4 + y*w*2)+3];

				int rgbIDX = (x+y*w)*3;

				YUVtoRGB(Y, Cb, Cr, dstBuffer[rgbIDX+0], dstBuffer[rgbIDX+1], dstBuffer[rgbIDX+2]);

			}
		}

		// set
		dst.setParameters( src.getWidth(), src.getHeight(), PixelFormat(V4L2_PIX_FMT_RGB24), (w*h*3) );

	}

}

#endif /* YUYV_RGB24_H_ */
