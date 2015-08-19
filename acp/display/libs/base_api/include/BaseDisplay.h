#ifndef MODULES_ACP_DISPLAY_LIBS_BASE_API_INCLUDE_BASEDISPLAY_H_
#define MODULES_ACP_DISPLAY_LIBS_BASE_API_INCLUDE_BASEDISPLAY_H_

#include <acp/core.h>

namespace acp_display_base_api {

	/********************************************************************************
	 * Base class for a display controller.
	 ********************************************************************************/
	class BaseDisplayController {
	public:
		// Initializes the display.
		virtual void init() = 0;

		// Prints a row
		virtual void printRow(int row, const char* chars, int length) = 0;

		// Turns on or off the display.
		virtual void setDisplayState(bool onState) = 0;
	};

	/********************************************************************************
	 * Base view for display.
	 ********************************************************************************/
	template <int ROWS, int COLS> class TDisplay {
	private:
		// Controller of the display
		BaseDisplayController& controller;

		// Content of the display
		char displayContent[ROWS * COLS];

		//--------------------------------------------------------------------------------
		// Returns length of a PROGMEM string
		int fshlen(const __FlashStringHelper* ifsh) {
			PGM_P p = reinterpret_cast<PGM_P>(ifsh);
			int n = 0;
			while (true) {
				unsigned char c = pgm_read_byte(p++);
				if (c == 0) {
					break;
				}
				n++;
			}

			return n;
		}

		//--------------------------------------------------------------------------------
		// Copy given number of bytes from PROGMEM string to a given destination.
		void fshcpy(char* dst, const __FlashStringHelper* ifsh, int len) {
			PGM_P p = reinterpret_cast<PGM_P>(ifsh);
			for (int i=0; i<len; i++) {
				*dst = pgm_read_byte(p);
				dst++;
				p++;
			}
		}

		//--------------------------------------------------------------------------------
		// Measures the length of string representation of value with unit.
		size_t measureValueWithUnit(long value, const char* unit) {
			size_t n = 0;
			if (value < 0) {
				n++;
				value = -value;
			}

			if (value == 0) {
				n = 1;
			} else {
				while (value > 0) {
					value /= 10;
					n++;
				}
			}

			if (unit != NULL) {
				n += strlen(unit);
			}
		}

		//--------------------------------------------------------------------------------
		// Prints string representation of value with unit. The method returns the number
		// of printed chars.
		size_t printValueWithUnit(char* dst, size_t dstLen, long value, const char* unit) {
			size_t dstLenOrig = dstLen;
			if (dstLen == 0) {
				return dstLenOrig - dstLen;
			}

			// Print the minus sign
			if (value < 0) {
				value = -value;
				*dst = '-';
				dst++;
				dstLen--;
			}

			if (dstLen == 0) {
				return dstLenOrig - dstLen;
			}

			if (value == 0) {
				// Handle the value 0
				*dst = '0';
				dst++;
				dstLen--;
			} else {
				// Handle non-zero value
				char buffer[sizeof(long)*3];

				// Convert number to decimal digits
				char* digit = buffer;
				while (value > 0) {
					*digit = '0' + (value % 10);
					value /= 10;
					digit++;
				}

				// Write digits to output
				do {
					digit--;
					*dst = *digit;
					dstLen--;
					dst++;
				}while ((digit != buffer) && (dstLen > 0));
			}

			// Print unit
			if ((unit != NULL) && (dstLen > 0)) {
				int unitLen = strlen(unit);
				if (unitLen < dstLen) {
					unitLen = dstLen;
				}

				memcpy(dst, unit, unitLen);
				dstLen -= unitLen;
			}

			return dstLenOrig - dstLen;
		}

		//--------------------------------------------------------------------------------
		// Measures the length of string representation of value with unit.
		size_t measureValueWithUnit(double value, uint8_t precision, const char* unit) {
			size_t n = 0;
			if (value < 0) {
				n++;
				value = -value;
			}

			// Rounding
			double rounding = 0.5;
			for (uint8_t i=0; i<precision; i++) {
				rounding /= 10.0;
			}
			value += rounding;

			int intPart = (int)value;
			if (intPart == 0) {
				n++;
			} else {
				while (intPart > 0) {
					intPart /= 10;
					n++;
				}
			}

			if (precision > 0) {
				n += 1 + precision;
			}

			if (unit != NULL) {
				n += strlen(unit);
			}
		}

		//--------------------------------------------------------------------------------
		// Prints string representation of value with unit. The method returns the number
		// of printed chars.
		size_t printValueWithUnit(char* dst, size_t dstLen, double value, uint8_t precision, const char* unit) {
			size_t dstLenOrig = dstLen;

			if (dstLen == 0) {
				return dstLenOrig - dstLen;
			}

			// Print the minus sign
			if (value < 0) {
				value = -value;
				*dst = '-';
				dst++;
				dstLen--;
			}

			// Rounding
			double rounding = 0.5;
			for (uint8_t i=0; i<precision; i++) {
				rounding /= 10.0;
			}
			value += rounding;

			const unsigned long intPart = (unsigned long)value;
			size_t charsPrinted = printValueWithUnit(dst, dstLen, intPart, (precision == 0) ? unit : NULL);
			dst += charsPrinted;
			dstLen -= charsPrinted;

			// Print floating point part (if necessary)
			if (precision > 0) {
				*dst = '.';
				*dst++;
				dstLen--;

				double remainder = value - intPart;
				for (uint8_t i=0; i<precision; i++) {
					remainder *= 10;
				}

				dstLen -= printValueWithUnit(dst, dstLen, (unsigned long)remainder, unit);
			}

			return dstLenOrig - dstLen;
		}

	public:
		//--------------------------------------------------------------------------------
		// Constructs a facade view to a display.
		TDisplay(BaseDisplayController& controller): controller(controller) {
			memset(displayContent, ' ', ROWS*COLS);
		}

		//--------------------------------------------------------------------------------
		// Clears the display
		void clear() {
			memset(displayContent, ' ', ROWS*COLS);
		}

		//--------------------------------------------------------------------------------
		// Flushes the content of the display
		void flush() {
			for (int i=0; i<ROWS; i++) {
				controller.printRow(i, displayContent + i*COLS, COLS);
			}
		}

		//--------------------------------------------------------------------------------
		// Clears content of a row
		void clearRow(int row) {
			if ((row < 0) || (row >= ROWS)) {
				return;
			}

			memset(displayContent + row*COLS, ' ', COLS);
		}

		//--------------------------------------------------------------------------------
		// Prints a value with unit
		void printValueRight(int row, int value, const char* unit) {
			char buffer[COLS+1];
			size_t len = printValueWithUnit(buffer, COLS, value, unit);
			buffer[len] = 0;
			printRight(row, buffer);
		}

		//--------------------------------------------------------------------------------
		// Prints a value with unit
		void printValueRight(int row, double value, size_t precision, const char* unit) {
			char buffer[COLS+1];
			size_t len = printValueWithUnit(buffer, COLS, value, precision, unit);
			buffer[len] = 0;
			printRight(row, buffer);
		}

		//--------------------------------------------------------------------------------
		// Prints a key value pair
		void printKeyValue(int row, const char* key, const char* value) {
			printLeft(row, key);
			printRight(row, value);
		}

		//--------------------------------------------------------------------------------
		// Prints a text aligned to left.
		void printLeft(int row, const char* text) {
			if ((row < 0) || (row >= ROWS)) {
				return;
			}

			int textLen = strlen(text);
			if (textLen > COLS) {
				textLen = COLS;
			}

			if (textLen > 0) {
				memcpy(displayContent + row * COLS, text, textLen);
			}
		}

		//--------------------------------------------------------------------------------
		// Prints a text aligned to right.
		void printRight(int row, const char* text) {
			if ((row < 0) || (row >= ROWS)) {
				return;
			}

			int textLen = strlen(text);
			if (textLen > COLS) {
				textLen = COLS;
			}

			if (textLen > 0) {
				memcpy(displayContent + row * COLS + COLS - textLen, text, textLen);
			}
		}

		//--------------------------------------------------------------------------------
		// Prints a text aligned to center.
		void printCenter(int row, const char* text) {
			if ((row < 0) || (row >= ROWS)) {
				return;
			}

			int textLen = strlen(text);
			if (textLen > COLS) {
				textLen = COLS;
			}

			if (textLen > 0) {
				memcpy(displayContent + row * COLS + (COLS - textLen) / 2, text, textLen);
			}
		}

		//--------------------------------------------------------------------------------
		// Prints a text aligned to left.
		void printLeft(int row, const __FlashStringHelper* ifsh) {
			if ((row < 0) || (row >= ROWS)) {
				return;
			}

			int textLen = fshlen(ifsh);
			if (textLen > COLS) {
				textLen = COLS;
			}

			if (textLen > 0) {
				fshcpy(displayContent + row * COLS, ifsh, textLen);
			}
		}

		//--------------------------------------------------------------------------------
		// Prints a text aligned to right.
		void printRight(int row, const __FlashStringHelper* ifsh) {
			if ((row < 0) || (row >= ROWS)) {
				return;
			}

			int textLen = fshlen(ifsh);
			if (textLen > COLS) {
				textLen = COLS;
			}

			if (textLen > 0) {
				fshcpy(displayContent + row * COLS + COLS - textLen, ifsh, textLen);
			}
		}

		//--------------------------------------------------------------------------------
		// Prints a text aligned to center.
		void printCenter(int row, const __FlashStringHelper* ifsh) {
			if ((row < 0) || (row >= ROWS)) {
				return;
			}

			int textLen = fshlen(ifsh);
			if (textLen > COLS) {
				textLen = COLS;
			}

			if (textLen > 0) {
				fshcpy(displayContent + row * COLS + (COLS - textLen) / 2, ifsh, textLen);
			}
		}

		//--------------------------------------------------------------------------------
		// Turns on the display
		void on() {
			controller.setDisplayState(true);
		}

		//--------------------------------------------------------------------------------
		// Turns off the display
		void off() {
			controller.setDisplayState(false);
		}
	};

}

#endif /* MODULES_ACP_DISPLAY_LIBS_BASE_API_INCLUDE_BASEDISPLAY_H_ */
