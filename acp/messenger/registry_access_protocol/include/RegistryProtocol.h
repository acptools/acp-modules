#ifndef MODULES_ACP_MESSENGER_REGISTRY_ACCESS_PROTOCOL_INCLUDE_REGISTRYPROTOCOL_H_
#define MODULES_ACP_MESSENGER_REGISTRY_ACCESS_PROTOCOL_INCLUDE_REGISTRYPROTOCOL_H_

#include <acp/core.h>
#include <limits.h>

namespace acp_messenger_registry_msg_protocol {

// Request for reading value of an integer register.
const uint8_t READ_INT_REGISTRY_REQUEST = 0x01;

// Request for writing value to an integer register.
const uint8_t WRITE_INT_REGISTRY_REQUEST = 0x02;

// Request for reading value of a binary register.
const uint8_t READ_BIN_REGISTRY_REQUEST = 0x03;

// Request for writing value to a binary register.
const uint8_t WRITE_BIN_REGISTRY_REQUEST = 0x04;

// Request for getting change hint - an indentifier of register whose value has been change but not read.
const uint8_t GET_CHANGE_HINT_REQUEST = 0x05;

// Response indicating an unknown request or failed request.
const uint8_t REQUEST_FAILED_RESPONSE = 0x00;

// Response indicating that request was completed.
const uint8_t REQUEST_OK_RESPONSE = 0x01;

// Response indicating that write request failed due to unwritable register.
const uint8_t UNWRITABLE_REGISTER_RESPONSE = 0x02;

/********************************************************************************
 * Controller implementing handler for a message based simple registry access protocol.
 ********************************************************************************/
class RegistryAccessProtocolController {
public:
	// Processing handler invoked when write of an integer register value is requested.
	bool (*writeIntRegisterEvent)(unsigned int registerId, long value);

	// Processing handler invoked when read of an integer register value is requested.
	long (*readIntRegisterEvent)(unsigned int registerId);

	// Processing handler invoked when write of a binary register value is requested.
	bool (*writeBinRegisterEvent)(unsigned int registerId, const char* data,
			int dataLength);

