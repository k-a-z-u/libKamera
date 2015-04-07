#ifndef K_JPEG_H
#define K_JPEG_H

/** helper to convert a WebcamImage to JPEG */

#include <jerror.h>
#include <jpeglib.h>

#include "../PixelFormat.h"
#include "../../Debug.h"
#include "../ConverterException.h"
#include "../WebcamImage.h"

namespace K {

	static boolean jpegBufferOverflow (j_compress_ptr cinfo) {
		(void) cinfo;
		throw ConverterException("jpeg compressor: out of memory");
	}

	static void jpegDummy (j_compress_ptr cinfo) {
		(void) cinfo;
	}

	/** convert JCS_RGB / JCS_YCbCr to JPEG */
	static void convertToJPEG(const WebcamImage& src, WebcamImage& dst, const uint8_t quality) {

		debug("ImageConverter", "converting to JPEG")

		// temporals
		J_COLOR_SPACE srcFormat;
		int numComponents;
		int stride;

		// get the input format
		switch (src.getPixelFormat()._int) {
			case V4L2_PIX_FMT_YUV420:	srcFormat = JCS_YCbCr;		numComponents = 3; stride = src.getWidth() * 3; break;
			case V4L2_PIX_FMT_GREY:		srcFormat = JCS_GRAYSCALE;	numComponents = 1; stride = src.getWidth() * 1; break;
			case V4L2_PIX_FMT_RGB24:	srcFormat = JCS_RGB;		numComponents = 3; stride = src.getWidth() * 3; break;
			default: throw ConverterException("jpeg does not support this input format", src.getPixelFormat());
		}

		const int maxSize = src.getWidth() * src.getHeight() * numComponents;
		dst.ensureSpace(maxSize);
		const uint8_t* srcBuffer = src.getData();
		uint8_t* dstBuffer = dst.getData();

		struct jpeg_compress_struct cinfo;
		struct jpeg_error_mgr jerr;
		struct jpeg_destination_mgr jdest;

		cinfo.err = jpeg_std_error (&jerr);
		jpeg_create_compress (&cinfo);
		jdest.next_output_byte = dstBuffer;
		jdest.free_in_buffer = maxSize;
		jdest.init_destination = jpegDummy;
		jdest.empty_output_buffer = jpegBufferOverflow;
		jdest.term_destination = jpegDummy;
		cinfo.dest = &jdest;

		// set image-information (width/height) and output parameters (quality)
		cinfo.image_width = src.getWidth();
		cinfo.image_height = src.getHeight();
		cinfo.input_components = numComponents;
		cinfo.in_color_space = srcFormat;
		jpeg_set_defaults (&cinfo);
		jpeg_set_quality (&cinfo, quality, TRUE);

		// start compression
		jpeg_start_compress (&cinfo, TRUE);

		// compress each scanline
		while (cinfo.next_scanline < src.getHeight()) {
			JSAMPROW row = (JSAMPROW)(srcBuffer + cinfo.next_scanline * stride);
			jpeg_write_scanlines (&cinfo, &row, 1);
		}

		// done -> cleanup
		jpeg_finish_compress (&cinfo);
		jpeg_destroy_compress (&cinfo);

		// return jpeg's file-size
		dst.setParameters( src.getWidth(), src.getHeight(), PixelFormat(V4L2_PIX_FMT_JPEG), (maxSize - jdest.free_in_buffer) );

	}

}

#endif
