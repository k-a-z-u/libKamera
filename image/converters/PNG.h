#ifndef K_PNG_H
#define K_PNG_H

#include <unistd.h>
#include <png.h>
#include "../WebcamImage.h"
#include "../ConverterException.h"

namespace K {

	static void PngWriteCallback(png_structp png, png_bytep data, png_size_t length) {
		WebcamImage* dst = (WebcamImage*) png_get_io_ptr(png);
		uint8_t* ptr = &dst->getData()[dst->getNumBytes()];
		memcpy(ptr, data, length);
		dst->setNumBytes(dst->getNumBytes() + length);
	}

	static void convertToPNG(const WebcamImage& src, WebcamImage& dst) {

		png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png) {throw "Could not allocate write struct\n";}

		png_infop info_ptr = png_create_info_struct(png);
		if (!info_ptr) {throw "error 2";}

		if (setjmp(png_jmpbuf(png))) {throw "error 3";}

		const int w = src.getWidth();
		const int h = src.getHeight();
		dst.reset();
		dst.setParameters(w, h, PixelFormat(0), 0);

		// configure the output pixel format depending on the input format
		int colorFormat;
		switch(src.getPixelFormat()._int) {
			case V4L2_PIX_FMT_GREY:		colorFormat = PNG_COLOR_TYPE_GRAY; dst.ensureSpace(w*h+4096); break;
			case V4L2_PIX_FMT_RGB24:	colorFormat = PNG_COLOR_TYPE_RGB; dst.ensureSpace(w*h*3+4096); break;
			default: throw ConverterException("unsupported input format", src.getPixelFormat());
		}

		// configure
		png_set_IHDR(png, info_ptr, w, h, 8, colorFormat, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		// create an array containing the start of each row within the image
		uint8_t* rows[h];
		for (int y = 0; y < h; ++y) {
			switch(src.getPixelFormat()._int) {
				case V4L2_PIX_FMT_GREY:		rows[y] = &src.getData()[y*w]; break;
				case V4L2_PIX_FMT_RGB24:	rows[y] = &src.getData()[y*w*3]; break;
			}
		}

		// set the rows to encode
		png_set_rows(png, info_ptr, &rows[0]);

		// write
		png_set_write_fn(png, (void*) &dst, PngWriteCallback, NULL);

		// finalize
		png_write_png(png, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	}

}

#endif // K_PNG_H
