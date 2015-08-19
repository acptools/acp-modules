#ifndef MODULES_ACP_TEMPERATURE_RTD_INCLUDE_RTD_H_
#define MODULES_ACP_TEMPERATURE_RTD_INCLUDE_RTD_H_

#include <acp/core.h>

namespace acp_temperature_rtd {

	template <int HISTORY, int SHDELAY> class TRtd;

	/********************************************************************************
	 * Controller for a RTD connected to an analog input pin.
	 ********************************************************************************/
	template <int HISTORY, int SHDELAY> class RtdController {
		friend class TRtd<HISTORY, SHDELAY>;
	private:
		// Analog pin that is read by this controller
		byte pin;

		// Last read values
		int values[HISTORY];

		// Position of the oldest value in the values array
		byte writeIdx;

		// Indices of values resulting in a sorted sequence.
		byte sortedIndices[HISTORY];

		// Computed temperature
		float temperature;

		// Multiplication factor
		float multiplicator;

		// Additive constant to translate readings to temperature
		float shift;

		// Recomputes temperature from raw value.
		inline void computeTemperature(const int rawValue) {
			temperature = multiplicator * rawValue + shift;
		}

	public:
		// Event handler for change of measured value.
		ACPEventHandler valueChangedEvent;

		//--------------------------------------------------------------------------------
		// Constructs the controller.
		inline RtdController(int pin, int temperature1, int reading1, int temperature2, int reading2):pin(pin) {
			// Compute parameters to map voltage readings to temperature
			multiplicator = (temperature2 - temperature1) / (float)(reading2 - reading1);
			shift = ((temperature1 - multiplicator * reading1) + (temperature2 - multiplicator * reading2)) / 2;
		}

		//--------------------------------------------------------------------------------
		// Initialize values.
		inline void init() {
			// Initial reading
			int firstValue = analogRead(pin);
			for (int i=0; i<HISTORY; i++) {
				values[i] = firstValue;
				sortedIndices[i] = i;
			}
			writeIdx = 0;

			computeTemperature(firstValue);
		}

		//--------------------------------------------------------------------------------
		// Looper method for reading the pin values.
		inline void readLooper() {
			const int oldRawValue = values[sortedIndices[HISTORY/2]];
			if (SHDELAY > 0) {
				analogRead(pin);
				delayMicroseconds(SHDELAY);
			}
			//const int readValue = acp::analogReadWD(pin, 250);
			const int readValue = analogRead(pin);
			values[writeIdx] = readValue;

			// Find position of the changed value
			byte* currentIdx = sortedIndices;
			while (*currentIdx != writeIdx) {
				currentIdx++;
			}

			// Move value to left (if possible)
			byte* leftIdx = currentIdx - 1;
			while ((currentIdx != sortedIndices) && (values[*leftIdx] > readValue)) {
				byte tmp = *leftIdx;
				*leftIdx = *currentIdx;
				*currentIdx = tmp;
				leftIdx--;
				currentIdx--;
			}

			// Move value to right (if possible)
			byte* rightIdx = currentIdx + 1;
			byte* const end = sortedIndices + HISTORY;
			while ((rightIdx != end) && (values[*rightIdx] < readValue)) {
				byte tmp = *rightIdx;
				*rightIdx = *currentIdx;
				*currentIdx = tmp;
				rightIdx++;
				currentIdx++;
			}

			writeIdx++;
			if (writeIdx >= HISTORY) {
				writeIdx = 0;
			}

			const int newRawValue = values[sortedIndices[HISTORY/2]];
			if (newRawValue != oldRawValue) {
				// Compute temperature
				computeTemperature(newRawValue);

				// Fire events
				if (valueChangedEvent != NULL) {
					valueChangedEvent();
				}
			}
		}
	};

	/********************************************************************************
	 * View for a RTD controller.
	 ********************************************************************************/
	template <int HISTORY, int SHDELAY> class TRtd {
	private:
		// The controller
		RtdController<HISTORY, SHDELAY>& controller;
	public:

		//--------------------------------------------------------------------------------
		// Constructs view associated with a thermistor controller.
		inline TRtd(RtdController<HISTORY, SHDELAY>& controller): controller(controller) {
			// Nothing to do
		}

		//--------------------------------------------------------------------------------
		// Returns the last read raw value.
		inline int getRawValue() {
			return controller.values[controller.sortedIndices[HISTORY/2]];
		}

		//--------------------------------------------------------------------------------
		// Returns the last measured temperature.
		inline float getTemperature() {
			return controller.temperature;
		}
	};
}

#endif /* MODULES_ACP_TEMPERATURE_RTD_INCLUDE_RTD_H_ */
