/*----------------------------------------------------------------------*
 * Implementation of DS3232 RTC device controller based on the library: *
 *                                                                      *
 * Jack Christensen 06Mar2013                                           *
 * http://github.com/JChristensen/DS3232RTC                             *
 *                                                                      *
 * CC BY-SA 4.0                                                         *
 * "Arduino DS3232RTC Library" by Jack Christensen is licensed under    *
 * CC BY-SA 4.0, http://creativecommons.org/licenses/by-sa/4.0/         *
 *----------------------------------------------------------------------*/

#include <acp/time/ds3232rtc/DS3232RTC.h>
#include <Wire.h>

//DS3232 I2C Address
#define RTC_ADDR 0x68

//DS3232 Register Addresses
#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x01
#define RTC_HOURS 0x02
#define RTC_DAY 0x03
#define RTC_DATE 0x04
#define RTC_MONTH 0x05
#define RTC_YEAR 0x06
#define ALM1_SECONDS 0x07
#define ALM1_MINUTES 0x08
#define ALM1_HOURS 0x09
#define ALM1_DAYDATE 0x0A
#define ALM2_MINUTES 0x0B
#define ALM2_HOURS 0x0C
#define ALM2_DAYDATE 0x0D
#define RTC_CONTROL 0x0E
#define RTC_STATUS 0x0F
#define RTC_AGING 0x10
#define TEMP_MSB 0x11
#define TEMP_LSB 0x12
#define SRAM_START_ADDR 0x14    //first SRAM address
#define SRAM_SIZE 236           //number of bytes of SRAM

//Alarm mask bits
#define A1M1 7
#define A1M2 7
#define A1M3 7
#define A1M4 7
#define A2M2 7
#define A2M3 7
#define A2M4 7

//Control register bits
#define EOSC 7
#define BBSQW 6
#define CONV 5
#define RS2 4
#define RS1 3
#define INTCN 2
#define A2IE 1
#define A1IE 0

//Status register bits
#define OSF 7
#define BB32KHZ 6
#define CRATE1 5
#define CRATE0 4
#define EN32KHZ 3
#define BSY 2
#define A2F 1
#define A1F 0

#define ALARM_1 1                  //constants for calling functions
#define ALARM_2 2

//Other
#define DS1307_CH 7                //for DS1307 compatibility, Clock Halt bit in Seconds register
#define HR1224 6                   //Hours register 12 or 24 hour mode (24 hour mode==0)
#define CENTURY 7                  //Century bit in Month register
#define DYDT 6                     //Day/Date flag bit in alarm Day/Date registers

