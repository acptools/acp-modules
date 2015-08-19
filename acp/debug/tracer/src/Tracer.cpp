#include <acp/debug/tracer/Tracer.h>
#include <stdarg.h>

namespace acp_debug_tracer {

// The singleton tracer instance.
Tracer tracer;

//--------------------------------------------------------------------------------
// Print a prefix of each trace message.
void Tracer::printTracePrefix() {
	unsigned long now = millis();
	output->print(now / 1000L);
	output->print(".");
	int milliseconds = now % 1000L;
	if (milliseconds < 100) {
		output->print('0');
	}

	if (milliseconds < 10) {
		output->print('0');
	}

	output->print(milliseconds);
	output->print(": ");
}

//--------------------------------------------------------------------------------
// Prints a message.
void Tracer::print(const __FlashStringHelper* message, ...) {
	if (output == NULL) {
		return;
	}

	printTracePrefix();

	// Count number of arguments
	int argCount = 0;
	PGM_P msgPtr = reinterpret_cast<PGM_P>(message);
	char c = pgm_read_byte(msgPtr);
	while (c != '\0') {
		if (c == '%') {
			msgPtr++;
			c = pgm_read_byte(msgPtr);

			if (c == '\0') {
				break;
			}

			if (c != '%') {
				argCount++;
			}
		}

		msgPtr++;
		c = pgm_read_byte(msgPtr);
	}

	// Print message if there are no arguments
	if (argCount == 0) {
		output->println(message);
		return;
	}

	// Print message with replaced arguments
	va_list argv;
	va_start(argv, argCount);
	msgPtr = reinterpret_cast<PGM_P>(message);
	c = pgm_read_byte(msgPtr);
	while (c != '\0') {
		if (c == '%') {
			msgPtr++;
			c = pgm_read_byte(msgPtr);

			if (c == '\0') {
				break;
			}

			// Process argument
			switch (c) {
				case '%':
				output->print('%');
				break;
				case 'd':
				output->print(va_arg(argv, int));
				break;
				case 'l':
				output->print(va_arg(argv, long));
				break;
				case 'f':
				output->print(va_arg(argv, double));
				break;
				case 'c':
				output->print((char)va_arg(argv, int));
				break;
				case 's':
				output->print(va_arg(argv, const char*));
				break;
				case 'S':
				output->print(va_arg(argv, const __FlashStringHelper*));
				break;
				default:
				;
			};

		} else {
			output->print(c);
		}

		msgPtr++;
		c = pgm_read_byte(msgPtr);
	}

	va_end(argv);
	output->println();
}

//--------------------------------------------------------------------------------
// Prints a message.
void Tracer::print(const String& message, ...) {
	if (output == NULL) {
		return;
	}

	printTracePrefix();

	// Count number of arguments
	int argCount = 0;
	const char* msgPtr = message.c_str();
	while (*msgPtr != '\0') {
		if (*msgPtr == '%') {
			msgPtr++;

			if (*msgPtr == '\0') {
				break;
			}

			if (*msgPtr != '%') {
				argCount++;
			}
		}

		msgPtr++;
	}

	// Print message if there are no arguments
	if (argCount == 0) {
		output->println(message);
		return;
	}

	// Print message with replaced arguments
	va_list argv;
	va_start(argv, argCount);
	msgPtr = message.c_str();
	while (*msgPtr != '\0') {
		if (*msgPtr == '%') {
			msgPtr++;
			if (*msgPtr == '\0') {
				break;
			}

			// Process argument
			switch (*msgPtr) {
			case '%':
				output->print('%');
				break;
			case 'd':
				output->print(va_arg(argv, int));
				break;
			case 'l':
				output->print(va_arg(argv, long));
				break;
			case 'f':
				output->print(va_arg(argv, double));
				break;
			case 'c':
				output->print((char)va_arg(argv, int));
				break;
			case 's':
				output->print(va_arg(argv, const char*));
				break;
			case 'S':
				output->print(va_arg(argv, const __FlashStringHelper*));
				break;
			default:
				;
			};

		} else {
			output->print(*msgPtr);
		}

		msgPtr++;
	}

	va_end(argv);
	output->println();
}

//--------------------------------------------------------------------------------
// Prints a message.
void Tracer::print(const char message[], ...) {
	if (output == NULL) {
		return;
	}

	printTracePrefix();

	// Count number of arguments
	int argCount = 0;
	const char* msgPtr = message;
	while (*msgPtr != '\0') {
		if (*msgPtr == '%') {
			msgPtr++;

			if (*msgPtr == '\0') {
				break;
			}

			if (*msgPtr != '%') {
				argCount++;
			}
		}

		msgPtr++;
	}

	// Print message if there are no arguments
	if (argCount == 0) {
		output->println(message);
		return;
	}

	// Print message with replaced arguments
	va_list argv;
	va_start(argv, argCount);
	msgPtr = message;
	while (*msgPtr != '\0') {
		if (*msgPtr == '%') {
			msgPtr++;
			if (*msgPtr == '\0') {
				break;
			}

			// Process argument
			switch (*msgPtr) {
			case '%':
				output->print('%');
				break;
			case 'd':
				output->print(va_arg(argv, int));
				break;
			case 'l':
				output->print(va_arg(argv, long));
				break;
			case 'f':
				output->print(va_arg(argv, double));
				break;
			case 'c':
				output->print((char)va_arg(argv, int));
				break;
			case 's':
				output->print(va_arg(argv, const char*));
				break;
			case 'S':
				output->print(va_arg(argv, const __FlashStringHelper*));
				break;
			default:
				;
			};

		} else {
			output->print(*msgPtr);
		}

		msgPtr++;
	}

	va_end(argv);
	output->println();
}

}
