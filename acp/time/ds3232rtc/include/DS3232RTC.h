#ifndef MODULES_ACP_TIME_DS3232RTC_INCLUDE_DS3232RTC_H_
#define MODULES_ACP_TIME_DS3232RTC_INCLUDE_DS3232RTC_H_

#include <acp/core.h>

namespace acp_time_ds3232rtc {

	class TDS3232Rtc;

	/********************************************************************************
	 * Controller for a DS3232RTC device.
	 ********************************************************************************/
	class DS3232Controller {
		friend class TDS3232Rtc;
	private:
		// Current day of month and day of week (dayOfWeek * 32 + dayOfMonth)
		byte day;

		// Current month
		byte month;

		// Current year starting from year 2000.
		byte y2kYear;

		// Current hour
		byte hour;

		// Current minute
		byte minute;

		// Current second
		byte second;

		// Seconds from starting the program when current day started.
		long secondsFromMidnight;

		// Minutes from midnight when the last reading from devices was performed.
		uint16_t lastUpdateInSM;

		// Interval in minutes after which the time in synchronized with RTC device.
		const uint8_t syncIntervalInMinutes;

		// Device temperature.
		signed char temperature;

		//--------------------------------------------------------------------------------
		// Read multiple bytes from RTC RAM.
		byte readRTC(byte addr, byte *values, byte nBytes);

		//--------------------------------------------------------------------------------
		// Read a single byte from RTC RAM.
		byte readRTC(byte addr);

		//--------------------------------------------------------------------------------
		// Write multiple bytes to RTC RAM.
		byte writeRTC(byte addr, byte *values, byte nBytes);

		//--------------------------------------------------------------------------------
		// Write a single byte to RTC RAM.
		byte writeRTC(byte addr, byte value);

		//--------------------------------------------------------------------------------
		// Reads the current time from RTC device.
		void readCurrentTimeFromDevice();

		//--------------------------------------------------------------------------------
		// Sets time of RTC device and returns whether the operation was successful.
		bool setDeviceTime(byte dayOfWeek, byte day, byte month, byte y2kYear, byte hour, byte minute, byte second);

		//--------------------------------------------------------------------------------
		// Reads the temperature from RTC device.
		void readTemparatureFromDevice();
	public:
		//--------------------------------------------------------------------------------
		// Constructs the controller
		inline DS3232Controller(int8_t syncInterval):syncIntervalInMinutes(syncInterval) {
			// Set default initial time 1.1.2000 00:00:00
			day = 6 * 32 + 1;
			month = 1;
			y2kYear = 0;
			hour = 0;
			minute = 0;
			second = 0;
			secondsFromMidnight = millis() / 1000;
			lastUpdateInSM = 0;
		}

		//--------------------------------------------------------------------------------
		// Initializes the controller and sets current time according to RTC device.
		inline void init() {
			readCurrentTimeFromDevice();
			readTemparatureFromDevice();
		}

		//--------------------------------------------------------------------------------
		// Handler of periodical looper updating time and date fields.
		inline void updateLooper() {
			const long nowFromMidnightInSeconds = (long)(millis() / 1000) - secondsFromMidnight;

			// read current time from RTC device on start of the new day
			if ((nowFromMidnightInSeconds >= 3600L * 24) || (nowFromMidnightInSeconds < 0)) {
				readCurrentTimeFromDevice();
				return;
			}

			second = nowFromMidnightInSeconds % 60;
			const int minutesFromMidnight = nowFromMidnightInSeconds / 60;

			// realize synchronization with rtc device, if necessary (according to update interval in minutes)
			if (minutesFromMidnight - lastUpdateInSM > syncIntervalInMinutes) {
				readCurrentTimeFromDevice();
				readTemparatureFromDevice();
				return;
			}

			minute = minutesFromMidnight % 60;
			hour = minutesFromMidnight / 60;
		}
	};

	/********************************************************************************
	 * View for a DS3232 device.
	 ********************************************************************************/
	class TDS3232Rtc {
	private:
		// The controller
		DS3232Controller &controller;
	public:
		//--------------------------------------------------------------------------------
		// Constructs view for controller of DS3232 RTC device.
		TDS3232Rtc(DS3232Controller &controller):controller(controller) {
			// Nothing to do
		}

		//--------------------------------------------------------------------------------
		// Returns the current day of the month starting from 1.
		inline byte getDay() {
			return controller.day % 32;
		}

		//--------------------------------------------------------------------------------
		// Returns the current day of the week starting from 1 (1-Monday, 7-Sunday).
		inline byte getDayOfWeek() {
			return controller.day / 32;
		}

		//--------------------------------------------------------------------------------
		// Returns the current month.
		inline byte getMonth() {
			return controller.month;
		}

		//--------------------------------------------------------------------------------
		// Returns the current year.
		inline int getYear() {
			return 2000 + controller.y2kYear;
		}

		//--------------------------------------------------------------------------------
		// Returns the hour component of the current time in 24-hour format.
		inline int getHour() {
			return controller.hour;
		}

		//--------------------------------------------------------------------------------
		// Returns the minute component of the current time.
		inline int getMinute() {
			return controller.minute;
		}

		//--------------------------------------------------------------------------------
		// Returns the seconds component of the current time.
		inline int getSecond() {
			return controller.second;
		}

		//--------------------------------------------------------------------------------
		// Returns the device temperature.
		inline int getTemperature() {
			return controller.temperature;
		}

		//--------------------------------------------------------------------------------
		// Sets the date and time
		inline bool setDateTime(byte dayOfWeek, byte day, byte month, byte y2kYear, byte hour, byte minute, byte second) {
			return controller.setDeviceTime(dayOfWeek, day, month, y2kYear, hour, minute, second);
		}

		//--------------------------------------------------------------------------------
		// Sets the time
		inline bool setTime(byte hour, byte minute, byte second) {
			return controller.setDeviceTime(getDayOfWeek(), getDay(), getMonth(), getYear()-2000, hour, minute, second);
		}


		//--------------------------------------------------------------------------------
		// Sets the date
		inline bool setDate(byte dayOfWeek, byte day, byte month, byte y2kYear) {
			return controller.setDeviceTime(dayOfWeek, day, month, y2kYear, controller.hour, controller.minute, controller.second);
		}

		//--------------------------------------------------------------------------------
		// Synchronizes time with rtc device.
		inline void sync() {
			controller.readCurrentTimeFromDevice();
		}

		//--------------------------------------------------------------------------------
		// Synchronizes temperature with rtc device.
		inline void syncTemperature() {
			controller.readTemparatureFromDevice();
		}
	};
}

#endif /* MODULES_ACP_TIME_DS3232RTC_INCLUDE_DS3232RTC_H_ */
