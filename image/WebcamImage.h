#ifndef K_WEBCAMIMAGE_H
#define K_WEBCAMIMAGE_H


#include <cstdint>
#include <cstdlib>
#include "PixelFormat.h"

#include "DataBuffer.h"

namespace K {

	/**
	 * represents an image read from a webcam.
	 * a WebcamImage has
	 *		width and height
	 *		raw data
	 *		a pixel format to describe how the raw-data looks like
	 *
	 * this is just a wrapper to annotate the raw-data
	 * with its width,height and format.
	 *
	 * the WebcamImage (and its data) usually belongs to the Webcam
	 * or an ImageConverter and is used over and over again
	 * to reduce the number of mallocs.
	 * Thus: most of the code is not thread-safe and images might
	 * be overwritten e.g. when webcam.readImage() is called twice.
	 *
	 */
	class WebcamImage {

	public:

		/** create an empty webcam image */
		WebcamImage() :
			width(0), height(0), pixelFormat(0), data() {
			;
		}

		/** dtor */
		~WebcamImage() {
			;
		}


		/** reset all internal values (except data) to zero */
		void reset() {width = 0; height = 0; pixelFormat = PixelFormat(0); data.setBytesUsed(0);}

		/** get the image's width in pixels */
		uint32_t getWidth() const {return width;}

		/** get the image's height in pixels */
		uint32_t getHeight() const {return height;}

		/** get the image's format (e.g. V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_YUYV, ...) */
		PixelFormat getPixelFormat() const {return pixelFormat;}

		/** get the image's size in bytes */
		uint32_t getNumBytes() const  {return data.getBytesUsed();}

		/** get the image's data */
		uint8_t* getData() const {return data.getData();}


		/** set the image's size in bytes */
		void setNumBytes(const uint32_t numBytes) {data.setBytesUsed(numBytes);}

		/** set the image's width in pixels */
		void setWidth(const uint32_t width) {this->width = width;}

		/** set the image's height in pixels */
		void setHeight(const uint32_t height) {this->height = height;}

		/** set the image's format (e.g. V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_YUYV, ...) */
		void setPixelFormat(const PixelFormat pixelFormat) {this->pixelFormat = pixelFormat;}


		/** set several parameters at once */
		void setParameters(const uint32_t width, const uint32_t height, const PixelFormat pixelFormat, const uint32_t usedBytes) {
			setWidth(width);
			setHeight(height);
			setPixelFormat(pixelFormat);
			setNumBytes(usedBytes);
		}

		void ensureSpace(const uint32_t numBytes) {
			data.ensureSpace(numBytes);
		}

		/** move ctor */
		WebcamImage(WebcamImage&& o) {
			this->data = std::move(o.data);
			this->width = o.width;
			this->height = o.height;
			this->pixelFormat = o.pixelFormat;
		}

	private:

		/** hidden copy ctor. not allowed */
		WebcamImage(const WebcamImage& o);


	private:

		friend class Webcam;

		/** the image's width */
		uint32_t width;

		/** the image's height */
		uint32_t height;

		/** the image's pixel format */
		PixelFormat pixelFormat;


		/** internal data storage */
		DataBuffer data;

	};

}

#endif
