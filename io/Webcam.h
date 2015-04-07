#ifndef K_WEBCAM_H
#define K_WEBCAM_H

#include <vector>
#include <fcntl.h>

#include "../image/WebcamImage.h"
#include "../image/PixelFormat.h"

#include "WebcamIO.h"
#include "WebcamIORW.h"
#include "WebcamIOMMAP.h"

#include "../Debug.h"
#include "WebcamException.h"

namespace K {

	/**
	 * handles to interaction with a webcam device (e.g. "/dev/video0")
	 * allows:
	 *	configuring the webcam to use one of the supported resolution/pixel-format.
	 *	retrieving images from the webcam
	 *
	 * the retrieved images (their memory) belongs to this class.
	 * retrieving the next image, overwrites the previous one! (only 1 buffer)
	 *
	 * usage:
	 *	open
	 *	setFormat
	 *	init
	 *	start
	 *	readImage, readImage, readImage...
	 *	stop
	 *	uninit
	 *	close
	 *
	 */
	class Webcam {

	public:

		/**
		 * ctor with device name
		 * @param dev the linux device name (e.g. "/dev/video0") to open
		 */
		Webcam(const std::string& dev) :
			dev(dev), io(0), fd(0), isOpen(false), isRunning(false), isInitialized(false) {
			;
		}

		/** dtor */
		~Webcam() {

			debug(dev, "destroying");

			// properly cleaning things up
			stop();
			uninit();
			close();
			if (io != 0) {delete io;}

		}

		/**
		 * open the webcam (if not yet open)
		 * this will open the webcam's device (e.g. "/dev/video0") and check
		 * whether this actually is a webcam device
		 */
		void open() {

			if (isOpen) {return;}

			// open a file-descriptor to the camera
			debug(dev, "opening");
			fd = ::open(dev.c_str(), O_RDWR | O_NONBLOCK, 0);
			if (fd == -1) {throw WebcamException("error while opening device", dev, errno);}

			// check if this is a V4L2 device and read its capabilities
			checkIsWebcam();

			// read all supported pixel formats into a vector
			readSupportedPixelFormats();

			isOpen = true;
		}

		/**
		 * initialize the webcam
		 * this method will get the best IO method for capturing
		 * images from this webcam
		 */
		void init() {
			if (isInitialized) {return;}
			if (!isOpen) {throw WebcamException("open() the webcam first!", dev);}
			if (io == nullptr) {io = getBestIO();}
			io->init(fmt.fmt.pix.sizeimage);
			isInitialized = true;
		}

		/** start capturing */
		void start() {
			if (!isInitialized) {throw WebcamException("init() the webcam first!", dev);}
			if (isRunning) {return;}
			io->start();
			isRunning = true;
		}

		/** stop capturing */
		void stop() {
			if (!isRunning) {return;}
			io->stop();
			isRunning = false;
		}

		/** un-initialize the webcam */
		void uninit() {
			if (!isInitialized) {return;}
			io->uninit();
			isInitialized = false;
		}

		/** close this webcam */
		void close() {

			if (!isOpen) {return;}

			debug(dev, "closing");
			if (fd != -1) {
				if (::close(fd) == -1) {throw WebcamException("error while closing device", dev, errno);}
				fd = -1;
			}

			isOpen = false;

		}

		/** the the device-file-name */
		const std::string& getDevice() const {return dev;}

		/** read all supported pixel formats from the webcam */
		const std::vector<PixelFormat>& getSupportedPixelFormats() {
			return supportedPixelFormats;
		}

		/** try to use the requested width/height and pixelFormat */
		void setFormat(const uint32_t width, const uint32_t height, const PixelFormat pf) {

			debug(dev, "initializing: " << width << "x" << height << " @ " << pf);

			// configure image format
			CLEAR(fmt);
			fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			fmt.fmt.pix.width       = width;
			fmt.fmt.pix.height      = height;
			fmt.fmt.pix.pixelformat = pf._int;
			fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
			if (WebcamIO::xioctl(fd, VIDIOC_S_FMT, &fmt) != 0) {throw WebcamException("error while setting image format", dev, errno);}

			// compare desired and actual image format
			if (fmt.fmt.pix.width != width || fmt.fmt.pix.height != height || fmt.fmt.pix.pixelformat != pf._int) {
				// TODO stop here?
				debug(dev, "format not available");//. camera suggests: " << fmt.fmt.pix.width << ":" << fmt.fmt.pix.height);
			}

			debug(dev, "\tcamera will use: " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << " @ " << PixelFormat(fmt.fmt.pix.pixelformat));
			debug(dev, "\timages will have a size of (max) " << fmt.fmt.pix.sizeimage << " bytes");

		}

