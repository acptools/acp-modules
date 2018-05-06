#ifndef MODULES_ACP_MESSENGER_RC433_SENDER_INCLUDE_RC433_RECEIVER_H_
#define MODULES_ACP_MESSENGER_RC433_SENDER_INCLUDE_RC433_RECEIVER_H_

#include <acp/core.h>
#include <acp/messenger/rc433/common/RCSwitch.h>

namespace acp_messenger_rc433_receiver {


	class Rc433ReceiverController;
	class TRc433Receiver;

	/********************************************************************************
	 * Controller-view for switch
	 ********************************************************************************/
	class Rc433ReceiverController {
		friend class TRc433Receiver;
	private:
		// 
		RCSwitch mySwitch;

		// Pin of the switch
		byte pin;

		// State of the switch
		byte protocol;
		
		// State of the switch
		byte pulse_length;

		// State of the switch
		byte repeat_times;
	public:
		// Event handler that makes processing on received data
		void (*messageReceivedEvent)(unsigned long receivedValue, unsigned int receivedBitlength, unsigned int receivedDelay, unsigned int receivedProtocol, unsigned int* receivedRawdata);

		//--------------------------------------------------------------------------------
		// Constructs the controller
		inline Rc433ReceiverController(int pin, byte protocol, byte pulse_length, byte repeat_times) {
			this->pin = (byte)pin;
			this->protocol = protocol;
			this->pulse_length = pulse_length;
			this->repeat_times = repeat_times;


 			mySwitch = RCSwitch();
 			enable();
		}

		inline void enable() {
			mySwitch.enableReceive(this->pin);
		}

		inline void disable() {
			mySwitch.disableReceive();
		}


		//--------------------------------------------------------------------------------
		// Looper method for reading the pin values.
		inline void readLooper() {

			if(messageReceivedEvent != NULL) {
				if (mySwitch.available()) {
			    	messageReceivedEvent(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedProtocol(), mySwitch.getReceivedRawdata());
					mySwitch.resetAvailable();
				}
			}
	
		}
	};

	/********************************************************************************
	 * View for a digital input pin.
	 ********************************************************************************/
	class TRc433Receiver {
	private:

		// The controller
		Rc433ReceiverController &controller;
	public:
		//--------------------------------------------------------------------------------
		// Constructs a switch on a digital pin
		TRc433Receiver(Rc433ReceiverController &controller):controller(controller) {
			// Nothing to do
		}


		inline void enable() {
			controller.enable();
		}

		inline void disable() {
			controller.disable();
		}

	};

}

#endif /* MODULES_ACP_MESSENGER_RC433_SENDER_INCLUDE_RC433_RECEIVER_H_r */
