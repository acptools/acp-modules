#ifndef MODULES_ACP_DEBUG_TRACER_INCLUDE_TRACER_H_
#define MODULES_ACP_DEBUG_TRACER_INCLUDE_TRACER_H_

#include <acp/core.h>
#include <Print.h>

//--------------------------------------------------------------------------------
// Macro for printing tracing messages.
#ifndef ACP_TRACE
#define ACP_TRACE(...) if(ACP_DEBUG) {acp_debug_tracer::tracer.print(__VA_ARGS__);}
#endif
//--------------------------------------------------------------------------------

namespace acp_debug_tracer {

	/********************************************************************************
	 * The tracer
	 ********************************************************************************/
	class Tracer {
	private:
		// Output for printing tracing messages
		Print* output;

		//--------------------------------------------------------------------------------
		// Print a prefix of each trace message.
		void printTracePrefix();

	public:
		//--------------------------------------------------------------------------------
		// Constructs and initializes the tracer.
		inline Tracer() {
			output = NULL;
		}

		//--------------------------------------------------------------------------------
		// Sets the  output for tracing messages.
		inline void setOutput(Print& output) {
			this->output = &output;
		}

		//--------------------------------------------------------------------------------
		// Sets the  output for tracing messages.
		inline void setOutput(Print* output) {
			this->output = output;
		}

		//--------------------------------------------------------------------------------
		// Prints a message.
		void print(const __FlashStringHelper* message, ...);

		//--------------------------------------------------------------------------------
		// Prints a message.
		void print(const String& message, ...);

		//--------------------------------------------------------------------------------
		// Prints a message.
		void print(const char message[], ...);
	};

	// Singleton tracer
	extern Tracer tracer;

	/********************************************************************************
	 * View for the tracer.
	 ********************************************************************************/
	class TTracer {
	public:
		//--------------------------------------------------------------------------------
		// Constructs a switch on a digital pin
		TTracer() {

		}

		//--------------------------------------------------------------------------------
		// Sets the  output for tracing messages.
		void setOutput(Print& output) {
			tracer.setOutput(output);
		}
	};
}

#endif /* MODULES_ACP_DEBUG_TRACER_INCLUDE_TRACER_H_ */
