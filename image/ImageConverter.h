#ifndef K_IMAGECONVERTER_H
#define K_IMAGECONVERTER_H

#include "WebcamImage.h"

#include <stdio.h>
#include <stdlib.h>

#include "PixelFormat.h"

#include "../Debug.h"
#include "ConverterException.h"

#include <linux/videodev2.h>
#include <cstring>

#include "converters/Yxx_RGB24.h"
#include "converters/Yxx_Yxx.h"
#include "converters/YUYV_RGB24.h"
#include "converters/YUV420_RGB24.h"
#include "converters/YUV420_YUV24.h"
#include "converters/YUV.h"
#include "converters/MJPEG_JPEG.h"
#include "converters/JPEG.h"
#include "converters/limit.h"

#define IMG_CONV_NUM_BUFFERS	2

namespace K {

	/**
	 * helper class for image conversion to/from ofen-used formats.
	 *
	 * uses internal buffers to reduce mallocs. all returned images belong
	 * to the converter and might change at any time!
	 * -> the next conversion MUST only be called if the data of the last one
	 * is not used any more.
	 *
	 * -> create several ImageConverters if concurrent conversions are needed
	 * or copy the result immediately
	 *
	 */
	class ImageConverter {

	private:

		/** pre-allocated buffers that can be used to reduce mallocs */
		WebcamImage buffers[IMG_CONV_NUM_BUFFERS];

	public:

		/** -------------------------------- OFTEN USED CONVERSIONS -------------------------------- */


		/**
		 * convert the given WebcamImage to RGB (if conversion is possible)
		 * BEWARE! the returned webcam image is volatile and its data belongs to the converter!
		 * The next call to getRGB() (and others) might overwrite the contained data!
		 * either only convert one image at a time, copy every converted image immediately
		 * or use multiple instances of ImageConverter!
		 * @param src the input WebcamImage
		 * @return the output WebcamImage in RGB format
		 */
		WebcamImage& getRGB(const WebcamImage& src) const {

			WebcamImage& dst = getEmptyImage();

			// convert
			switch (src.getPixelFormat()._int) {
				case V4L2_PIX_FMT_YUV420:	convertYUV420toRGB24(src, dst); break;
				case V4L2_PIX_FMT_YUYV:		convertYUYVtoRGB24(src, dst); break;
				case V4L2_PIX_FMT_Y12:		convertYxxToRGB24(12, src, dst); break;
				case V4L2_PIX_FMT_Y11:		convertYxxToRGB24(11, src, dst); break;
				case V4L2_PIX_FMT_Y16:		convertYxxToRGB24(16, src, dst); break;
				default:					throw ConverterException(src.getPixelFormat());
			}

			return dst;

		}

		/** convert a WebcamImage to JPEG */
		WebcamImage& getJPEG(const WebcamImage& src, uint8_t quality) const {

			// convert
			switch (src.getPixelFormat()._int) {

				case V4L2_PIX_FMT_YUV420: {
					convertYUV420toYUV24(src, (WebcamImage&) buffers[0]);
					convertToJPEG(buffers[0], (WebcamImage&) buffers[1], quality);
					return (WebcamImage&) buffers[1];
				}

				case V4L2_PIX_FMT_GEPJ: {
					debug("ImageConverter", "is already a JPEG ;)");
					return (WebcamImage&) src;
				}

				case V4L2_PIX_FMT_MJPEG: {
					convertMJPEGtoJPEG(src, (WebcamImage&) buffers[0]);
					return (WebcamImage&) buffers[0];
				}

				default: {
					throw ConverterException(src.getPixelFormat());
					//return nullptr;
				}

			}

		}


	private:

		/** get the next, empty, writeable image, using one of the internal data buffers */
		WebcamImage& getEmptyImage() const {
			static int idx = 0;
			idx = (idx + 1) % IMG_CONV_NUM_BUFFERS;
			WebcamImage& img = (WebcamImage&) buffers[idx];
			img.reset();
			return img;
		}

	};

}

#endif
