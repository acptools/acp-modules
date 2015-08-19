#ifndef MODULES_ACP_UTILS_STRINGS_INCLUDE_STRINGUTILS_H_
#define MODULES_ACP_UTILS_STRINGS_INCLUDE_STRINGUTILS_H_

#include <acp/core.h>

namespace acp_utils {

	/********************************************************************************
	 * Builder of a null-terminated string.
	 ********************************************************************************/
	class StringBuilder: public Print {
	private:
		// Output buffer
		char* buffer;

		// Size of the output buffer
		unsigned int bufferSize;

		// Number of characters that were printed
		unsigned int printedChars;
	public:
		//--------------------------------------------------------------------------------
		// Constructs the string builder.
		StringBuilder(char* outputBuffer, unsigned int bufferSize);

		//--------------------------------------------------------------------------------
		// Resets the buffer
		void reset();

		//--------------------------------------------------------------------------------
		// Prints time
		void printTime(int hh, byte mm, byte ss);

		//--------------------------------------------------------------------------------
		// Prints time
		void printTime(int mm, byte ss);

		//--------------------------------------------------------------------------------
		// Writes a new char to the output buffer.
		virtual size_t write(uint8_t ch);

		using Print::write;
	};
}

#endif /* MODULES_ACP_UTILS_STRINGS_INCLUDE_STRINGUTILS_H_ */
