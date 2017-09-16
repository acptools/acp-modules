#ifndef MODULES_ACP_COMMON_PULSE_SENSOR_INCLUDE_PULSESENSOR_H_
#define MODULES_ACP_COMMON_PULSE_SENSOR_INCLUDE_PULSESENSOR_H_

#include <acp/core.h>

namespace acp_common_pulsesensor {

	/********************************************************************************
	 * Controller-view for pulse sensor
	 ********************************************************************************/
    template<int TRIGGER_PIN, int PULSE_PIN, unsigned int TRIGGER_MICROS, unsigned int MAX_PULSE_MILLIS> class TPulseSensor {
	public:
    	//--------------------------------------------------------------------------------
    	// Constructs the pulse sensor.
    	inline TPulseSensor() {
    		pinMode(TRIGGER_PIN, OUTPUT);
    		pinMode(PULSE_PIN, INPUT);
    	}

    	//--------------------------------------------------------------------------------
		// Reverts the state of the switch
		inline unsigned long measurePulse() {
			// prepare trigger with LOW
			digitalWrite(TRIGGER_PIN, LOW);
			delayMicroseconds(2);

			// activate trigger with HIGH pulse
			digitalWrite(TRIGGER_PIN, HIGH);
			delayMicroseconds(TRIGGER_MICROS);
			digitalWrite(TRIGGER_PIN, LOW);

			// measure pulse length
			return pulseIn(PULSE_PIN, HIGH, MAX_PULSE_MILLIS * 1000l);
		}
	};
}

#endif /* MODULES_ACP_COMMON_PULSE_SENSOR_INCLUDE_PULSESENSOR_H_ */