	// Processing handler invoked when read of a binary register value is requested.
	int (*readBinRegisterEvent)(unsigned int registerId, char* dstBuffer,
			int dstBufferSize);

};

/********************************************************************************
 * View implementing handler for a message based simple registry access protocol.
 ********************************************************************************/
template<int CHANGE_WATCH_SIZE> class TRegistryAccessProtocol {
private:
	// The controller
	RegistryAccessProtocolController& controller;

	// Arrays storing which modified registers have not been read
	uint8_t registerWatch[(CHANGE_WATCH_SIZE + 7) / 8];

	// Circular index over modified registers
	uint8_t watchIdx;

	//--------------------------------------------------------------------------------
	// Decodes ID of register.
	inline int readRegisterId(const char* &request, int &requestSize) {
		if (requestSize == 0) {
			return false;
		}

		uint8_t hByte = (uint8_t)(*request);
		request++;
		requestSize--;

		if (hByte < 128) {
			return hByte;
		}

		if (requestSize == 0) {
			return -1;
		}

		const int result = (hByte & 0x7F) * 256 + ((uint8_t)(*request));
		request++;
		requestSize--;
		return result;
	}

	//--------------------------------------------------------------------------------
	// Decodes encoded long
	inline bool readEncodedLong(const char* &request, int &requestSize,
			long &output) {
		output = 0;
		if (requestSize == 0) {
			return false;
		}

		uint8_t numberOfBytes = 1;

		// Process the first byte of encoded long value
		uint8_t aByte = *request;
		bool hasNextByte = aByte & 0x80;// the highest bit indicates whether there is a next byte
		bool signFlag = aByte & 0x40;// second highest  bit indicates whether the number is negative
		output = aByte & 0x3F;		// take the 6 lowest bits
		request++;
		requestSize--;

		// Special handling for value 0.
		if (signFlag && (!hasNextByte) && (output == 0)) {
			output = LONG_MIN;
			return true;
		}

		// Read additional bytes
		while (hasNextByte) {
			if ((requestSize == 0) || (numberOfBytes > 5)) {
				return false;
			}

			aByte = *request;
			output = output * 128L + (aByte & 0x7F);
			hasNextByte = aByte & 0x80;

			request++;
			requestSize--;
			numberOfBytes++;
		}

		if (signFlag) {
			output = -output;
		}

		return true;
	}

	//--------------------------------------------------------------------------------
	// Encodes a long value using a variable-length encoding. The method returns
	// the number of written bytes or 0, if the output buffer is too small.
	inline uint8_t writeEncodedLong(char* outputBuffer, int bufferSize,
			long value) {
		if (bufferSize < 1) {
			return 0;
		}

		// Encode minimal long value as negative 0.
		if (value == LONG_MIN) {
			*outputBuffer = 0x40;
			return 1;
		}

		// Ensure that the encoded value is positive
		bool negativeValue = false;
		if (value < 0) {
			negativeValue = true;
			value = -value;
		}

		// Encode value in reversed order
		uint8_t rawReversed[5];
		uint8_t rawLength = 0;
		while (value > 63) {
			rawReversed[rawLength] = value % 128;
			value /= 128;
			rawLength++;
		}
		rawReversed[rawLength] = value;
		rawLength++;

		if (bufferSize < rawLength) {
			return 0;
		}

		// Add flags to the first byte (=the last reversed byte)
		if (negativeValue) {
			rawReversed[rawLength - 1] = rawReversed[rawLength - 1] | 0x40;
		}

		// Set "next byte" flags and store bytes to output buffers
		for (int i = rawLength - 1; i > 0; i--) {
			*outputBuffer = rawReversed[i] | 0x80;
			outputBuffer++;
		}

		// Store the last reversed byte
		*outputBuffer = *rawReversed;

		// Return number of written bytes
		return rawLength;
	}

	//--------------------------------------------------------------------------------
	// Creates response indicating that the request failed.
	inline void createFailResponse(char* responseBuffer, int &responseSize) {
		responseBuffer[0] = REQUEST_FAILED_RESPONSE;
		responseSize = 1;
	}

	//--------------------------------------------------------------------------------
	// Creates response indicating that the request failed.
	inline void markRegisterRead(int registerId) {
		if ((registerId >= CHANGE_WATCH_SIZE) || (registerId < 0)) {
			return;
		}

		// Clear register bit (after value of register has been read)
		registerWatch[registerId / 8] &= ~(1 << (registerId % 8));
	}

	//--------------------------------------------------------------------------------
	// The register index to a next register whose (modified&unread) bit is set
	inline bool moveRegisterIndex() {
		if (CHANGE_WATCH_SIZE == 0) {
			return false;
		}

		const uint8_t blockCount = (CHANGE_WATCH_SIZE + 7) / 8;

		// Initial setup of block index and block mask corresponding to current position of watch index
		uint8_t blockIdx = watchIdx / 8;
		uint8_t blockMask = 1 << (watchIdx % 8);
		int watchIdx = this->watchIdx;

		// Find next register with underlying bit set to one
		for (int i = 0; i < blockCount * 8; i++) {
			// Move to next index
			watchIdx++;
			blockMask = blockMask << 1;

			// If mask is 0, we have to move to the next block
			if (blockMask == 0) {
				blockMask = 1;
				blockIdx++;

				// Skip to last register of the block, if the block is empty
				if ((blockIdx < blockCount) && (registerWatch[blockIdx] == 0)) {
					blockMask = 1 << 7;
					i += 7;
					watchIdx += 7;
				}
			}

			// Circular change
			if (watchIdx >= CHANGE_WATCH_SIZE) {
				watchIdx = 0;
				blockIdx = 0;
				blockMask = 1;
				// Skip to last register of the first block, if the block is empty
				if (registerWatch[0] == 0) {
					blockMask = 1 << 7;
					watchIdx = 7;
					i += 7;
				}
			}

			// Check whether register bit is set
			if (registerWatch[blockIdx] & blockMask) {
				this->watchIdx = watchIdx;
				return true;
			}
		}

		this->watchIdx = 0;
		return false;
	}
public:
	//--------------------------------------------------------------------------------
	// Constructs view associated with a controller
	inline TRegistryAccessProtocol(RegistryAccessProtocolController& controller) :
			controller(controller) {
		memset(registerWatch, 0, (CHANGE_WATCH_SIZE + 7) / 8);
		watchIdx = 0;
	}

	//--------------------------------------------------------------------------------
	// Marks that register with given id has been modified.
	void markModifiedRegister(int registerId) {
		if ((registerId < 0) || (registerId >= CHANGE_WATCH_SIZE)) {
			return;
		}

		// Set register bit as modified&unread
		registerWatch[registerId / 8] |= 1 << (registerId % 8);
	}

	//--------------------------------------------------------------------------------
	// Handles a request by filling response buffer and setting response size.
	// Initially the responseSize must contain size of the response buffer (at least 10 bytes are recommended).
	// If the size of response is 0 then the handling routine failed.
	void handleRequest(const char* request, int requestSize,
			char* responseBuffer, int &responseSize) {
		// Ensure that there is enough space for response
		if (responseSize < 1) {
			responseSize = 0;
			return;
		}

		// Read request
		if (requestSize == 0) {
			createFailResponse(responseBuffer, responseSize);
			return;
		}

		uint8_t requestCode = (uint8_t) request[0];
		request++;
		requestSize--;

		// Action for retrieving change hint
		if (requestCode == GET_CHANGE_HINT_REQUEST) {
			int hint = -1;
			if (moveRegisterIndex()) {
				hint = watchIdx;
			}
			uint8_t writtenBytes = writeEncodedLong(responseBuffer + 1,
					responseSize - 1, hint);

			if (writtenBytes == 0) {
				createFailResponse(responseBuffer, responseSize);
				return;
			}

			// Complete response
			responseSize = 1 + writtenBytes;
			*responseBuffer = REQUEST_OK_RESPONSE;
			return;
		}

		// If action code is followed by encoded ID of a register, we read the id
		int registerId;
		if ((requestCode == READ_INT_REGISTRY_REQUEST)
				|| (requestCode == WRITE_INT_REGISTRY_REQUEST)
				|| (requestCode == READ_BIN_REGISTRY_REQUEST)
				|| (requestCode == WRITE_BIN_REGISTRY_REQUEST)) {
			registerId = readRegisterId(request, requestSize);
			if (registerId < 0) {
				createFailResponse(responseBuffer, responseSize);
				return;
			}
		}

		// Request to read a value from an integer register
		if (requestCode == READ_INT_REGISTRY_REQUEST) {
			if (controller.readIntRegisterEvent == NULL) {
				createFailResponse(responseBuffer, responseSize);
				return;
			}

			const long value = controller.readIntRegisterEvent(registerId);
			uint8_t writtenBytes = writeEncodedLong(responseBuffer + 1,
					responseSize - 1, value);
			if (writtenBytes == 0) {
				createFailResponse(responseBuffer, responseSize);
				return;
			}

			markRegisterRead(registerId);

			// Complete response
			responseSize = 1 + writtenBytes;
			*responseBuffer = REQUEST_OK_RESPONSE;
			return;
		}

		// Request to write a value to an integer register
		if (requestCode == WRITE_INT_REGISTRY_REQUEST) {
			long value;
			if ((controller.writeIntRegisterEvent == NULL)
					|| (!readEncodedLong(request, requestSize, value))) {
				createFailResponse(responseBuffer, responseSize);
				return;
			}

			if (controller.writeIntRegisterEvent(registerId, value)) {
				*responseBuffer = REQUEST_OK_RESPONSE;
			} else {
				*responseBuffer = UNWRITABLE_REGISTER_RESPONSE;
			}

			responseSize = 1;
			return;
		}

		// Request to read a value from a binary register
		if (requestCode == READ_BIN_REGISTRY_REQUEST) {
			if (controller.readBinRegisterEvent == NULL) {
				createFailResponse(responseBuffer, responseSize);
				return;
			}

			const int writtenBytes = controller.readBinRegisterEvent(registerId,
					responseBuffer + 1, responseSize - 1);
			if ((writtenBytes < 0) || (writtenBytes >= responseSize)) {
				createFailResponse(responseBuffer, responseSize);
				return;
			}

			markRegisterRead(registerId);

			// Complete response
			responseSize = 1 + writtenBytes;
			*responseBuffer = REQUEST_OK_RESPONSE;
			return;
		}

		// Request to write a value to an integer register
		if (requestCode == WRITE_BIN_REGISTRY_REQUEST) {
			if (controller.writeBinRegisterEvent == NULL) {
				createFailResponse(responseBuffer, responseSize);
				return;
			}

			if (controller.writeBinRegisterEvent(registerId, request,
					requestSize)) {
				*responseBuffer = REQUEST_OK_RESPONSE;
			} else {
				*responseBuffer = UNWRITABLE_REGISTER_RESPONSE;
			}

			responseSize = 1;
			return;
		}
	}
};

}

#endif /* MODULES_ACP_MESSENGER_REGISTRY_ACCESS_PROTOCOL_INCLUDE_REGISTRYPROTOCOL_H_ */
