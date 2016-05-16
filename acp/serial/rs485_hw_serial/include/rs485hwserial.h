#ifndef MODULES_ACP_SERIAL_RS485_HW_SERIAL_INCLUDE_RS485HWSERIAL_H_
#define MODULES_ACP_SERIAL_RS485_HW_SERIAL_INCLUDE_RS485HWSERIAL_H_

#include <acp/core.h>

namespace acp_serial_rs48_hw_serial {

	/********************************************************************************
	 * Stream for a rs485 serial using a hardware (UART) serial
	 ********************************************************************************/
	class THardwareRS485Serial: public Stream {
	private:
		// Hardware serial used to communicate with rs485 adapter.
		HardwareSerial &hwSerial;

		// Pin that must be enabled for data transmission.
		const uint8_t enablePin;
	public:
		//--------------------------------------------------------------------------------
		// Constructs rs485 stream over a software serial.
		THardwareRS485Serial(HardwareSerial &hwSerial, uint8_t enablePin): hwSerial(hwSerial), enablePin(enablePin) {
			pinMode(enablePin, OUTPUT);
			digitalWrite(enablePin, LOW);
		}

		//--------------------------------------------------------------------------------
		// Facade to HW serial.
		//--------------------------------------------------------------------------------

		operator bool() {
			return hwSerial;
		}

		int available() {
			return hwSerial.available();
		}

		int read() {
			return hwSerial.read();
		}

		int peek() {
			return hwSerial.peek();
		}

		void flush() {
			hwSerial.flush();
		}

		using Print::write;

		size_t write(uint8_t data) {
			digitalWrite(enablePin, HIGH);
			size_t result = hwSerial.write(data);
			hwSerial.flush();
			digitalWrite(enablePin, LOW);
			return result;
		}

		size_t write(const uint8_t *buffer, size_t size) {
			digitalWrite(enablePin, HIGH);
			size_t result = hwSerial.write(buffer, size);
			hwSerial.flush();
			digitalWrite(enablePin, LOW);
			return result;
		}
	};


	/*********************************************************************************
	 * Initializer for a hardware (UART) serial
	 ********************************************************************************/
	class HardwareRS485SerialController {
	public:
		inline void initHWSerial(HardwareSerial &hwSerial, unsigned long baud) {
			hwSerial.begin(baud);
		}
	};
}

#endif /* MODULES_ACP_SERIAL_RS485_HW_SERIAL_INCLUDE_RS485HWSERIAL_H_ */
