#ifndef K_WEBCAMIO_H
#define K_WEBCAMIO_H

#include <sys/ioctl.h>			// IO-commands
#include <errno.h>				// error handling
#include <stdint.h>
#include <stdlib.h>

#include "../image/DataBuffer.h"

/** reset provided element's memory to zeros */
#define CLEAR(x) memset(&(x), 0, sizeof(x))

namespace K {

	/**
	 * each Webcam supports other types of IO.
	 *		e.g. "Simple Read/Write", "Memory-Mapped", "User-Pointers", ...
	 *
	 * this class is used to provide an interface for each IO method
	 * to remove the used IO-Method from the Webcam-Class and provide
	 * an easy way to change IO methods
	 *
	 */
	class WebcamIO {

	public:

		/** dtor */
		virtual ~WebcamIO() {;}

		/**
		 * perform necessary setup (if any)
		 * @param maxImageSize the maximum size one image might have (in bytes)
		 */
		virtual void init(const uint32_t maxImageSize) = 0;

		/** perform necessary setup to start capuring */
		virtual void start() = 0;

		/** read one image into the provided buffer */
		virtual void read(DataBuffer& dst) = 0;

		/** perform necessary shutdown after capturing */
		virtual void stop() = 0;

		/** perform necessary shutdown (if any) */
		virtual void uninit() = 0;

		/** often needed helper method to communicate with the webcam */
		static int32_t xioctl(const int fh, const int request, void* arg) {

			// try until OK or unfixable error
			// EINTR = "was interrupted but you can try again"
			int32_t r;
			do {
				r = ioctl(fh, request, arg);
			} while (r == -1 && errno == EINTR);

			// if r is -1, return the error number, else: everything fine
			return (r == -1) ? (errno) : (0);

		}

	};

}

#endif
