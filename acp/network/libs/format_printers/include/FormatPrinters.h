#ifndef MODULES_ACP_NETWORK_LIBS_FORMAT_PRINTERS_INCLUDE_FORMATPRINTERS_H_
#define MODULES_ACP_NETWORK_LIBS_FORMAT_PRINTERS_INCLUDE_FORMATPRINTERS_H_

#include <acp/core.h>

namespace acp_network_libs_format_printers {

	/********************************************************************************
	 * JSON format printer for generating an object with key-value pairs.
	 ********************************************************************************/
	class JSONMapPrinter {
	private:
		// Output
		Print* out;

		// Indicates state of printer
		// 0 - no key-value pairs has been written
		// 1 - at-least one key-value pair has been written
		// 2 - generated JSON object is closed
		byte state;

		//--------------------------------------------------------------------------------
		// Opens printing of a new key-value pair and returns whether the opening succeeded.
		bool openPair();

		//--------------------------------------------------------------------------------
		// Prints characters in \uxxxx notation (assumes valid output).
		void printUEscapedChar(char c);

		//--------------------------------------------------------------------------------
		// Prints an escaped string (assumes valid output).
		void printEscapedString(const char* str);

		//--------------------------------------------------------------------------------
		// Prints an escaped string (assumes valid output).
		void printEscapedString(const __FlashStringHelper* fstr);
	public:
		//--------------------------------------------------------------------------------
		// Construct a formatter printer that generates output to given Print object.
		JSONMapPrinter(Print* print);

		//--------------------------------------------------------------------------------
		// Construct a formatter printer that generates output to given Print object.
		JSONMapPrinter(Print& print): JSONMapPrinter(&print) {}

		//--------------------------------------------------------------------------------
		// Destructs the printer (and closed the generated map if necessary)
		~JSONMapPrinter();

		//--------------------------------------------------------------------------------
		// Add a key value pair to output.
		void add(const char* key, const char* value);
		void add(const __FlashStringHelper* key, const char* value);
		void add(const char* key, const __FlashStringHelper* value);
		void add(const __FlashStringHelper* key, const __FlashStringHelper* value);
		void add(const char* key, int value);
		void add(const __FlashStringHelper* key, int value);
		void add(const char* key, unsigned int value);
		void add(const __FlashStringHelper* key, unsigned int value);
		void add(const char* key, long value);
		void add(const __FlashStringHelper* key, long value);
		void add(const char* key, unsigned long value);
		void add(const __FlashStringHelper* key, unsigned long value);
		void add(const char* key, double value);
		void add(const __FlashStringHelper* key, double value);
		void add(const char* key, bool value);
		void add(const __FlashStringHelper* key, bool value);

		//--------------------------------------------------------------------------------
		// Closes the generated output.
		void close();
	};

	/********************************************************************************
	 * XML format printer for generating an object with key-value pairs.
	 ********************************************************************************/
	class XMLMapPrinter {
	private:
		// Output
		Print* out;

		// Indicates whether generated xml map is closed.
		bool closed;

		//--------------------------------------------------------------------------------
		// Opens printing of a new key-value pair and returns whether the opening succeeded.
		bool openPair();

		//--------------------------------------------------------------------------------
		// Opens printing of a new key-value pair and returns whether the opening succeeded.
		bool openEntry();

		//--------------------------------------------------------------------------------
		// Continues printing of a new key-value pair.
		bool continueEntry();

		//--------------------------------------------------------------------------------
		// Closes printing of a new key-value pair.
		bool closeEntry();

		//--------------------------------------------------------------------------------
		// Prints character encoded as an xml entity
		void printEntity(char c);

		//--------------------------------------------------------------------------------
		// Prints an escaped string (assumes valid output).
		void printEscapedString(const char* str);

		//--------------------------------------------------------------------------------
		// Prints an escaped string (assumes valid output).
		void printEscapedString(const __FlashStringHelper* fstr);
	public:
		//--------------------------------------------------------------------------------
		// Construct a formatter printer that generates output to given Print object.
		XMLMapPrinter(Print* print);

		//--------------------------------------------------------------------------------
		// Construct a formatter printer that generates output to given Print object.
		XMLMapPrinter(Print& print): XMLMapPrinter(&print) {}

		//--------------------------------------------------------------------------------
		// Destructs the printer (and closed the generated map if necessary)
		~XMLMapPrinter();

		//--------------------------------------------------------------------------------
		// Add a key value pair to output.
		void add(const char* key, const char* value);
		void add(const __FlashStringHelper* key, const char* value);
		void add(const char* key, const __FlashStringHelper* value);
		void add(const __FlashStringHelper* key, const __FlashStringHelper* value);
		void add(const char* key, int value);
		void add(const __FlashStringHelper* key, int value);
		void add(const char* key, unsigned int value);
		void add(const __FlashStringHelper* key, unsigned int value);
		void add(const char* key, long value);
		void add(const __FlashStringHelper* key, long value);
		void add(const char* key, unsigned long value);
		void add(const __FlashStringHelper* key, unsigned long value);
		void add(const char* key, double value);
		void add(const __FlashStringHelper* key, double value);
		void add(const char* key, bool value);
		void add(const __FlashStringHelper* key, bool value);

		//--------------------------------------------------------------------------------
		// Closes the generated output.
		void close();
	};

	/********************************************************************************
	 * Printer printing data from a stream
	 ********************************************************************************/
	template<int BUFFER_SIZE> class StreamPrinter {
	private:
		// Output
		Print* out;
	public:
		//--------------------------------------------------------------------------------
		// Construct a stream printer that generates output to given Print object.
		StreamPrinter(Print* print) {
			out = print;
		}

		//--------------------------------------------------------------------------------
		// Construct a stream printer that generates output to given Print object.
		StreamPrinter(Print &print): StreamPrinter(&print) {}

		//--------------------------------------------------------------------------------
		// Prints all available bytes in the stream to the output.
		inline void printAvailable(Stream& stream) {
			printAvailable(&stream);
		}

		//--------------------------------------------------------------------------------
		// Prints all available bytes in the stream to the output.
		void printAvailable(Stream* stream) {
			if (stream == NULL) {
				return;
			}

			uint8_t buffer[BUFFER_SIZE];
			while (true) {
				int availableBytes = stream->available();
				if (availableBytes <= 0) {
					break;
				}

				if (availableBytes > BUFFER_SIZE) {
					availableBytes = BUFFER_SIZE;
				}

				availableBytes = stream->readBytes(buffer, availableBytes);
				if (availableBytes <= 0) {
					break;
				}

				out->write(buffer, availableBytes);
			}
		}
	};

}

#endif /* MODULES_ACP_NETWORK_LIBS_FORMAT_PRINTERS_INCLUDE_FORMATPRINTERS_H_ */
