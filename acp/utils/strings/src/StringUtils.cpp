#include <acp/utils/strings/StringUtils.h>

namespace acp_utils {

//--------------------------------------------------------------------------------
// Constructs the string builder.
StringBuilder::StringBuilder(char* outputBuffer, unsigned int bufferSize) {
	this->buffer = outputBuffer;
	if ((this->buffer == NULL) || (bufferSize == 0)) {
		this->bufferSize = 0;
	} else {
		this->bufferSize = bufferSize - 1;
		this->buffer[0] = 0;
	}

	reset();
}

//--------------------------------------------------------------------------------
// Resets the buffer
void StringBuilder::reset() {
	if (bufferSize > 0) {
		printedChars = 0;
		*buffer = 0;
	}
}

//--------------------------------------------------------------------------------
// Prints time
void StringBuilder::printTime(int hh, byte mm, byte ss) {
	if (hh < 0) {
		hh = -hh;
	}

	if (hh < 10) {
		print('0');
	}
	print(hh);
	print(':');
	printTime(mm, ss);
}

//--------------------------------------------------------------------------------
// Prints time
void StringBuilder::printTime(int mm, byte ss) {
	if (mm < 0) {
		mm = -mm;
	}

	if (mm < 10) {
		print('0');
	}
	print(mm);
	print(':');
	if (ss < 10) {
		print('0');
	}
	print(ss);
}

//--------------------------------------------------------------------------------
// Writes a new char to the output buffer.
size_t StringBuilder::write(uint8_t ch) {
	if (printedChars < bufferSize) {
		buffer[printedChars] = ch;
		printedChars++;
		buffer[printedChars] = 0;
	} else {
		return 0;
	}
}

}
