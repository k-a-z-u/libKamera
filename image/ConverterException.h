#ifndef K_CONVERTEREXCEPTION_H
#define K_CONVERTEREXCEPTION_H

#include <exception>
#include "PixelFormat.h"

namespace K {

	class ConverterException : public std::exception {

	private:

		/** the error message */
		std::string msg;

	public:

		/** ctor from string */
		ConverterException (const std::string& msg) : msg(msg) {
			;
		}

		/** pixel format unsupported */
		ConverterException(const std::string& msg, const PixelFormat& pf) {
			this->msg = msg + pf.asString();
		}

		/** pixel format unsupported */
		ConverterException(const PixelFormat& pf) {
			msg = "Unsupported pixel format: " + pf.asString();
		}

		/** output the error message */
		const char* what() const throw() override {
			return msg.c_str();
		}

	};

}

#endif // CONVERTEREXCEPTION_H
