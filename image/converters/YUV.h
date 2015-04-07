#ifndef K_YUV_H
#define K_YUV_H

#include "limit.h"

namespace K {

	/** convert the given YUV (pixel) to RGB */
	static void YUVtoRGB(const uint8_t y, const uint8_t u, const uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b) {

		int32_t _C = (int32_t)y - 16;
		int32_t _D = (int32_t)u - 128;
		int32_t _E = (int32_t)v - 128;

		// convert them to RGB
		r = limit8(( 298 * _C            + 409 * _E + 128) >> 8);
		g = limit8(( 298 * _C - 100 * _D - 208 * _E + 128) >> 8);
		b = limit8(( 298 * _C + 516 * _D            + 128) >> 8);

	}

}

#endif
