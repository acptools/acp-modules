#ifndef MODULES_ACP_COMMON_MEASURED_SWITCH_INCLUDE_MEASURED_SWITCH_H_
#define MODULES_ACP_COMMON_MEASURED_SWITCH_INCLUDE_MEASURED_SWITCH_H_

#include <acp/core.h>

namespace acp_common_measured_switch {

	/********************************************************************************
	 * Controller-view for measured switch
	 ********************************************************************************/
	class TMeasuredSwitch {
	private:
		// Pin of the switch
		byte pin;

		// State of the switch
		struct {
			// Indicates whether switch has inverted logic
			bool invertedLogic: 1;
			// Indicates whether switch is in ON state.
			bool on: 1;
		} state;

		// Total time that device was in ON state
		unsigned long runningTimeInSeconds;

		// Time when the state of switch changed from OFF to ON
		unsigned long millisWhenTurnOn;
	public:
		//--------------------------------------------------------------------------------
		// Constructs a switch on a digital pin
		TMeasuredSwitch(int pin, boolean onState, boolean invertedLogic) {
			runningTimeInSeconds = 0;
			millisWhenTurnOn = 0;
			this->pin = (byte)pin;
			pinMode(pin, OUTPUT);
			state.invertedLogic = invertedLogic;
			setState(onState);
		}

		//--------------------------------------------------------------------------------
		// Returns whether the switch is in the ON state.
		inline boolean isOn() {
			return state.on;
		}

		//--------------------------------------------------------------------------------
		// Turns on the switch
		inline void turnOn() {
			if (!isOn()) {
				millisWhenTurnOn = millis();
			}

			state.on = true;
			digitalWrite(pin, state.invertedLogic ? LOW : HIGH);
		}

		//--------------------------------------------------------------------------------
		// Turns off the switch
		inline void turnOff() {
			if (isOn()) {
				runningTimeInSeconds += ((millis() - millisWhenTurnOn) + 500L /*rounding*/) / 1000L;
			}

			state.on = false;
			digitalWrite(pin, state.invertedLogic ? HIGH : LOW);
		}

		//--------------------------------------------------------------------------------
		// Sets the on/off state. Set true for ON state and false for OFF state.
		inline void setState(boolean onState) {
			if (onState) {
				turnOn();
			} else {
				turnOff();
			}
		}

		//--------------------------------------------------------------------------------
		// Confirm state to underlying hardware by setting state of the pin.
		inline void confirmState() {
			setState(isOn());
		}

		//--------------------------------------------------------------------------------
		// Reverts the state of the switch
		inline void revert() {
			if (isOn()) {
				turnOff();
			} else {
				turnOn();
			}
		}

		//--------------------------------------------------------------------------------
		// Returns the total duration in seconds when the switch was in the ON state.
		inline unsigned long getRunningTime() {
			return getUptime();
		}

		//--------------------------------------------------------------------------------
		// Set initial value for counter measuring total duration of ON state in seconds.
		inline void setRunningTime(unsigned long value) {
			setUptime(value);
		}

		//--------------------------------------------------------------------------------
		// Returns the total uptime in seconds.
		inline unsigned long getUptime() {
			if (isOn()) {
				return runningTimeInSeconds + ((millis() - millisWhenTurnOn) + 500L /*rounding*/) / 1000L;
			} else {
				return runningTimeInSeconds;
			}
		}

		//--------------------------------------------------------------------------------
		// Set initial value for counter measuring total uptime in seconds.
		inline void setUptime(unsigned long value) {
			runningTimeInSeconds = value;
			millisWhenTurnOn = millis();
		}
	};
}

#endif /* MODULES_ACP_COMMON_MEASURED_SWITCH_INCLUDE_MEASURED_SWITCH_H_ */
