#ifndef K_DATABUFFER_H
#define K_DATABUFFER_H

#include "ConverterException.h"
#include <cstdint>
#include <cstdlib>
#include <string.h>

namespace K {

	/**
	 * something like a std::vector but slightly different.
	 * holds data and can be adjusted to the needed size
	 */
	class DataBuffer {

	private:

		friend class Webcam;
		friend class WebcamImage;

		/** the pointer to the allocated data */
		uint8_t* data;

		/** the numbfer of bytes allocated */
		uint32_t allocatedBytes;

		/** the number of bytes in use */
		uint32_t usedBytes;

	public:

		/** ctor */
		DataBuffer() : data(0), allocatedBytes(0), usedBytes(0) {
			;
		}

		/** dtor */
		~DataBuffer() {

			// cleanup
			delete data;
			data = nullptr;

		}

		/** ensure the internal buffer holds at least the give number of bytes */
		void ensureSpace(const uint32_t numBytes) {

			// already enough space allocated?
			if (allocatedBytes >= numBytes) {return;}

			// cleanup previous allocation
			delete data;

			// allocate new buffer
			data = (uint8_t*) malloc(numBytes);

			// sanity check
			if (data == nullptr) {throw new ConverterException("out of memory");}

			// adjust
			memset(data, 0, numBytes);
			allocatedBytes = numBytes;

			debug("DataBuffer", "allocated " << numBytes << " bytes");

		}

		/** get the data pointer */
		uint8_t* getData() const {return data;}


		/** set the number of used bytes */
		void setBytesUsed(const uint32_t usedBytes) {this->usedBytes = usedBytes;}

		/** get the number of used bytes */
		uint32_t getBytesUsed() const {return this->usedBytes;}


		/** move ctor */
		DataBuffer(DataBuffer&& o) {
			this->data = o.data;
			this->usedBytes = o.usedBytes;
			this->allocatedBytes = o.allocatedBytes;
			o.data = nullptr;
		}

	private:

		/** hidden copy ctor */
		DataBuffer(const DataBuffer&);

		/** hidden assignment operator */
		DataBuffer& operator = (const DataBuffer& o);

	};

}

#endif // K_DATABUFFER_H
