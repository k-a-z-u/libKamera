#ifndef K_WEBCAMEXCEPTION_H
#define K_WEBCAMEXCEPTION_H

#include <exception>
#include <string.h>

namespace K {

	/**
	 * exception handling within the webcam subsystem
	 */
	class WebcamException : public std::exception {

	private:

		/** the error message */
		std::string msg;

	public:

		/** ctor from device and error-string */
		WebcamException ( const std::string& err, const std::string& dev ) {
			msg = dev + ": " + err;
		}

		/** ctor from device, error-string and details via errno */
		WebcamException ( const std::string& err, const std::string& dev, const int errnum ) {
			msg = dev + ": " + err + " (" + strerror(errnum) + ")";
		}

		/** output the error message */
		const char* what() const throw() override {
			return msg.c_str();
		}

	};

}

#endif // K_WEBCAMEXCEPTION_H
