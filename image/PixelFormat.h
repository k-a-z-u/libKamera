#ifndef K_PIXELFORMAT_H
#define K_PIXELFORMAT_H

#include <cstdint>
#include <iostream>
#include "PixelFormats.h"

namespace K {

	/**
	 * this union describtes the pixel format used within one webcam image.
	 * those are usually given as 32-bit ints and contain 4 chars uniquely
	 * describing the pixel format.
	 *
	 * common formats are e.g.
	 *	V4L2_PIX_FMT_RGB24 -> "RGB3" -> RGB-8-8-8
	 *	V4L2_PIX_FMT_YUYV  -> "YUYV" -> YUV 4:2:2
	 *
	 */
	union PixelFormat {

		/** get the pixel format as 32-bit int */
		uint32_t _int;

		/** get the pixel format as 4 chars */
		char _chars[4];

	public:

		/** empty ctor */
		PixelFormat() : _int(0) {;}

		/** ctor from 32-bit int */
		explicit PixelFormat(const uint32_t _int) : _int(_int) {
			;
		}

		/** ctor from 4 chars */
		explicit PixelFormat(const char c3, const char c2, const char c1, const char c0) {
			_chars[3] = c3; _chars[2] = c2; _chars[1] = c1; _chars[0] = c0;
		}

		/** comparison */
		bool operator == (const PixelFormat& o) const {
			return o._int == _int;
		}

		/** get format as string */
		std::string asString() const {
			const std::string fmt(_chars, 4);
			return "'" + fmt + "'";
		}

		/** toString */
		friend std::ostream& operator << (std::ostream& out, const PixelFormat& pf) {
			out << pf.asString();
			return out;
		}


	};

}

#endif // K_PIXELFORMAT_H
