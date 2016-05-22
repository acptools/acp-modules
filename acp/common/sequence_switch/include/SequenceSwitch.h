#ifndef MODULES_ACP_COMMON_SEQUENCE_SWITCH_INCLUDE_SQSWITCH_H_
#define MODULES_ACP_COMMON_SEQUENCE_SWITCH_INCLUDE_SQSWITCH_H_

#include <acp/core.h>

namespace acp_common_sequence_switch {
	class TSequenceSwitch;
	class SequenceSwitchController;

	/********************************************************************************
	 * Controller of sequence switch.
	 ********************************************************************************/
	class SequenceSwitchController {
		friend class TSequenceSwitch;
	private:
		// Controlled pin
		byte pin;

		// Duration of sequence unit in 10-milliseconds
		byte baseDuration;

		// Switch sequence. Sequence consists of 4 parts (enabled, disabled, enabled, disabled).
		// Duration of each part is determined consists of given base units.
		byte sequence[4];

		// State of controller (7th bit: inverted logic, 6th bit: enabled, bit 0-1: sequence position
		byte state;

		// Updates pin state according to state of switch
		inline void updatePin() {
			boolean pinState = ((state & B01000000) != 0) && ((state & B00000001) == 0);
			if (state & B10000000) {
				pinState = !pinState;
			}
			digitalWrite(pin, (pinState) ? HIGH : LOW);
		}
	public:
		// Identifier of the looper
		int looperId;

		// Constructs a sequence switch controller
		SequenceSwitchController(int pin, boolean invertedLogic, byte baseDuration) {
			memset(sequence, 0, sizeof(sequence));
			this->baseDuration = baseDuration;
			this->pin = (byte)pin;
			pinMode(pin, OUTPUT);
			state = invertedLogic ? B10000000 : B00000000;
			updatePin();
		}

		// Initializes the controller
		inline void init(byte enabled1, byte disabled1, byte enabled2, byte disabled2) {
			acp::disableLooper(looperId);
			setSequence(enabled1, disabled1, enabled2, disabled2);
		}

		// Sets duration of the sequence unit in 10-milliseconds
		inline void setBaseDuration(byte baseDuration) {
			this->baseDuration = max(1, baseDuration);
		}

		// Sets the sequence
		inline void setSequence(byte enabled1, byte disabled1, byte enabled2, byte disabled2) {
			sequence[0] = enabled1;
			sequence[1] = disabled1;
			sequence[2] = enabled2;
			sequence[3] = disabled2;
		}

		// Returns whether switch is enabled.
		inline boolean isEnabled() {
			return state & B01000000;
		}

		// Enables/disables the switch
		void setEnabled(boolean newEnabled) {
			if (isEnabled() == newEnabled) {
				return;
			}

			if (newEnabled) {
				state |= B01000011;
				acp::enableLooper(looperId);
			} else {
				state &= B10111111;
				acp::disableLooper(looperId);
			}
		}

		// Looper for changing switch
		inline unsigned long looper() {
			if (!isEnabled()) {
				return baseDuration;
			}

			for (int i=0; i<4; i++) {
				const byte sequencePos = ((state % 4) + 1) % 4;
				state = (state & B11111100) + sequencePos;
				if (sequence[sequencePos] != 0) {
					break;
				}
			}

			updatePin();
			return 10ul * baseDuration * sequence[state % 4];
		}
	};

	/********************************************************************************
	 * View for sequence switch controller
	 ********************************************************************************/
	class TSequenceSwitch
	{
	private:
		// Associated controller
		SequenceSwitchController &controller;
	public:
		// Constructs timer view associated with given controller
		inline TSequenceSwitch(SequenceSwitchController& controller):controller(controller) {
			// Nothing to do
		}

		// Returns whether the sequence switch is enabled.
		inline boolean isEnabled() {
			return controller.isEnabled();
		}

		// Enables the sequence switch
		inline void enable() {
			controller.setEnabled(true);
		}

		// Disables the sequence switch
		inline void disable() {
			controller.setEnabled(false);
		}

		// Sets duration of the sequence unit in 10-milliseconds
		inline void setBaseDuration(byte baseDuration) {
			controller.setBaseDuration(baseDuration);
		}

		// Returns duration of sequence unit in 10-milliseconds
		inline byte getBaseInterval() {
			return controller.baseDuration;
		}

		// Sets the sequence
		inline void setSequence(byte enabled1, byte disabled1, byte enabled2, byte disabled2) {
			controller.setSequence(enabled1, disabled1, enabled2, disabled2);
		}
	};
}

#endif /* MODULES_ACP_COMMON_SEQUENCE_SWITCH_INCLUDE_SQSWITCH_H_ */