		/**
		 * read the next image from the webcam.
		 * BEWARE! the returned data is volatile and belongs to the webcam!
		 * process it as fast as you can or manually copy it!
		 * @return the next image read from the webcam
		 */
		WebcamImage& readImage() {

			// read data from webcam and create WebcamImage
			io->read(img.data);
			img.setParameters(fmt.fmt.pix.width, fmt.fmt.pix.height, PixelFormat(fmt.fmt.pix.pixelformat), img.data.usedBytes);
			return img;

		}

		/** dump the webcam's capabilities */
		void dumpCapabilities() {

			debug(dev, "webcam supports:");
			//debugBool(dev, "\tcapture: ", cap.capabilities & V4L2_CAP_VIDEO_CAPTURE);
			debugBool(dev, "\tmem2mem: ", cap.capabilities & V4L2_CAP_VIDEO_M2M);
			debugBool(dev, "\tR/W: ", cap.capabilities & V4L2_CAP_READWRITE);
			debugBool(dev, "\tAIO: ", cap.capabilities & V4L2_CAP_ASYNCIO);
			debugBool(dev, "\tstream: ", cap.capabilities & V4L2_CAP_STREAMING);

		}

	private:

		/** the name of the device to use (e.g. /dev/video0) */
		std::string dev;

		/** the mode to use for IO (e.g. read/write, memory-mapped, ...) */
		WebcamIO* io;

		/** the file-descriptor for accessing the device */
		int fd;

		/** store all supported pixel-formats (like YU12, ..) here */
		std::vector<PixelFormat> supportedPixelFormats;


		/** is the file-descriptor open? */
		bool isOpen;

		/** is the webcam running (=capturing) */
		bool isRunning;

		/** is the webcam initialized? */
		bool isInitialized;


		/** the webcam's capabilities */
		struct v4l2_capability cap;

		/** the configured image-format */
		struct v4l2_format fmt;


		/** the currently read image */
		WebcamImage img;


		/** check if this device is a V4L2 device and supports capture */
		void checkIsWebcam() {

			// try to read webcam's capabilities into the capabilities-struct
			int ret = WebcamIO::xioctl(fd, VIDIOC_QUERYCAP, &cap);
			if (ret == EINVAL || ret == ENOTTY) {
				throw WebcamException("has no V4L2 capabilities -> no V4L2 device!", dev, errno);
			}

			// check if device supports capture
			if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
				throw WebcamException("selected device does not support capture!", dev);
			}

		}

		/** read all supported pixel-formats from the webcam */
		void readSupportedPixelFormats() {

			debug(dev, "reading supported pixel formats");

			// we want to query all supported pixel formats for CAPTURE. start at index 0
			struct v4l2_fmtdesc argp;
			CLEAR(argp);
			argp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			argp.index = 0;

			// for each pixel-format we want to query all supported resolutions. start at index 0
			struct v4l2_frmsizeenum size;
			CLEAR(size);

			// increment index until camera responds with EINVAL
			while (true) {

				// query format. returns EINVAL if all available indices have been queried.
				if (WebcamIO::xioctl(fd, VIDIOC_ENUM_FMT, &argp) == EINVAL) {break;}

				const PixelFormat pf(argp.pixelformat);
				supportedPixelFormats.push_back(pf);
				debug(dev, "\t" << pf);

				// read all supported resolutions for the current pixel-format
				size.index = 0;
				size.type = argp.type;
				size.pixel_format = argp.pixelformat;

				while (true) {

					// query resolution. returns EINVAL if all available indices have been queried.
					if (WebcamIO::xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &size) == EINVAL) {break;}
					debug(dev, "\t\t" << size.discrete.width << "x" << size.discrete.height);
					++size.index;

				}


				++argp.index;
			}

		}


		/** check all supported IO modes and select the best one */
		WebcamIO* getBestIO() {

			debug(dev, "initializing IO");

			// check the supported IO modes, and select one
			if		(cap.capabilities & V4L2_CAP_STREAMING) {return new WebcamIOMMAP(fd, dev);}
			else if	(cap.capabilities & V4L2_CAP_READWRITE) {return new WebcamIORW(fd, dev);}
			else											{throw WebcamException("unknown IO format", dev);}

		}


	};

}

#endif
