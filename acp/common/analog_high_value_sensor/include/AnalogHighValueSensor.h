#ifndef MODULES_ACP_COMMON_ANALOG_HIGH_VALUE_SENSOR_INCLUDE_ANALOGHIGHVALUESENSOR_H_
#define MODULES_ACP_COMMON_ANALOG_HIGH_VALUE_SENSOR_INCLUDE_ANALOGHIGHVALUESENSOR_H_

#include <acp/core.h>

namespace acp_common_analog_high_value_sensor {

	template<int HISTORY> class TAnalogHighValueSensor;

	/*********************************************************************************
	 * Controller for an analog high value sensor.
	 ********************************************************************************/
	template<int HISTORY> class AnalogHighValueSensorController {
		friend class TAnalogHighValueSensor<HISTORY>;
	private:
		// Analog pin that is sensed by this controller.
		byte pin;

		// The most recent values
		unsigned int values[HISTORY];

		// Cyclic write index to the array of values
		byte writeIdx;

		// Sum of values stored in the array of values
		unsigned int valueSum;

		// Threshold
		unsigned int threshold;

		// Time interval in milliseconds after which HIGH state is automatically reset to LOW state.
		unsigned int resetInterval;

		// The last time when real value changed from HIGH to LOW
		unsigned long lastHLTime;

		// State flags:
		// bit 0 - state (requiring reset),
		// bit 1 - hw state,
		// bit 2 - change flag,
		// bit 3 - stable hw state,
		// bit 4 - final state
		// bit 5 - report high state,
		// bit 6 - report low state,
		// bit 7 - inverted logic
		byte state;

		// Counter that counts the number of read loops in the ON state of the pin.
		byte reportCounter;

		// The number of readings after which the report event is fired.
		byte fireReportCount;
	public:
		// Event handler for change of the state.
		ACPEventHandler stateChangedEvent;

		// Event handler for reporting the continuation of a state.
		ACPEventHandler stateReportedEvent;

		//--------------------------------------------------------------------------------
		// Constructs the controller
		inline AnalogHighValueSensorController(int pin, unsigned int threshold, boolean invertedLogic, unsigned int resetInterval, boolean reportHighState, boolean reportLowState, byte fireReportCount) {
			this->pin = pin;
			this->fireReportCount = fireReportCount;
			this->threshold = threshold;
			this->resetInterval = resetInterval;

			pinMode(pin, INPUT);

			// set initial state according to the first pin reading
			unsigned int analogValue = analogRead(pin);
			for (int i=0; i<HISTORY; i++) {
				values[i] = analogValue;
			}
			writeIdx = 0;
			valueSum = analogValue * HISTORY;

			if (invertedLogic) {
				state = (analogValue < threshold) ? B10000001 : B10001010;
			} else {
				state = (analogValue < threshold) ? B00000000 : B00001011;
			}

			// set continues reporting of the HIGH state
			if (reportHighState) {
				state |= B00100000;
			}

			// set continues reporting of the LOW state
			if (reportLowState) {
				state |= B01000000;
			}
		}

		//--------------------------------------------------------------------------------
		// Looper method for reading the pin values.
		inline void readLooper() {
			// (1) read and debounce hw value - result stored in bit 4
			byte oldState = state;

			valueSum -= values[writeIdx];
			values[writeIdx] = analogRead(pin);
			valueSum += values[writeIdx];
			if (writeIdx == HISTORY-1) {
				writeIdx = 0;
			} else {
				writeIdx++;
			}

			// store hw read
			if (valueSum/HISTORY < threshold) {
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
					if ((checkState != B00000000) && (checkState != B00001010)) {
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

						// handle change of real value
						if (pinState) {
							// update real state and set HIGH state
							state |= B00010001;
							// handle LH transition
							if (!(oldState & B00000001)) {
								// fire state changed event
								if (stateChangedEvent != NULL) {
									stateChangedEvent();
								}

								// exit immediately after change of state
								reportCounter = 0;
								return;
							}
						} else {
							// update real state
							state &= B11101111;
							// store current time in case of HL transition
							lastHLTime = millis();
						}
					}
				}
			}

			// (2) handle reset case: state is HIGH but real value is bellow threshold
			if ((state & B00000001) && !(state & B00010000))  {
				if ((this->resetInterval > 0) && (millis() - this->lastHLTime > this->resetInterval)) {
					state &= B11111110;
					// fire state changed event
					if (stateChangedEvent != NULL) {
						stateChangedEvent();
					}

					// exit immediately after change of state
					reportCounter = 0;
					return;
				}
			}

			// (3) process continues reporting
			if (stateReportedEvent != NULL) {
				reportCounter++;
				if (reportCounter >= fireReportCount) {
					reportCounter = 0;

					if (state & B00000001) {
						// report high state
						if (state & B00100000) {
							stateReportedEvent();
						}
					} else {
						// report low state
						if (state & B01000000) {
							stateReportedEvent();
						}
					}
				}
			}
		}
	};

	/********************************************************************************
	 * View for an analog high value sensor.
	 ********************************************************************************/
	template <int HISTORY> class TAnalogHighValueSensor {
	private:
		// The controller
		AnalogHighValueSensorController<HISTORY> &controller;
	public:
		//--------------------------------------------------------------------------------
		// Constructs view for an analog high value sensor.
		TAnalogHighValueSensor(AnalogHighValueSensorController<HISTORY> &controller):controller(controller) {
			// Nothing to do
		}

		//--------------------------------------------------------------------------------
		// Returns the current state (HIGH - true, LOW - false)
		inline boolean getState() {
			return controller.state & B00000001;
		}

		//--------------------------------------------------------------------------------
		// Reset the state to LOW if the value is bellow threshold.
		inline void reset() {
			if (!(controller.state & B00010000)) {
				controller.state &= B11111110;
			}
		}

		//--------------------------------------------------------------------------------
		// Returns current value on the analog pin.
		inline int getValue() {
			return controller.valueSum / HISTORY;
		}
	};
}

#endif /* MODULES_ACP_COMMON_ANALOG_HIGH_VALUE_SENSOR_INCLUDE_ANALOGHIGHVALUESENSOR_H_ */
