#include <acp/network/libs/format_printers/FormatPrinters.h>

using namespace acp_network_libs_format_printers;

/********************************************************************************
 * JSON format printer for generating an object with key-value pairs.
 ********************************************************************************/

//--------------------------------------------------------------------------------
// Construct a formatter printer that generates output to given Print object.
JSONMapPrinter::JSONMapPrinter(Print* print) :
		out(print), state(0) {

	if (out != NULL) {
		out->print('{');
	}
}

//--------------------------------------------------------------------------------
// Destructs the printer (and closed the generated map if necessary)
JSONMapPrinter::~JSONMapPrinter() {
	close();
}

//--------------------------------------------------------------------------------
// Opens printing of a new key-value pair and returns whether the opening succeeded.
bool JSONMapPrinter::openPair() {
	if ((out == NULL) || (state == 2)) {
		return false;
	}

	if (state == 1) {
		out->print(',');
	}

	state = 1;
	return true;
}

//--------------------------------------------------------------------------------
// Prints characters in \uxxxx notation (assumes valid output).
void JSONMapPrinter::printUEscapedChar(char c) {
	int code = (int) c;
	byte hexDigits[4];
	for (int i = 3; i >= 0; i++) {
		hexDigits[i] = code % 16;
		code = code / 16;
	}

	out->print('\\');
	out->print('u');
	for (int i = 0; i < 4; i++) {
		if (hexDigits[i] < 10) {
			out->print((char) ('0' + hexDigits[i]));
		} else {
			out->print((char) ('A' + (hexDigits[i] - 10)));
		}
	}
}

//--------------------------------------------------------------------------------
// Prints an escaped string (assumes valid output).
void JSONMapPrinter::printEscapedString(const char* str) {
	if (str == NULL) {
		out->print(F("null"));
		return;
	}

	out->print('\"');

	char c = *str;
	while (c != 0) {
		if ((c == '\"') || (c == '\\') || (c == '/')) {
			out->print('\\');
			out->print(c);
		} else if (c < ' ') {
			printUEscapedChar(c);
		} else {
			out->print(c);
		}

		str++;
		c = *str;
	}

	out->print('\"');
}

//--------------------------------------------------------------------------------
// Prints an escaped string (assumes valid output).
void JSONMapPrinter::printEscapedString(const __FlashStringHelper* fstr) {
	if (fstr == NULL) {
		out->print(F("null"));
		return;
	}

	PGM_P str = reinterpret_cast<PGM_P>(fstr);
	out->print('\"');

	char c = pgm_read_byte(str);
	while (c != 0) {
		if ((c == '\"') || (c == '\\') || (c == '/')) {
			out->print('\\');
			out->print(c);
		} else if (c < ' ') {
			printUEscapedChar(c);
		} else {
			out->print(c);
		}

		str++;
		c = pgm_read_byte(str);
	}

	out->print('\"');
}

