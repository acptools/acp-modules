#ifndef MODULES_ACP_COMMON_SWITCH_INCLUDE_SWITCH_H_
#define MODULES_ACP_COMMON_SWITCH_INCLUDE_SWITCH_H_

#include <acp/core.h>

namespace acp_common_switch {

	/********************************************************************************
	 * Controller-view for switch
	 ********************************************************************************/
	class TSwitch {
	private:
		// Pin of the switch
		byte pin;

		// State of the switch
		byte state;
	public:
		//--------------------------------------------------------------------------------
		// Constructs a switch on a digital pin
		TSwitch(int pin, boolean onState, boolean invertedLogic) {
			this->pin = (byte)pin;
			pinMode(pin, OUTPUT);
			state = invertedLogic ? B00000010 : B00000000;
			setState(onState);
		}

		//--------------------------------------------------------------------------------
		// Returns whether the switch is in the ON state.
		inline boolean isOn() {
			return state & B00000001;
		}

		//--------------------------------------------------------------------------------
		// Turns on the switch
		inline void turnOn() {
			state |= B00000001;
			digitalWrite(pin, (state & B00000010) ? LOW : HIGH);
		}

		//--------------------------------------------------------------------------------
		// Turns off the switch
		inline void turnOff() {
			state &= B11111110;
			digitalWrite(pin, (state & B00000010) ? HIGH : LOW);
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
	};
}

#endif /* MODULES_ACP_COMMON_SWITCH_INCLUDE_SWITCH_H_ */
