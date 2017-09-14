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

		// Duration of base unit as multiples of 10 milliseconds.
		byte baseDuration;

		// Switch sequence. Sequence consists of 4 parts (enabled, disabled, enabled, disabled).
		// Duration of each part is determined in base units.
		byte sequence[4];

		// State of the switch
		struct {
			// Indicates whether switch has inverted logic
			bool invertedLogic: 1;
			// Indicates whether sequencing is active (enabled)
			bool enabled: 1;
			// Indicates whether switch is forced to be in ON state.
			bool on: 1;
			// Indicates whether sequence is invalid
			bool invalidSequence: 1;
			// Sequence position
			byte seqPosition: 2;
		} state;

		// Updates pin state according to state of switch
		inline void updatePin() {
			boolean pinState = state.on || (state.enabled && (state.seqPosition % 2 == 0));
			if (state.invertedLogic) {
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
			this->baseDuration = max(1, baseDuration);
			this->pin = (byte)pin;
			pinMode(pin, OUTPUT);

			state.enabled = false;
			state.on = false;
			state.invertedLogic = invertedLogic;
			state.seqPosition = 0;
			state.invalidSequence = true;
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
			setEnabled(false);

			sequence[0] = enabled1;
			sequence[1] = disabled1;
			sequence[2] = enabled2;
			sequence[3] = disabled2;

			state.invalidSequence = (enabled1+disabled1+enabled2+disabled2 == 0);
		}

		// Enables/disables sequencing
		void setEnabled(boolean newEnabled) {
			if (state.enabled == newEnabled) {
				return;
			}

			state.enabled = newEnabled;
			state.seqPosition = 0;
			if (state.enabled && (!state.invalidSequence)) {
				acp::enableLooper(looperId);
			} else {
				acp::disableLooper(looperId);
			}

			updatePin();
		}

		// Sets the forced on/off state. Set true for forced ON state and false for forced OFF state.
		void setState(boolean onState) {
			state.on = onState;
			updatePin();
		}

		// Looper for changing switch
		inline unsigned long looper() {
			if (!state.enabled || state.invalidSequence) {
				return baseDuration;
			}

			for (int i=0; i<4; i++) {
				state.seqPosition = (state.seqPosition + 1) % 4;
				if (sequence[state.seqPosition] != 0) {
					break;
				}
			}

			updatePin();
			return 10ul * baseDuration * sequence[state.seqPosition];
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
			return controller.state.enabled;
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

		// Sets the forced on/off state. This state overrides states defined by sequence.
		inline void setState(boolean onState) {
			controller.setState(onState);
		}
	};
}

#endif /* MODULES_ACP_COMMON_SEQUENCE_SWITCH_INCLUDE_SQSWITCH_H_ */
