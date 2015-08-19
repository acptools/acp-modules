#ifndef MODULES_ACP_COMMON_INTERRUPT_COUNTER_INCLUDE_INTERRUPTCOUNTER_H_
#define MODULES_ACP_COMMON_INTERRUPT_COUNTER_INCLUDE_INTERRUPTCOUNTER_H_

#include <acp/core.h>

namespace acp_common_interrupt_counter {

	class TInterruptCounter;

	/********************************************************************************
	 * Controller for an interrupt counter.
	 ********************************************************************************/
	class InterruptCounterController {
		friend class TInterruptCounter;
	private:
		// Counter of interrupts
		unsigned long counter;

		// Counter used by interrupt handler
		volatile unsigned int rawCounter;
	public:
		//--------------------------------------------------------------------------------
		// Constructs the controller
		inline InterruptCounterController() {
			rawCounter = 0;
			counter = 0;
		}

		//--------------------------------------------------------------------------------
		// Initialize counter
		inline void init(uint8_t interrupt, int interruptMode, void (*handler)(void)) {
			attachInterrupt(interrupt, handler, interruptMode);
		}

		//--------------------------------------------------------------------------------
		// Method for handling interrupts
		inline void handleInterrupt() {
			rawCounter++;
		}

		//--------------------------------------------------------------------------------
		// Looper method for updating counters
		inline void updateLooper() {
			unsigned int rawCounterCopy;
			noInterrupts();
			rawCounterCopy = rawCounter;
			rawCounter = 0;
			interrupts();
			counter += rawCounterCopy;
		}
	};

	/********************************************************************************
	 * View for an interrupt counter.
	 ********************************************************************************/
	class TInterruptCounter {
	private:
		// The controller
		InterruptCounterController &controller;
	public:
		//--------------------------------------------------------------------------------
		// Constructs view for an interrupt counter.
		TInterruptCounter(InterruptCounterController &controller):controller(controller) {
			// Nothing to do
		}

		//--------------------------------------------------------------------------------
		// Returns the number of counted interrupts.
		inline unsigned long getValue() {
			return controller.counter;
		}

		//--------------------------------------------------------------------------------
		// Resets the counter.
		inline void reset() {
			controller.counter = 0;
		}
	};
}

#endif /* MODULES_ACP_COMMON_INTERRUPT_COUNTER_INCLUDE_INTERRUPTCOUNTER_H_ */
