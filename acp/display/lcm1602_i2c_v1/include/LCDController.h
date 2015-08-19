#ifndef MODULES_ACP_DISPLAY_LCM1602_I2C_V1_INCLUDE_LCDCONTROLLER_H_
#define MODULES_ACP_DISPLAY_LCM1602_I2C_V1_INCLUDE_LCDCONTROLLER_H_

#include <acp/core.h>
#include <acp/display/libs/base_api/BaseDisplay.h>
#include <acp/display/libs/basic_hw_types/LiquidCrystal_I2C.h>

namespace acp_display_lcm1602_i2c_v1 {
	using namespace acp_display_base_api;
	using namespace acp_display_libs_basic_hw;

	/********************************************************************************
	 * LCD controller for lcm1602_i2c_v1 display according to
	 ********************************************************************************/
	class LCDController: public BaseDisplayController {
	private:
		// Wrapped implementation of access to a lcd display.
		LiquidCrystal_I2C lcd;
	public:
		//--------------------------------------------------------------------------------
		// Constructs the controller
		LCDController():lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE) {
			// nothing to do
		}

		//--------------------------------------------------------------------------------
		// Initializes the display.
		void init() {
			lcd.begin(16, 2);
		}

		//--------------------------------------------------------------------------------
		// Prints a row
		void printRow(int row, const char* chars, int length) {
			lcd.setCursor(0, row);
			for (int i=0; i<length; i++) {
				lcd.write(chars[i]);
			}
		}

		//--------------------------------------------------------------------------------
		// Turns on or off the display.
		void setDisplayState(bool onState) {
			if (onState) {
				lcd.on();
			} else {
				lcd.off();
			}
		}
	};
}

#endif /* MODULES_ACP_DISPLAY_LCM1602_I2C_V1_INCLUDE_LCDCONTROLLER_H_ */
