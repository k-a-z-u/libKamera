#ifndef K_WEBCAMIO_RW_H
#define K_WEBCAMIO_RW_H

#include "WebcamIO.h"
#include <string>
#include <unistd.h>

#include "../Debug.h"
#include "WebcamException.h"

namespace K {

	/**
	 * IO directly accessing the webcams's device-file with fread()
	 * methods to retrieve images
	 */
	class WebcamIORW : public WebcamIO {

	private:

		uint32_t maxImageSize;

	public:

		/** ctor */
		WebcamIORW(int fd, const std::string& dev) : fd(fd), dev(dev) {
			;
		}

		/** dtor */
		~WebcamIORW() {
			;
		}

		void init(const uint32_t maxImageSize) override {

			debug(dev, "\tinitializing R/W-IO");

			this->maxImageSize = maxImageSize;

		}

		void start() override {

			// nothing to do here
			debug(dev, "\tstarting R/W-IO");

		}

		void read(DataBuffer& dst) override {

			debug(dev, "reading image (using R/W-IO)");
			dst.ensureSpace(maxImageSize);
			int numBytes = 0;

			// try to read one image
			while(true) {
				numBytes = ::read(fd, dst.getData(), maxImageSize);
				if		(numBytes > 0)		{break;}										// image available -> proceed
				else if	(errno == EAGAIN)	{usleep(10); continue;}							// wait some time and try again
				else						{throw WebcamException("error while reading image", dev, errno);}	// error
			}

		}

		void stop() override {

			debug(dev, "\tstopping R/W-IO");
			// nothing to do here ;)

		}

		void uninit() override {

			debug(dev, "\tun-initializing R/W-IO");

		}


	private:

		/** the file-descriptor for accessing the device */
		const int fd;

		/** the device name */
		std::string dev;

	};

}

#endif
