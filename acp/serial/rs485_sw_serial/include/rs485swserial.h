#ifndef MODULES_ACP_SERIAL_RS485_SW_SERIAL_INCLUDE_RS485SWSERIAL_H_
#define MODULES_ACP_SERIAL_RS485_SW_SERIAL_INCLUDE_RS485SWSERIAL_H_

#include <acp/core.h>
#include <SoftwareSerial.h>

namespace acp_serial_rs48_sw_serial {

	/********************************************************************************
	 * Stream for a rs485 serial using a software serial
	 ********************************************************************************/
	class TSoftwareRS485Serial: public Stream {
	private:
		// Software serial used to communicate with rs485 adapter.
		SoftwareSerial swSerial;

		// Pin that must be enabled for data transmission.
		const uint8_t enablePin;
	public:
		//--------------------------------------------------------------------------------
		// Constructs rs485 stream over a softare serial.
		TSoftwareRS485Serial(uint8_t rxPin, uint8_t txPin, uint8_t enablePin, unsigned long baud): swSerial(rxPin, txPin), enablePin(enablePin) {
			pinMode(enablePin, OUTPUT);
			digitalWrite(enablePin, LOW);
			swSerial.begin(baud);
		}

		//--------------------------------------------------------------------------------
		// Facade to SW serial.
		//--------------------------------------------------------------------------------

		operator bool() {
			return swSerial;
		}

		int available() {
			return swSerial.available();
		}

		int read() {
			return swSerial.read();
		}

		int peek() {
			return swSerial.peek();
		}

		void flush() {
			swSerial.flush();
		}

		using Print::write;

		size_t write(uint8_t data) {
			digitalWrite(enablePin, HIGH);
			size_t result = swSerial.write(data);
			digitalWrite(enablePin, LOW);
			return result;
		}

		size_t write(const uint8_t *buffer, size_t size) {
			digitalWrite(enablePin, HIGH);
			size_t result = swSerial.write(buffer, size);
			digitalWrite(enablePin, LOW);
			return result;
		}
	};

}

#endif /* MODULES_ACP_SERIAL_RS485_SW_SERIAL_INCLUDE_RS485SWSERIAL_H_ */