//--------------------------------------------------------------------------------
// Closes the generated output.
void JSONMapPrinter::close() {
	if (state == 2) {
		return;
	}

	if (out != NULL) {
		out->println('}');
	}

	state = 2;
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const char* key, const char* value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	printEscapedString(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const __FlashStringHelper* key, const char* value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	printEscapedString(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const char* key, const __FlashStringHelper* value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	printEscapedString(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const __FlashStringHelper* key, const __FlashStringHelper* value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	printEscapedString(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const char* key, int value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const __FlashStringHelper* key, int value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const char* key, unsigned int value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const __FlashStringHelper* key, unsigned int value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const char* key, long value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const __FlashStringHelper* key, long value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const char* key, unsigned long value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const __FlashStringHelper* key, unsigned long value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const char* key, double value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const __FlashStringHelper* key, double value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value);
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const char* key, bool value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value ? F("true") : F("false"));
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void JSONMapPrinter::add(const __FlashStringHelper* key, bool value) {
	if (!openPair()) {
		return;
	}

	printEscapedString(key);
	out->print(':');
	out->print(value ? F("true") : F("false"));
}

/********************************************************************************
 * XML format printer for generating an object with key-value pairs.
 ********************************************************************************/

//--------------------------------------------------------------------------------
// Construct a formatter printer that generates output to given Print object.
XMLMapPrinter::XMLMapPrinter(Print* print) :
		out(print), closed(false) {

	if (out != NULL) {
		out->println(F("<?xml version=\"1.0\"?>"));
		out->println(F("<map>"));
	}
}

//--------------------------------------------------------------------------------
// Destructs the printer (and closed the generated map if necessary)
XMLMapPrinter::~XMLMapPrinter() {
	close();
}

//--------------------------------------------------------------------------------
// Opens printing of a new key-value pair and returns whether the opening succeeded.
bool XMLMapPrinter::openEntry() {
	if ((out == NULL) || closed) {
		return false;
	}

	out->print(F("<entry key=\""));
	return true;
}

//--------------------------------------------------------------------------------
// Continues printing of a new key-value pair.
bool XMLMapPrinter::continueEntry() {
	out->print(F("\" value=\""));
	return true;
}

//--------------------------------------------------------------------------------
// Closes printing of a new key-value pair.
bool XMLMapPrinter::closeEntry() {
	out->println(F("\"/>"));
}

//--------------------------------------------------------------------------------
// Prints character encoded as an xml entity
void XMLMapPrinter::printEntity(char c) {
	if (c == '&') {
		out->print(F("&amp;"));
	} else if (c == '<') {
		out->print(F("&lt;"));
	} else if (c == '>') {
		out->print(F("&gt;"));
	} else if (c == '\"') {
		out->print(F("&quot;"));
	} else if (c == '\'') {
		out->print(F("&apos;"));
	} else {
		out->print(F("&#"));
		out->print((int) c);
		out->print(';');
	}
}

//--------------------------------------------------------------------------------
// Prints an xml escaped string (assumes valid output).
void XMLMapPrinter::printEscapedString(const char* str) {
	if (str == NULL) {
		return;
	}

	char c = *str;
	while (c != 0) {
		if ((c == '&') || (c == '<') || (c == '>') || (c == '\"')
				|| (c == '\'')) {
			printEntity(c);
		} else {
			out->print(c);
		}

		str++;
		c = *str;
	}
}

//--------------------------------------------------------------------------------
// Prints an xml escaped string (assumes valid output).
void XMLMapPrinter::printEscapedString(const __FlashStringHelper* fstr) {
	if (fstr == NULL) {
		return;
	}

	PGM_P str = reinterpret_cast<PGM_P>(fstr);
	char c = pgm_read_byte(str);
	while (c != 0) {
		if ((c == '&') || (c == '<') || (c == '>') || (c == '\"') || (c == '\'')) {
			printEntity(c);
		} else {
			out->print(c);
		}

		str++;
		c = pgm_read_byte(str);
	}
}

//--------------------------------------------------------------------------------
// Closes the generated output.
void XMLMapPrinter::close() {
	if (closed) {
		return;
	}

	if (out != NULL) {
		out->println(F("</map>"));
	}

	closed = true;
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const char* key, const char* value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	printEscapedString(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const __FlashStringHelper* key, const char* value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	printEscapedString(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const char* key, const __FlashStringHelper* value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	printEscapedString(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const __FlashStringHelper* key, const __FlashStringHelper* value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	printEscapedString(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const char* key, int value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const __FlashStringHelper* key, int value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const char* key, unsigned int value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const __FlashStringHelper* key, unsigned int value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const char* key, long value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const __FlashStringHelper* key, long value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const char* key, unsigned long value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const __FlashStringHelper* key, unsigned long value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const char* key, double value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const __FlashStringHelper* key, double value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value);
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const char* key, bool value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value ? F("true") : F("false"));
	closeEntry();
}

//--------------------------------------------------------------------------------
// Add a key-value pair to output.
void XMLMapPrinter::add(const __FlashStringHelper* key, bool value) {
	if (!openEntry()) {
		return;
	}

	printEscapedString(key);
	continueEntry();
	out->print(value ? F("true") : F("false"));
	closeEntry();
}
