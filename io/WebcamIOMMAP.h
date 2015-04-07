#ifndef K_WEBCAMIO_MMAP_H
#define K_WEBCAMIO_MMAP_H

#include "WebcamIO.h"
#include <string>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <unistd.h>

#include "../Debug.h"
#include "WebcamException.h"

namespace K {

	/** buffer to store temporal data */
	struct WebcamIOMMAPBuffer {
		void* start;
		size_t  length;
	};

	/**
	 * webcam-IO using Memory-Mapped buffers
	 */
	class WebcamIOMMAP : public WebcamIO {

	private:

		uint32_t maxImageSize;

	public:

		/** ctor */
		WebcamIOMMAP(const int fd, const std::string& dev) : fd(fd), dev(dev), buffers(nullptr), numBuffers(0) {
			;
		}

		/** dtor */
		~WebcamIOMMAP() {

			// remove all mem-map buffers
			free(buffers);

		}

		void init(const uint32_t maxImageSize) override {

			debug(dev, "\tinitializing MMAP-IO");

			this->maxImageSize = maxImageSize;

			// request 4 MMAP buffers for capturing from the webcam
			// (you need at least 2/3 or you will have funny image glitches)
			struct v4l2_requestbuffers req;
			CLEAR(req);
			req.count = 4;
			req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			req.memory = V4L2_MEMORY_MMAP;
			int ret = WebcamIO::xioctl(fd, VIDIOC_REQBUFS, &req);

			// check if everything went fine
			if (ret == EINVAL)	{throw WebcamException("does not support MMAP", dev);}
			if (ret != 0)		{throw WebcamException("other error while requesting MMAP", dev);}
			if (req.count < 2)	{throw WebcamException("insufficient buffer memory", dev);}
			numBuffers = req.count; 										// store for later use

			debug(dev, "\tdriver allocated "+std::to_string(numBuffers)+" buffers");

			// allocate buffers for reading
			buffers = (struct WebcamIOMMAPBuffer*) calloc(req.count, sizeof(*buffers));
			if (!buffers) {throw WebcamException("out of memory", dev);}

			// initialize all acquired buffers
			for (uint32_t i = 0; i < req.count; ++i) {

				// set-up current buffer
				struct v4l2_buffer buf;
				CLEAR(buf);
				buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory      = V4L2_MEMORY_MMAP;
				buf.index       = i;
				if (WebcamIO::xioctl(fd, VIDIOC_QUERYBUF, &buf) != 0) {
					throw WebcamException("error while querying buffer", dev, errno);
				}

				// set-up MMAP
				buffers[i].length = buf.length;
				buffers[i].start =	mmap(NULL, buf.length,
						PROT_READ | PROT_WRITE,	/* required */
						MAP_SHARED,				/* recommended */
						fd, buf.m.offset);

				if (MAP_FAILED == buffers[i].start) {throw WebcamException("error while creating MMAP", dev);}

				// TODO: add munmap
				//http://linuxtv.org/downloads/v4l-dvb-apis/mmap.html

			}

		}

		void start() override {

			// START CAPTURING
			debug(dev, "\tstarting MMAP-IO (-> start streaming)");

			// initialize all buffers for Capture using MMAP
			for (uint32_t i = 0; i < numBuffers; ++i) {

				struct v4l2_buffer buf;
				CLEAR(buf);
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				buf.index = i;
				if (WebcamIO::xioctl(fd, VIDIOC_QBUF, &buf) != 0) {throw WebcamException("error: VIDIOC_QBUF", dev);}

			}

			// start the stream
			enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (WebcamIO::xioctl(fd, VIDIOC_STREAMON, &type) != 0) {throw WebcamException("error while starting stream", dev);}

		}

		void read(DataBuffer& dst) override {

			debug(dev, "reading image (using MMAP-IO)");

			struct v4l2_buffer buf;
			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;

			// read until available
			while(true) {
				int ret = WebcamIO::xioctl(fd, VIDIOC_DQBUF, &buf);
				if		(ret == 0)		{break;}										// image available -> proceed
				else if	(ret == EAGAIN)	{std::cout << "."; usleep(2000); continue;}		// wait 2ms and try again
				else					{throw WebcamException("error while reading image", dev, errno);}		// error
			}

			// sanity check
			if (buf.index >= numBuffers) {throw WebcamException("buffer index out of range", dev);}

			// return the buffer containing the data
			//buffers[buf.index].length = buf.bytesused;
			//return buffers[buf.index];

			// we copy this buffer to a temporal one, as the driver might
			// already be overwriting it while the user still reads from it..
			// this results in very funny image glitches ;)
			dst.ensureSpace(maxImageSize);
			memcpy(dst.getData(), buffers[buf.index].start, buf.bytesused);
			dst.setBytesUsed(buf.bytesused);

			// re-enque the buffer (make it usable again)
			if (WebcamIO::xioctl(fd, VIDIOC_QBUF, &buf) != 0) {throw WebcamException("error while querying buffer", dev, errno);}

		}

		void stop() override {

			debug(dev, "\tstopping MMAP-IO (-> stop streaming)");

			// stop the stream
			enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			WebcamIO::xioctl(fd, VIDIOC_STREAMOFF, &type);

		}

		void uninit() override {

			debug(dev, "\tun-initializing MMAP-IO");

			// m-unmp all buffers
			for (uint32_t i = 0; i < numBuffers; ++i) {
				if (munmap(buffers[i].start, buffers[i].length) == -1) {
					throw WebcamException("error while munmapping memory", dev, errno);
				}
			}

		}

	private:

		/** the file-descriptor for accessing the device */
		const int fd;

		/** the device name */
		std::string dev;

		/** the buffers to use for reading */
		struct WebcamIOMMAPBuffer* buffers;
		uint32_t numBuffers;

		/**
		 * we copy any aquired image to this temporal buffer
		 * to ensure the driver does not overwrite the image
		 * while the user still reads data from it
		 */
		struct DataBuffer copyBuffer;

	};

}

#endif
