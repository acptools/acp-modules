#ifndef MODULES_ACP_TEMPERATURE_NTC_THERMISTOR_INCLUDE_NTC_THERMISTOR_H_
#define MODULES_ACP_TEMPERATURE_NTC_THERMISTOR_INCLUDE_NTC_THERMISTOR_H_

#include <acp/core.h>
#include <math.h>

namespace acp_temperature_ntcthermistor {

	template <int HISTORY, long THERMISTORNOMINAL, int TEMPERATURENOMINAL, int BCOEFFICIENT, long SERIESRESISTOR, int SHDELAY> class TNTCThermistor;

	/********************************************************************************
	 * Controller for a ntc thermistor connected to an analog input pin.
	 ********************************************************************************/
	template <int HISTORY, long THERMISTORNOMINAL, int TEMPERATURENOMINAL, int BCOEFFICIENT, long SERIESRESISTOR, int SHDELAY> class NTCThermistorController {
		friend class TNTCThermistor<HISTORY, THERMISTORNOMINAL, TEMPERATURENOMINAL, BCOEFFICIENT, SERIESRESISTOR, SHDELAY>;
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

		//--------------------------------------------------------------------------------
		// Recomputes temperature from raw value.
		void computeTemperature(const int rawValue) {
			// if the value is extremely high, the thermistor is probably disconnected
			if (rawValue > 1020) {
				temperature = NAN;
				return;
			}

			// based on https://learn.adafruit.com/thermistor/using-a-thermistor
			const float resitance = SERIESRESISTOR / ((1023.0f / rawValue) - 1);

			float steinhart;
			steinhart = resitance / THERMISTORNOMINAL;
			steinhart = log(steinhart);
			steinhart /= BCOEFFICIENT;
			steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15);
			steinhart = 1.0 / steinhart;
			steinhart -= 273.15;
			temperature = steinhart;
		}

	public:
		// Event handler for change of measured value.
		ACPEventHandler valueChangedEvent;

		//--------------------------------------------------------------------------------
		// Constructs the controller
		inline NTCThermistorController(int pin):pin(pin) {
			// nothing to do
		}

		//--------------------------------------------------------------------------------
		// Initialize values
		inline void init() {
			const int firstValue = analogRead(pin);
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
	 * View for a ntc thermistor controller.
	 ********************************************************************************/
	template <int HISTORY, long THERMISTORNOMINAL, int TEMPERATURENOMINAL, int BCOEFFICIENT, long SERIESRESISTOR, int SHDELAY> class TNTCThermistor {
	private:
		// The controller
		NTCThermistorController<HISTORY, THERMISTORNOMINAL, TEMPERATURENOMINAL, BCOEFFICIENT, SERIESRESISTOR, SHDELAY>& controller;
	public:
		//--------------------------------------------------------------------------------
		// Constructs view associated with a ntc thermistor controller.
		inline TNTCThermistor(NTCThermistorController<HISTORY, THERMISTORNOMINAL, TEMPERATURENOMINAL, BCOEFFICIENT, SERIESRESISTOR, SHDELAY>& controller): controller(controller) {
			// Nothing to do
		}

		//--------------------------------------------------------------------------------
		// Returns the last read raw value
		inline int getRawValue() {
			return controller.values[controller.sortedIndices[HISTORY/2]];
		}

		//--------------------------------------------------------------------------------
		// Returns the last measured temperature
		inline float getTemperature() {
			return controller.temperature;
		}
	};
}

#endif /* MODULES_ACP_TEMPERATURE_NTC_THERMISTOR_INCLUDE_NTC_THERMISTOR_H_ */
