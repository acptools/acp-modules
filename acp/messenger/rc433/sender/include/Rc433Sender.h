#ifndef MODULES_ACP_MESSENGER_RC433_SENDER_INCLUDE_RC433_SENDER_H_
#define MODULES_ACP_MESSENGER_RC433_SENDER_INCLUDE_RC433_SENDER_H_

#include <acp/core.h>
#include <acp/messenger/rc433/common/RCSwitch.h>

namespace acp_messenger_rc433_sender {


	class Rc433SenderController;
	class TRc433Sender;

	/********************************************************************************
	 * Controller-view for switch
	 ********************************************************************************/
	class Rc433SenderController {
		friend class TRc433Sender;
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
		// Event handler for change of the pin state.
		ACPEventHandler messageReceivedEvent;

		//--------------------------------------------------------------------------------
		// Constructs the controller
		inline Rc433SenderController(int pin, byte protocol, byte pulse_length, byte repeat_times) {
			this->pin = (byte)pin;
			this->protocol = protocol;
			this->pulse_length = pulse_length;
			this->repeat_times = repeat_times;


 			mySwitch = RCSwitch();
 			enable();
		}

		inline void enable() {
			mySwitch.enableTransmit(this->pin);
			mySwitch.setProtocol(this->protocol);
			//mySwitch.setPulseLength(this->pulse_length);
			mySwitch.setRepeatTransmit(this->repeat_times);
		}

		inline void disable() {
			mySwitch.disableTransmit();
		}

		inline void send(unsigned long code, unsigned int length) {
			mySwitch.send(code, length);
		}

		inline void send(const char* sCodeWord) {
			mySwitch.send(sCodeWord);
		}

		inline void sendTriState(const char* sCodeWord) {
			mySwitch.send(sCodeWord);
		}

		//--------------------------------------------------------------------------------
		// Looper method for reading the pin values.
		inline void readLooper() {

			/*if(messageReceivedEvent != NULL) {
				if (mySwitch.available()) {
			    	messageReceivedEvent(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(),mySwitch.getReceivedProtocol());
					mySwitch.resetAvailable();
				}
			}*/
	
		}
	};

	/********************************************************************************
	 * View for a digital input pin.
	 ********************************************************************************/
	class TRc433Sender {
	private:

		// The controller
		Rc433SenderController &controller;
	public:
		//--------------------------------------------------------------------------------
		// Constructs a switch on a digital pin
		TRc433Sender(Rc433SenderController &controller):controller(controller) {
			// Nothing to do
		}


		inline void enable() {
			controller.enable();
		}

		inline void disable() {
			controller.disable();
		}

		inline void send(unsigned long code, unsigned int length) {
			controller.send(code, length);
		}

		inline void send(const char* sCodeWord) {
			controller.send(sCodeWord);
		}

		inline void sendTriState(const char* sCodeWord) {
			controller.send(sCodeWord);
		}

	};

}

#endif /* MODULES_ACP_MESSENGER_RC433_SENDER_INCLUDE_RC433_SENDER_H_r */
