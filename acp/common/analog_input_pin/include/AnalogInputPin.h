#ifndef MODULES_ACP_COMMON_ANALOG_INPUT_PIN_INCLUDE_ANALOGINPUTPIN_H_
#define MODULES_ACP_COMMON_ANALOG_INPUT_PIN_INCLUDE_ANALOGINPUTPIN_H_

#include <acp/core.h>

namespace acp_common_aip {

	class AnalogInputPinController;
	class TAnalogInputPin;

	/********************************************************************************
	 * Controller for an analog input pin.
	 ********************************************************************************/
	class AnalogInputPinController {
		friend class TAnalogInputPin;
	private:
		// Analog pin that is read by this controller
		byte pin;

		// Last read value.
		int value;
	public:
		// Event handler for change of measured value.
		ACPEventHandler valueChangedEvent;

		//--------------------------------------------------------------------------------
		// Constructs the controller
		inline AnalogInputPinController(int pin) {
			this->pin = pin;
			value = analogRead(pin);
		}

		//--------------------------------------------------------------------------------
		// Looper method for reading the pin values.
		inline void readLooper() {
			const int oldValue = value;
			value = analogRead(pin);
			if ((valueChangedEvent != NULL) && (oldValue != value)) {
				valueChangedEvent();
			}
		}
	};

	/********************************************************************************
	 * View for an analog input pin.
	 ********************************************************************************/
	class TAnalogInputPin {
	private:
		// The controller
		AnalogInputPinController &controller;
	public:
		//--------------------------------------------------------------------------------
		// Constructs view for analog input pin
		TAnalogInputPin(AnalogInputPinController &controller):controller(controller) {
			// Nothing to do
		}

		//--------------------------------------------------------------------------------
		// Returns the last read value
		inline int getValue() {
			return controller.value;
		}
	};
}

#endif /* MODULES_ACP_COMMON_ANALOG_INPUT_PIN_INCLUDE_ANALOGINPUTPIN_H_ */
