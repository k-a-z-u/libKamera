#ifndef K_PIXELFORMATS_H
#define K_PIXELFORMATS_H

#include <linux/videodev2.h>

/* some formats seem to be missing in the v4l headers.. */
#define V4L2_PIX_FMT_GEPJ		0x4745504A
#define V4L2_PIX_FMT_YUV24		(uint32_t)('Y'<<24|'U'<<16|'V'<<8|'3');
#define V4L2_PIX_FMT_Y11		v4l2_fourcc('Y', '1', '1', ' ') /* 11  Greyscale     */

#endif
