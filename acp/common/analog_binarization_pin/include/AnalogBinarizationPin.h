#ifndef MODULES_ACP_COMMON_ANALOG_BINARIZATION_PIN_INCLUDE_ANALOGBINARYZATIONPIN_H_
#define MODULES_ACP_COMMON_ANALOG_BINARIZATION_PIN_INCLUDE_ANALOGBINARYZATIONPIN_H_

#include <acp/core.h>

namespace acp_common_abp {

	class AnalogBinarizationPinController;
	class TAnalogBinarizationPin;

	/*********************************************************************************
	 * Controller for an analog binarization pin.
	 ********************************************************************************/
	class AnalogBinarizationPinController {
		friend class TAnalogBinarizationPin;
	private:
		// Analog pin that is sensed by this controller.
		byte pin;

		// Binarization threshold
		unsigned int threshold;

		// State of the pin (bit 0 - logical state, bit 1 - hw state, bit 2 - change flag, bit 3 - stable hw state,
		// bit 5 - report on state, bit 6 - report off state, bit 7 - inverted logic)
		byte state;

		// Counter that counts the number of read loops in the ON state of the pin.
		byte reportCounter;

		// The number of readings after which the report event is fired.
		byte fireReportCount;
	public:
		// Event handler for change of the pin state.
		ACPEventHandler stateChangedEvent;

		// Event handler for reporting the continuation of logical true state.
		ACPEventHandler stateReportedEvent;

		//--------------------------------------------------------------------------------
		// Constructs the controller
		inline AnalogBinarizationPinController(int pin, unsigned int threshold, boolean invertedLogic, boolean reportOnState, boolean reportOffState, byte fireReportCount) {
			this->pin = pin;
			this->fireReportCount = fireReportCount;
			this->threshold = threshold;

			pinMode(pin, INPUT);

			// set initial state according to the first pin reading
			int value = (analogRead(pin) < threshold) ? LOW : HIGH;
			if (invertedLogic) {
				state = (value == LOW) ? B10000001 : B10001010;
			} else {
				state = (value == LOW) ? B00000000 : B00001011;
			}

			// set continues reporting of the logical on state
			if (reportOnState) {
				state |= B00100000;
			}

			// set continues reporting of the logical off state
			if (reportOffState) {
				state |= B01000000;
			}
		}

		//--------------------------------------------------------------------------------
		// Looper method for reading the pin values.
		inline void readLooper() {
			byte oldState = state;
			int readValue = (analogRead(pin) < threshold) ? LOW : HIGH;
			// store hw read
			if (readValue == LOW) {
				state &= B11111101;
			} else {
				state |= B00000010;
			}

			// if there is a change, set change flag otherwise consider the read value as stable
			if ((oldState & B00000010) != (state & B00000010)) {
				// set change flag
				state |= B00000100;
			} else {
				// if there is change flag, check change of stabilized value
				if (state & B00000100) {
					// reset change flag
					state &= B11111011;

					// check consistency of the read value and stabilized value
					const byte checkState = state & B00001010;
					if ((checkState != 0) && (checkState != B00001010)) {
						// change of pin on/off state
						byte pinState = state & B00000010;

						// update stabilized hw state
						if (pinState) {
							state |= B00001000;
						} else {
							state &= B11110111;
						}

						// compute logical state in case of inverted logic
						if (state & B10000000) {
							pinState = !pinState;
						}

						// update logical state
						if (pinState) {
							state |= B00000001;
						} else {
							state &= B11111110;
						}

						// fire state changed event
						if (stateChangedEvent != NULL) {
							stateChangedEvent();
						}

						// exit immediately after change of state
						reportCounter = 0;
						return;
					}
				}
			}

			// process continues reporting
			if (stateReportedEvent != NULL) {
				reportCounter++;
				if (reportCounter >= fireReportCount) {
					reportCounter = 0;

					if (state & B00000001) {
						// report on
						if (state & B00100000) {
							stateReportedEvent();
						}
					} else {
						// report off
						if (state & B01000000) {
							stateReportedEvent();
						}
					}
				}
			}
		}
	};

	/********************************************************************************
	 * View for an analog binarization pin.
	 ********************************************************************************/
	class TAnalogBinarizationPin {
	private:
		// The controller
		AnalogBinarizationPinController &controller;
	public:
		//--------------------------------------------------------------------------------
		// Constructs view for analog binarization input pin
		TAnalogBinarizationPin(AnalogBinarizationPinController &controller):controller(controller) {
			// Nothing to do
		}

		//--------------------------------------------------------------------------------
		// Returns the logical state of the pin.
		inline boolean getState() {
			return controller.state & B00000001;
		}
	};
}

#endif /* MODULES_ACP_COMMON_ANALOG_BINARIZATION_PIN_INCLUDE_ANALOGBINARYZATIONPIN_H_ */