namespace acp_time_ds3232rtc {

//Square-wave output frequency (TS2, RS1 bits)
enum SQWAVE_FREQS_t {
	SQWAVE_1_HZ, SQWAVE_1024_HZ, SQWAVE_4096_HZ, SQWAVE_8192_HZ, SQWAVE_NONE
};

//Alarm masks
enum ALARM_TYPES_t {
	ALM1_EVERY_SECOND = 0x0F,
	ALM1_MATCH_SECONDS = 0x0E,
	ALM1_MATCH_MINUTES = 0x0C,     //match minutes *and* seconds
	ALM1_MATCH_HOURS = 0x08,       //match hours *and* minutes, seconds
	ALM1_MATCH_DATE = 0x00,        //match date *and* hours, minutes, seconds
	ALM1_MATCH_DAY = 0x10,         //match day *and* hours, minutes, seconds
	ALM2_EVERY_MINUTE = 0x8E,
	ALM2_MATCH_MINUTES = 0x8C,     //match minutes
	ALM2_MATCH_HOURS = 0x88,       //match hours *and* minutes
	ALM2_MATCH_DATE = 0x80,        //match date *and* hours, minutes
	ALM2_MATCH_DAY = 0x90,         //match day *and* hours, minutes
};

//--------------------------------------------------------------------------------
// Decimal-to-BCD conversion                                            *
uint8_t dec2bcd(uint8_t n) {
	return n + 6 * (n / 10);
}

//--------------------------------------------------------------------------------
// BCD-to-Decimal conversion
uint8_t bcd2dec(uint8_t n) {
	return n - 6 * (n >> 4);
}

//--------------------------------------------------------------------------------
// Read multiple bytes from RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
// Number of bytes (nBytes) must be between 1 and 32 (Wire library
// limitation).
// Returns the I2C status (zero if successful).
byte DS3232Controller::readRTC(byte addr, byte *values, byte nBytes) {
	Wire.beginTransmission(RTC_ADDR);
	Wire.write(addr);

	const byte i2cStatus = Wire.endTransmission();
	if (i2cStatus != 0) {
		return i2cStatus;
	}

	Wire.requestFrom((uint8_t) RTC_ADDR, nBytes);
	for (byte i = 0; i < nBytes; i++) {
		values[i] = Wire.read();
	}

	return 0;
}

//--------------------------------------------------------------------------------
// Read a single byte from RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
byte DS3232Controller::readRTC(byte addr) {
	byte b;
	readRTC(addr, &b, 1);
	return b;
}

//--------------------------------------------------------------------------------
// Write multiple bytes to RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
// Number of bytes (nBytes) must be between 1 and 31 (Wire library
// limitation).
// Returns the I2C status (zero if successful).
byte DS3232Controller::writeRTC(byte addr, byte *values, byte nBytes) {
	Wire.beginTransmission(RTC_ADDR);
	Wire.write(addr);
	for (byte i = 0; i < nBytes; i++) {
		Wire.write(values[i]);
	}

	return Wire.endTransmission();
}

//--------------------------------------------------------------------------------
// Write a single byte to RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
// Returns the I2C status (zero if successful).
byte DS3232Controller::writeRTC(byte addr, byte value) {
	return (writeRTC(addr, &value, 1));
}

//--------------------------------------------------------------------------------
// Reads the current time from RTC device.
void DS3232Controller::readCurrentTimeFromDevice() {
	Wire.beginTransmission(RTC_ADDR);
	Wire.write((uint8_t) RTC_SECONDS);

	// exit on error
	if (Wire.endTransmission() != 0) {
		return;
	}

	//request 7 bytes (secs, min, hr, dow, date, mth, yr)
	Wire.requestFrom(RTC_ADDR, 7);
	second = bcd2dec(Wire.read() & ~_BV(DS1307_CH));
	minute = bcd2dec(Wire.read());
	hour = bcd2dec(Wire.read() & ~_BV(HR1224));    //assumes 24hr clock
	const byte dayOfWeek = Wire.read();
	const byte dayOfMonth = bcd2dec(Wire.read());
	day = dayOfWeek * 32 + dayOfMonth;
	month = bcd2dec(Wire.read() & ~_BV(CENTURY)); //don't use the Century bit
	y2kYear = bcd2dec(Wire.read()) - 30; // convert year from 1970 to Y2K year

	const long nowInSeconds = (long) (millis() / 1000);
	const int minutesFromMidnight = int(60) * hour + minute;
	secondsFromMidnight = nowInSeconds - (60L * minutesFromMidnight + second);
	lastUpdateInSM = minutesFromMidnight;
}

//--------------------------------------------------------------------------------
// Sets time of RTC device and returns whether the operation was successful.
bool DS3232Controller::setDeviceTime(byte dayOfWeek, byte day, byte month,
		byte y2kYear, byte hour, byte minute, byte second) {
	Wire.beginTransmission(RTC_ADDR);
	Wire.write((uint8_t) RTC_SECONDS);
	Wire.write(dec2bcd(second));
	Wire.write(dec2bcd(minute));
	Wire.write(dec2bcd(hour));  //sets 24 hour format (Bit 6 == 0)
	Wire.write(dayOfWeek);
	Wire.write(dec2bcd(day));
	Wire.write(dec2bcd(month));
	Wire.write(dec2bcd(y2kYear + 30)); // convert Y2K year to year from 1970
	byte i2cStatus = Wire.endTransmission();

	uint8_t s = readRTC(RTC_STATUS);      //read the status register
	writeRTC(RTC_STATUS, s & ~_BV(OSF));  //clear the Oscillator Stop Flag

	readCurrentTimeFromDevice();	// sync time with device

	return (i2cStatus == 0);
}

//--------------------------------------------------------------------------------
// Reads the temperature from RTC device.
void DS3232Controller::readTemparatureFromDevice() {
	union int16_byte {
		int i;
		byte b[2];
	} rtcTemp;

	rtcTemp.b[0] = readRTC(TEMP_LSB);
	rtcTemp.b[1] = readRTC(TEMP_MSB);
	int rawTemperature = rtcTemp.i >> 6;
	temperature = (signed char) (rawTemperature / 4.0
			+ ((rawTemperature > 0) ? 0.5 : -0.5));
}

}

