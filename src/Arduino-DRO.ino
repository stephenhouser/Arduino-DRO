/*
 ArduinoDRO + Tach V6.0
 
 iGaging/AccuRemote Digital Scales Controller V3.3
 Created 5 July 2014
 Update 15 July 2014
 Copyright (C) 2014 Yuriy Krushelnytskiy, http://www.yuriystoys.com
 
 
 Updated 24 January 2019 by Ryszard Malinowski
 http://www.rysium.com 

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

 
 Version 2.b - Added support for tachometer on axis T with accurate timing
 Version 3.0 - Added option to send rpm raw data (time and count)
 Version 5.2 - Correction to retrieving scale sign bit.
 Version 5.2 - Corrected scale frequency clock.
 Version 5.2 - Added option to pre-scale tach reading compensating for more than one tach pulse per rotation.
 Version 5.3 - Added option to average and round tach output values.
 Version 5.3 - Added option to select max tach update frequency
 Version 5.4 - Replace Yuriy's method of clocking scales with method written by Les Jones
 Version 5.5 - Optimizing the scale reading logic using method written by Les Jones
 Version 5.6 - Adding 4us delay between scale clock signal change and reading first axis data
 Version 5.7 - Added option to smooth DRO reading by implementing weighted average with automatic smoothing factor
 Version 5.8 - Correction to calculate average for scale X. Increase weighted average sample size to 32.
 Version 5.9 - Reduce flickering on RPM display.  Remove long delay in RPM displaying Zero after the rotation stops.
 Version 5.10 - Add "smart rounding" on tach display.  Fix 1% tach rounding.  Support processors running at 8MHz clock.
 Version 5.11 - Add "touch probe" support.
 Version 5.12 - Fix "touch probe" port definition and comments.
 Version 6.0 - Add suport for Quadrature Encoder scales through LS7366R-type shield.
 
 
 NOTE: This program supports pulse sensor to measure rpm and switch type touch probe .  For quadrature encoder scales use LS7366R-based shield.
 If at least one quadrature scale is used do not conect other devices to SPI dedicated pins as LS7366R uses SPI for communication
 Read your Arduino board documentation for SPI pins as on some boards they are shared with "normal" I/O pins (on Arduino UNO it is 11, 12 and 13).
 
 Configuration parameters:
	SCALE_<n>_ENABLED
		Defines if DRO functionality on axis <n> should be supported.  
		If supported DRO scale should be connected to I/O pin defined in constant "<n>DataPin" and 
		DRO data is sent to serial port with corresponding axis prefix (X, Y, Z or W)
		Clock pin is common for all iGaging scales should be connected to I/O pin defined in constant "clockPin" 
		Possible values:
			0 = DRO functionality on axis <n> is not supported
			1 = DRO functionality on axis <n> is supported
		Default value = 1

	SCALE_<n>_TYPE
		Defines the type of scale used on axis <n>.  
		Two types of scales are supported: iGaging/AccuRemote Digital Scales and  quadrature encoder scales (common glass or magnetic)
		Note: If at least on scale is type 1 do not connect any other devices to SPI pins as it will interfere with communication with LS7366R.
		Possible values:
			0 = iGaging/AccuRemote Digital Scales with 21bit protocol
			1 = Quadrature Encoder scales through LS7366R-type shield (32-bit quadrature counter with serial interface).
		Default value = 0

	SCALE_CLK_PIN
		Defines the I/O pin where clock signal for all iGaging DRO scales is connected.  Used only if at least one scale is type 0.
		Possible values:
			integer number between 2 and 13
		Default value = 2

	SCALE_<n>_PIN
		Defines the I/O pin where DRO data signal for selected scale is connected
		Note: For quadrature scale this pin is connected to SPI SS pin in corresponding LS7366R.
		Possible values:
			integer number between 2 and 13
		Default values = 3, 4, 5, 6 (for corresponding axis X, Y, Z and W)

	SCALE_<n>_AVERAGE_ENABLED
		Defines if DRO reading should be averaged using weighted average calculation with automating smoothing factor.   
		If average is enabled the reading is much more stable without "jumping" and "flickering" when the scale "can't decide" on the value.  
		Note: This value is not used when corresponding SCALE_<n>_ENABLED is 0 
		Possible values:
			0 = exact measured from the scale is sent
			1 = scale reading averaged using weighted average calculation with automatic smoothing factor
		Default value = 1

	AXIS_AVERAGE_COUNT
		Defines the number of last DRO readings that will be used to calculate weighted average for DRO.
		For machines with power feed on any axis change this value to lower number i.e. 8 or 16.
		Possible values:
			integer number between 4 and 32 
		Recommended values:
			16 for machines with power feed 
			32 for all manual machines
		Default value = 24

	TACH_ENABLED
		Defines if tach sensor functionality should be supported.  
		If supported tach sensor should be connected to I/O pin defined in constant INPUT_TACH_PIN and 
		rpm value is sent to serial port with axis prefix "T" 
		Possible values:
			0 = tach sensor functionality is not supported
			1 = tach sensor functionality is supported
		Default value = 1

	INPUT_TACH_PIN
		Defines the I/O pin where tach sensor signal is connected
		Possible values:
			integer number between 2 and 13
		Default value = 7

	TACH_PRESCALE
		Defines how many tach pulses per one revolution the sensor sends.  
		For example if tach sensor uses two magnets on the shaft the sensor will generate two pulses per revolution.
		This can be used to get better resolution and faster response time for very low rpm
		Note: This value is not used when TACH_RAW_DATA_FORMAT is enabled 
		Possible values:
			any integer number greater than 0
		Default value = 1

	TACH_AVERAGE_COUNT
		Defines the number of last tach readings that will be used to calculate average tach rpm.
		If you want to send measured rpm instead of average rpm set this value to 1.
		Note: This value is not used when TACH_RAW_DATA_FORMAT is enabled.
		      It is recommended to set this value 2 times or more of TACH_PRESCALE value.
		      For example: if TACH_PRESCALE = 4, set TACH_AVERAGE_COUNT = 8
		Possible values:
			1 = exact measured tach reading is sent
			any integer number greater than 1 - average tach reading is sent 
		Default value = 6

	TACH_ROUND
		Defines how tach reading should be rounded.   
		If rounding is enabled the reading can be rounded either by 1% of current rpm or to the fixed "round" number with predefined RPM thresholds ("smart rounding"). 
		For example with 1% rounding if measured rpm is between 980rpm and  1020 rpm the display will show numbers rounded to 9 and 10 (i.e. 981, 990, 999, 1000, 1010, 1020 etc.). 
		With "smart rounding" the measured rpm is rounded to the nearest 1, 2, 5, 10, 20, 50 and 100 depends on measured RPM (change at predefined thresholds).
		For example with "smart rounding" all measured rpm is between 500pm and  2000 rpm the display will show numbers rounded to the nearest 5 (i.e. 980, 985, 990, 995, 1000, 1005  etc.). 
		Note: This value is not used when TACH_RAW_DATA_FORMAT is enabled 
		Possible values:
			0 = exact measured tach reading is sent
			1 = tach reading is rounded to the nearest 1% of measured rpm (1% rounding)
			2 = tach reading is rounded to the nearest "round" number with fixed thresholds ("smart rounding")
		Default value = 2

	TACH_RAW_DATA_FORMAT
		Defines the format of tach data sent to serial port.
		Note: when raw data format is used, then TACH_PRESCALE, TACH_AVERAGE_COUNT and TACH_ROUND are ignored 
		Possible values:
			0 = tach data is sent in single value format: T<rpm>;
			1 = tach data is sent in raw (two values) format: T<total_time>/<number_of_pulses>;
		Default value = 0

	MIN_RPM_DELAY
		Defines the delay (in milliseconds) in showing 0 when rotation stops.  If rpm is so low and time between tach pulse
		changes longer than this value, value zero rpm ("T0;") will be sent to the serial port.
		Note: this number will determine the slowest rpm that can be measured.  In order to measure smaller rpm I suggest 
		      to use a sensor with more than one "ticks per revolution" (for example hall sensor with two or more magnets).
		      The number of "ticks per revolution" should be set in tachometer setting in Android app.
		Possible values:
			any integer number greater than 0
		Default value = 1200 (the minimum rpm measured will be 50 rpm)

	OUTPUT_TACH_LED_ENABLED
		Defines if the tach LED feedback is supported.  
		If supported the tach feedback LED should be connected to I/O pin defined in constant OUTPUT_TACH_LED_PIN below 
		Possible values:
			0 = tach LED feedback functionality is not supported
			1 = tach LED feedback functionality is supported
		Default value = 1

	OUTPUT_TACH_LED_PIN
		Defines the I/O pin where the tach LED feedback is connected.  
		Tach LED feedback indicates the status of INPUT_TACH_PIN for debugging purposes
		Possible values:
			integer number between 2 and 13
		Default value = 9

	PROBE_ENABLED
		Defines if touch probe sensor functionality should be supported.  
		If supported touch probe should be connected to I/O pin defined in constant INPUT_PROBE_PIN.   
		Possible values:
			1 = touch probe functionality is supported
			0 = touch probe functionality is not supported
		Default value = 1

	INPUT_PROBE_PIN
		Defines the I/O pin where touch probe signal is connected
		Possible values:
			integer number between 2 and 13
		Default value = 8

	PROBE_INVERT
		Defines if the touch probe input pin signal needs to be inverted (enter the signal level when touch probe is not touching).
		Possible values:
			0 = touch probe input pin signal is LOW (logical Zero) when touch probe is in "normal open" status (not touching)
			1 = touch probe input pin signal is HIGH (logical One) when touch probe is in "normal open" status (not touching)
		Default value = 0

	OUTPUT_PROBE_LED_ENABLED
		Defines if the touch probe LED feedback is supported.  
		If supported the touch probe feedback LED should be connected to I/O pin defined in constant INPUT_PROBE_PIN below 
		Possible values:
			1 = touch probe LED feedback functionality is supported
			0 = touch probe LED feedback functionality is not supported
		Default value = 1

	OUTPUT_PROBE_LED_PIN
		Defines the I/O pin where the touch probe LED feedback is connected.  
		Touch probe LED feedback indicates the status of INPUT_PROBE_PIN for debugging purposes
		Possible values:
			integer number between 2 and 13
		Default value = 10

	UART_BAUD_RATE
		Defines the serial port baud rate.  Make sure it matches the Bluetooth module's baud rate.
		Recommended value:
			1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200
		Default value = 9600

	UPDATE_FREQUENCY
		Defines the Frequency in Hz (number of timer per second) the scales are read and the data is sent to the application.
		Possible values:
			any integer number between 1 and 64 
		Default value = 24
		
	TACH_UPDATE_FREQUENCY
		Defines the max Frequency in Hz (number of timer per second) the tach output is sent to the application.
		Note: This value must be a divider of UPDATE_FREQUENCY that would result zero reminder.
		      For example for UPDATE_FREQUENCY = 24 valid TACH_UPDATE_FREQUENCY are: 1, 2, 3, 4, 6, 8, 12 and 24 
		Possible values:
			any integer number between 1 and UPDATE_FREQUENCY 
		Default value = 4

 */

/* === Display DRO === */
#include "LedController.hpp"
#include <EEPROM.h>

/*
 * pin 12 is connected to the DataIn 
 * pin 11 is connected to the CLK 
 * pin 10 is connected to LOAD 
 */

#define DISPLAY_COUNT			4
#define DISPLAY_WIDTH			9	// one extra to account for the decimal point 
#define DISPLAY_PRECISION		-4	// number of digits after '.'. Negative means round last to 0/5 

#define DISPLAY_MENU_ADDRESS	3

#define MODE_CHAR_ABS	'A'
#define MODE_CHAR_INC	' '
#define MODE_CHAR_SEL	'-'

#define SCALE_MM				(2560.0 / 25.4)
#define SCALE_INCH				(2560.0)

#define IFRAME_TIMEDELAY 		(5 * 1000)	/* ms */

// LedController seven_seg = LedController(12, 11, 10, DISPLAY_COUNT, false);
LedController seven_seg = LedController(11, 13, 10, DISPLAY_COUNT, true);

bool iFrameTrigger = false;		// Should we force a display and data update?
unsigned long iFrameLastTime = 0L;		// The last time we forced a data push to serial

// TODO: Factor out Switch Ports to a #define 
#define ANALOG_BUTTON_COUNT		3

#define ANALOG_DEBOUNCE_TIME	100
#define ANALOG_IGNORE_TIME		500
#define ANALOG_HIGH_SW_LEVEL	855
#define ANALOG_MED_SW_LEVEL		515
#define ANALOG_LOW_SW_LEVEL		170

typedef enum {
	button_none			= 0x000,
	button_x_zero		= 0x001,
	button_y_zero		= 0x002,
	button_z_zero		= 0x004,
	button_w_zero		= 0x008,

	button_x_absinc		= 0x010,
	button_y_absinc		= 0x020,
	button_z_absinc		= 0x040,
	button_w_absinc		= 0x080,

	button_up			= 0x100,
	button_select		= 0x200,
	button_down			= 0x400
} _interface_buttons;

#define SUBMODE_TIMEOUT	(15 * 1000)		// 15 seconds
#define BLINK_TIME		(250)			// millis

int buttonInputPins[ANALOG_BUTTON_COUNT] = { A0, A1, A2 };	// Pins to configure as analog decoded button inputs
_interface_buttons lastButtons = 0;	// The last button set we saw. To keep from repeating ourselves
unsigned long lastButtonTime = 0L;

typedef enum {			// state machine for UI and operations
	invalid_state = 0,	// unset state.
	show_values,			// normal operating mode
	show_menu,			// show the config menu (> when in show_values)

	set_axis_count,		// set the counts per inch for axis
	set_brightness,		// setting brightness
	set_precision,		// set the display precision (number of decimal places)
	set_units,			// set the display units (inch or mm)
	reverse_axis,		// reverse axis orientation (+/-)

	set_axis_half,		// setting an axis to 1/2 value, select axis (+ when in show_values)
	set_axis_value,		// set axis to a specific value
	zero_all,			// zero all axes (- when in show_values)
} _state;

_state lastState;
_state currentState;	// current state of our UI state machine

typedef struct {
	const char *title;
	_state state;
} _menu;

/* the ordering of the menu items is really just my prefernece and
 * predisposition to hit the down button when thinking of non-saved
 * settings. So there's no real logic or reason here.
 * menuSelected gets set to the first non-saved setting 1/2 axis
 * again, just as my preference and thought this will be the most used
 * selection.
 */
_menu menu[] = {
	{ " hell ", 	show_values },
	{ "CPI",		set_axis_count },
	{ "bright", 	set_brightness },
	{ "digits",		set_precision },
	{ "units",		set_axis_half },
	{ "reverse", 	reverse_axis },	
	/* saved settings above this point, UP to get to them */

	/* not saved settings below this point DOWN to get to them*/
	{ "1-2 axis",	set_axis_half },
	{ "set value", 	set_axis_value },	
	{ "zero all",	zero_all },
	{ "boobies", 	show_values },
};

int menuSelected = 5;

typedef enum {
	units_inch = 0,
	units_mm
} _display_units;

/* Settings that get saved in EEPROM to be restored on restart */
#define SETTINGS_MAGIC_VERSION	0x02
#define SETTINGS_MAGIC_ADDRESS	0
#define SETTINGS_EEPROM_ADDRESS	1

#define SETTINGS_BRIGHT_MASK	0x0F
#define SETTINGS_PRECISION_MASK	0xF0

#define SETTINGS_X_BIT	0
#define SETTINGS_Y_BIT	1
#define SETTINGS_Z_BIT	2
#define SETTINGS_W_BIT	3
#define SETTINGS_S_BIT	4

struct _saved_settings {
	byte	precision_brightness;	// brightness in lower nybble, precision in upper (0-15)
	byte	axis_enabled;			// x x x S  W Z Y X -- 1 enabled, 0-disabled
	byte	orientation;			// x x x S  W Z Y X -- 1=revrse, 0=normal
	byte	units;					// x x x S  W Z Y X -- 1=metric, 0=imperial
	unsigned int	xCountPerInch;	// number of counts per inch for X axis (2^16)
	unsigned int	yCountPerInch;	// number of counts per inch for Y axis (2^16)
	unsigned int	zCountPerInch;	// number of counts per inch for Z axis (2^16)
	unsigned int	wCountPerInch;	// number of counts per inch for W axis (2^16)
	unsigned int	sCountPerRevolution;	// number of counts per revolution for spindle (2^16)
};

struct _saved_settings droSettings;

void resetSettings() {
	droSettings.precision_brightness = 0x33;	// precision -4, brightness 3
	droSettings.axis_enabled = 0x17;			// Spindle, Z, Y, and X enabled
	droSettings.orientation = 0x00;				// All oriented to whatever scale reads
	droSettings.units = 0x00;					// All using inches
	droSettings.xCountPerInch = 2560;			// default for iGaging EZView scale
	droSettings.yCountPerInch = 2560;			// default for iGaging EZView scale
	droSettings.zCountPerInch = 2560;			// default for iGaging EZView scale
	droSettings.wCountPerInch = 2560;			// default for iGaging EZView scale
	droSettings.sCountPerRevolution = 1;		// default for iGaging EZView scale
	EEPROM.put(SETTINGS_EEPROM_ADDRESS, droSettings);
}

void loadSettings() {
	if (checkSettings()) {
		EEPROM.get(SETTINGS_EEPROM_ADDRESS, droSettings);
	} else {
		resetSettings();
	}
}

void saveSettings() {
	EEPROM.put(SETTINGS_MAGIC_ADDRESS, SETTINGS_MAGIC_VERSION);
	EEPROM.put(SETTINGS_EEPROM_ADDRESS, droSettings);
}

bool checkSettings() {
	return EEPROM.read(SETTINGS_MAGIC_ADDRESS) == SETTINGS_MAGIC_VERSION;
}

inline int displayBrightness() {
	return droSettings.precision_brightness & SETTINGS_BRIGHT_MASK;
}

inline void setDisplayBrightness(int bright) {
	droSettings.precision_brightness = (droSettings.precision_brightness & ~SETTINGS_BRIGHT_MASK) 
										| (bright & SETTINGS_BRIGHT_MASK);
	saveSettings();
}

inline int displayPrecision() {
	 // bias is 8 for negative numbers
	return ((droSettings.precision_brightness >> 4) & 0x0F) - 8;
}

inline void setDisplayPrecision(int precision) {
	droSettings.precision_brightness = (droSettings.precision_brightness & ~SETTINGS_PRECISION_MASK) 
										| (((precision + 8) << 4) & SETTINGS_PRECISION_MASK);
	saveSettings();
}
inline bool xAxisReversed() {
	return bitRead(droSettings.orientation, SETTINGS_X_BIT);
}

inline void setXAxisReversed(bool reversed) {
	if (reversed) {
		bitSet(droSettings.orientation, SETTINGS_X_BIT);
	} else {
		bitClear(droSettings.orientation, SETTINGS_X_BIT);
	}
	
	saveSettings();
}

inline bool yAxisReversed() {
	return bitRead(droSettings.orientation, SETTINGS_Y_BIT);
}

inline void setYAxisReversed(bool reversed) {
	if (reversed) {
		bitSet(droSettings.orientation, SETTINGS_Y_BIT);
	} else {
		bitClear(droSettings.orientation, SETTINGS_Y_BIT);
	}
	
	saveSettings();
}

inline bool zAxisReversed() {
	return bitRead(droSettings.orientation, SETTINGS_Z_BIT);
}

inline void setZAxisReversed(bool reversed) {
	if (reversed) {
		bitSet(droSettings.orientation, SETTINGS_Z_BIT);
	} else {
		bitClear(droSettings.orientation, SETTINGS_Z_BIT);
	}
	
	saveSettings();
}

inline bool wAxisReversed() {
	return bitRead(droSettings.orientation, SETTINGS_W_BIT);
}

inline void setWAxisReversed(bool reversed) {
	if (reversed) {
		bitSet(droSettings.orientation, SETTINGS_W_BIT);
	} else {
		bitClear(droSettings.orientation, SETTINGS_W_BIT);
	}

	saveSettings();
}

_display_units displayUnits() {
	// TODO: Allow independently settable units rather than just using the X for them all.
	return bitRead(droSettings.units, SETTINGS_X_BIT) ? units_inch : units_mm;
}

void setDisplayUnits(_display_units units) {
	// TODO: Allow independently settable units rather than just using the X for them all.
	if (units == units_mm) {
		bitSet(droSettings.units, SETTINGS_X_BIT);
	} else {
		bitClear(droSettings.units, SETTINGS_X_BIT);
	}

	saveSettings();
}

inline int xCountPerInch() {
	return droSettings.xCountPerInch;
}

inline int yCountPerInch() {
	return droSettings.yCountPerInch;
}

inline int zCountPerInch() {
	return droSettings.zCountPerInch;
}

inline int wCountPerInch() {
	return droSettings.wCountPerInch;
}

inline int sCountPerRevolution() {
	return droSettings.sCountPerRevolution;
}


/* === Display DRO === */

// DRO config (if axis is not connected change in the corresponding constant value from "1" to "0")
#define SCALE_X_ENABLED 1
#define SCALE_Y_ENABLED 1
#define SCALE_Z_ENABLED 1
#define SCALE_W_ENABLED 0

// DRO config (if axis is connected to Quadrature Encoder scales through LS7366R-type shield change in the corresponding constant value from "0" to "1")
#define SCALE_X_TYPE 0
#define SCALE_Y_TYPE 0
#define SCALE_Z_TYPE 0
#define SCALE_W_TYPE 0

// I/O ports config (change pin numbers if DRO, Tach sensor or Tach LED feedback is connected to different ports)
#define SCALE_CLK_PIN 2

#define SCALE_X_PIN 3
#define SCALE_Y_PIN 4
#define SCALE_Z_PIN 5
#define SCALE_W_PIN 6

// DRO rounding On/Off (if not enabled change in the corresponding constant value from "1" to "0")
#define SCALE_X_AVERAGE_ENABLED 1
#define SCALE_Y_AVERAGE_ENABLED 1
#define SCALE_Z_AVERAGE_ENABLED 1
#define SCALE_W_AVERAGE_ENABLED 1

// DRO rounding sample size.  Change it to 16 for machines with power feed
#define AXIS_AVERAGE_COUNT 24

// Tach config (if Tach is not connected change in the corresponding constant value from "1" to "0")
#define TACH_ENABLED 1
#define INPUT_TACH_PIN 8

// Tach pre-scale value (number of tach sensor pulses per revolution)
#define  TACH_PRESCALE 1

// Number of tach measurements to average 
#define TACH_AVERAGE_COUNT 6

// This is rounding for tachometer display (set to 0 to disable or 1 for 1% rounding)
#define TACH_ROUND 0

// Tach data format
#define TACH_RAW_DATA_FORMAT 0			// single value format: T<rpm>;

// Tach RPM config
#define MIN_RPM_DELAY 1200				// 1.2 sec calculates to low range = 50 rpm.

// Tach LED feadback configmm
#define OUTPUT_TACH_LED_ENABLED 0
#define OUTPUT_TACH_LED_PIN 9


// Touch probe config (if Touch Probe is not connected change in the corresponding constant value from "1" to "0")
#define PROBE_ENABLED 0
#define INPUT_PROBE_PIN 8				// Pin 8 connected to Touch Probe

// Touch probe invert signal config
#define PROBE_INVERT 0					// Touch Probe signal inversion: Open = Input pin is Low; Closed = Input pin is High

// Touch probe LED feadback config
#define OUTPUT_PROBE_LED_ENABLED 1
#define OUTPUT_PROBE_LED_PIN 10			// When Quadrature Encoder scale are not used, on Arduino Uno you may change it to on-board LED pin 13.


// General Settings
#define UART_BAUD_RATE 9600				//  Set this so it matches the BT module's BAUD rate 
#define UPDATE_FREQUENCY 24				//  Frequency in Hz (number of timer per second the scales are read and the data is sent to the application)
#define TACH_UPDATE_FREQUENCY 4 		//  Max Frequency in Hz (number of timer per second) the tach output is sent to the application

//---END OF CONFIGURATION PARAMETERS ---


//---DO NOT CHANGE THE CODE BELOW UNLESS YOU KNOW WHAT YOU ARE DOING ---

/* iGaging Clock Settings (do not change) */
#define SCALE_CLK_PULSES 21				// iGaging and Accuremote scales use 21 bit format
#define SCALE_CLK_FREQUENCY 9000		// iGaging scales run at about 9-10KHz
#define SCALE_CLK_DUTY 20				// iGaging scales clock run at 20% PWM duty (22us = ON out of 111us cycle)

/* weighted average constants */ 
#define FILTER_SLOW_EMA AXIS_AVERAGE_COUNT	// Slow movement EMA
#define FILTER_FAST_EMA 2 					// Fast movement EMA


#if (SCALE_X_ENABLED > 0) || (SCALE_Y_ENABLED > 0) || (SCALE_Z_ENABLED > 0) || (SCALE_W_ENABLED > 0)
#define DRO_ENABLED 1
#else
#define DRO_ENABLED 0
#endif

#if (SCALE_X_ENABLED > 0 && SCALE_X_TYPE == 0) || (SCALE_Y_ENABLED > 0 && SCALE_Y_TYPE == 0) || (SCALE_Z_ENABLED > 0 && SCALE_Z_TYPE == 0) || (SCALE_W_ENABLED > 0 && SCALE_W_TYPE == 0)
#define DRO_TYPE0_ENABLED 1
#else
#define DRO_TYPE0_ENABLED 0
#endif

#if (SCALE_X_ENABLED > 0 && SCALE_X_TYPE == 1) || (SCALE_Y_ENABLED > 0 && SCALE_Y_TYPE == 1) || (SCALE_Z_ENABLED > 0 && SCALE_Z_TYPE == 1) || (SCALE_W_ENABLED > 0 && SCALE_W_TYPE == 1)
#define DRO_TYPE1_ENABLED 1
#else
#define DRO_TYPE1_ENABLED 0
#endif


#if (SCALE_X_AVERAGE_ENABLED > 0) || (SCALE_Y_AVERAGE_ENABLED > 0) || (SCALE_Z_AVERAGE_ENABLED > 0) || (SCALE_W_AVERAGE_ENABLED > 0)
#define SCALE_AVERAGE_ENABLED 1
#else
#define SCALE_AVERAGE_ENABLED 0
#endif

// Define registers and pins for scale ports
#if SCALE_CLK_PIN < 8 
#define CLK_PIN_BIT SCALE_CLK_PIN
#define SCALE_CLK_DDR DDRD
#define SCALE_CLK_OUTPUT_PORT PORTD
#else
#define CLK_PIN_BIT (SCALE_CLK_PIN - 8)
#define SCALE_CLK_DDR DDRB
#define SCALE_CLK_OUTPUT_PORT PORTB
#endif

#if SCALE_X_PIN < 8 
#define X_PIN_BIT SCALE_X_PIN
#define X_DDR DDRD
#define X_INPUT_PORT PIND
#define X_OUTPUT_PORT PORTD
#else
#define X_PIN_BIT (SCALE_X_PIN - 8)
#define X_DDR DDRB
#define X_INPUT_PORT PINB
#define X_OUTPUT_PORT PORTB
#endif

#if SCALE_Y_PIN < 8 
#define Y_PIN_BIT SCALE_Y_PIN
#define Y_DDR DDRD
#define Y_INPUT_PORT PIND
#define Y_OUTPUT_PORT PORTD
#else
#define Y_PIN_BIT (SCALE_Y_PIN - 8)
#define Y_DDR DDRB
#define Y_INPUT_PORT PINB
#define Y_OUTPUT_PORT PORTB
#endif

#if SCALE_Z_PIN < 8 
#define Z_PIN_BIT SCALE_Z_PIN
#define Z_DDR DDRD
#define Z_INPUT_PORT PIND
#define Z_OUTPUT_PORT PORTD
#else
#define Z_PIN_BIT (SCALE_Z_PIN - 8)
#define Z_DDR DDRB
#define Z_INPUT_PORT PINB
#define Z_OUTPUT_PORT PORTB
#endif

#if SCALE_W_PIN < 8 
#define W_PIN_BIT SCALE_W_PIN
#define W_DDR DDRD
#define W_INPUT_PORT PIND
#define W_OUTPUT_PORT PORTD
#else
#define W_PIN_BIT (SCALE_W_PIN - 8)
#define W_DDR DDRB
#define W_INPUT_PORT PINB
#define W_OUTPUT_PORT PORTB
#endif

// Define tach interrupt for selected pin
#if INPUT_TACH_PIN == 2 
#define TACH_PIN_BIT 2
#define TACH_DDR DDRD
#define TACH_INPUT_PORT PIND
#define TACH_INTERRUPT_VECTOR PCINT2_vect
#define TACH_INTERRUPT_REGISTER PCIE2
#define TACH_INTERRUPT_MASK PCMSK2
#define TACH_INTERRUPT_PIN PCINT18

#elif INPUT_TACH_PIN == 3
#define TACH_PIN_BIT 3
#define TACH_DDR DDRD
#define TACH_INPUT_PORT PIND
#define TACH_INTERRUPT_VECTOR PCINT2_vect
#define TACH_INTERRUPT_REGISTER PCIE2
#define TACH_INTERRUPT_MASK PCMSK2
#define TACH_INTERRUPT_PIN PCINT19

#elif INPUT_TACH_PIN == 4
#define TACH_PIN_BIT 4
#define TACH_DDR DDRD
#define TACH_INPUT_PORT PIND
#define TACH_INTERRUPT_VECTOR PCINT2_vect
#define TACH_INTERRUPT_REGISTER PCIE2
#define TACH_INTERRUPT_MASK PCMSK2
#define TACH_INTERRUPT_PIN PCINT20

#elif INPUT_TACH_PIN == 5
#define TACH_PIN_BIT 5
#define TACH_DDR DDRD
#define TACH_INPUT_PORT PIND
#define TACH_INTERRUPT_VECTOR PCINT2_vect
#define TACH_INTERRUPT_REGISTER PCIE2
#define TACH_INTERRUPT_MASK PCMSK2
#define TACH_INTERRUPT_PIN PCINT21

#elif INPUT_TACH_PIN == 6
#define TACH_PIN_BIT 6
#define TACH_DDR DDRD
#define TACH_INPUT_PORT PIND
#define TACH_INTERRUPT_VECTOR PCINT2_vect
#define TACH_INTERRUPT_REGISTER PCIE2
#define TACH_INTERRUPT_MASK PCMSK2
#define TACH_INTERRUPT_PIN PCINT22

#elif INPUT_TACH_PIN == 7
#define TACH_PIN_BIT 7
#define TACH_DDR DDRD
#define TACH_INPUT_PORT PIND
#define TACH_INTERRUPT_VECTOR PCINT2_vect
#define TACH_INTERRUPT_REGISTER PCIE2
#define TACH_INTERRUPT_MASK PCMSK2
#define TACH_INTERRUPT_PIN PCINT23

#elif INPUT_TACH_PIN == 8
#define TACH_PIN_BIT 0
#define TACH_DDR DDRB
#define TACH_INPUT_PORT PINB
#define TACH_INTERRUPT_VECTOR PCINT0_vect
#define TACH_INTERRUPT_REGISTER PCIE0
#define TACH_INTERRUPT_MASK PCMSK0
#define TACH_INTERRUPT_PIN PCINT0

#elif INPUT_TACH_PIN == 9
#define TACH_PIN_BIT 1
#define TACH_DDR DDRB
#define TACH_INPUT_PORT PINB
#define TACH_INTERRUPT_VECTOR PCINT0_vect
#define TACH_INTERRUPT_REGISTER PCIE0
#define TACH_INTERRUPT_MASK PCMSK0
#define TACH_INTERRUPT_PIN PCINT1

#elif INPUT_TACH_PIN == 10
#define TACH_PIN_BIT 2
#define TACH_DDR DDRB
#define TACH_INPUT_PORT PINB
#define TACH_INTERRUPT_VECTOR PCINT0_vect
#define TACH_INTERRUPT_REGISTER PCIE0
#define TACH_INTERRUPT_MASK PCMSK0
#define TACH_INTERRUPT_PIN PCINT2

#elif INPUT_TACH_PIN == 11
#define TACH_PIN_BIT 3
#define TACH_DDR DDRB
#define TACH_INPUT_PORT PINB
#define TACH_INTERRUPT_VECTOR PCINT0_vect
#define TACH_INTERRUPT_REGISTER PCIE0
#define TACH_INTERRUPT_MASK PCMSK0
#define TACH_INTERRUPT_PIN PCINT3

#elif INPUT_TACH_PIN == 12
#define TACH_PIN_BIT 4
#define TACH_DDR DDRB
#define TACH_INPUT_PORT PINB
#define TACH_INTERRUPT_VECTOR PCINT0_vect
#define TACH_INTERRUPT_REGISTER PCIE0
#define TACH_INTERRUPT_MASK PCMSK0
#define TACH_INTERRUPT_PIN PCINT4

#elif INPUT_TACH_PIN == 13
#define TACH_PIN_BIT 5
#define TACH_DDR DDRB
#define TACH_INPUT_PORT PINB
#define TACH_INTERRUPT_VECTOR PCINT0_vect
#define TACH_INTERRUPT_REGISTER PCIE0
#define TACH_INTERRUPT_MASK PCMSK0
#define TACH_INTERRUPT_PIN PCINT5
#endif

#if OUTPUT_TACH_LED_PIN < 8 
#define TACH_LED_PIN_BIT OUTPUT_TACH_LED_PIN
#define TACH_LED_DDR DDRD
#define TACH_LED_OUTPUT_PORT PORTD
#else
#define TACH_LED_PIN_BIT (OUTPUT_TACH_LED_PIN - 8)
#define TACH_LED_DDR DDRB
#define TACH_LED_OUTPUT_PORT PORTB
#endif

// Define registers and pins for touch probe
#if INPUT_PROBE_PIN < 8 
#define PROBE_PIN_BIT INPUT_PROBE_PIN
#define PROBE_DDR DDRD
#define PROBE_INPUT_PORT PIND
#else
#define PROBE_PIN_BIT (INPUT_PROBE_PIN - 8)
#define PROBE_DDR DDRB
#define PROBE_INPUT_PORT PINB
#endif

#if OUTPUT_PROBE_LED_PIN < 8 
#define PROBE_LED_PIN_BIT OUTPUT_PROBE_LED_PIN
#define PROBE_LED_DDR DDRD
#define PROBE_LED_OUTPUT_PORT PORTD
#else
#define PROBE_LED_PIN_BIT (OUTPUT_PROBE_LED_PIN - 8)
#define PROBE_LED_DDR DDRB
#define PROBE_LED_OUTPUT_PORT PORTB
#endif

#if DRO_TYPE1_ENABLED
#include <SPI.h>
#endif
// Some constants calculated here
unsigned long const minRpmTime = (((long) MIN_RPM_DELAY) * ((long) 1000));
long const longMax = __LONG_MAX__;
long const longMin = (- __LONG_MAX__ - (long) 1);
long const slowSc = ((long) 2000) / (((long) FILTER_SLOW_EMA) + ((long) 1));
long const fastSc = ((long) 20) / (((long) FILTER_FAST_EMA) + ((long) 1));

#if TACH_UPDATE_FREQUENCY == UPDATE_FREQUENCY
int const tachUpdateFrequencyCounterLimit = 1;
#else
int const tachUpdateFrequencyCounterLimit = (((long) UPDATE_FREQUENCY) / ((long) TACH_UPDATE_FREQUENCY));
#endif

int const updateFrequencyCounterLimit = (int) (((unsigned long) SCALE_CLK_FREQUENCY) /((unsigned long) UPDATE_FREQUENCY));
int const clockCounterLimit = (int) (((unsigned long) (F_CPU/8)) / (unsigned long) SCALE_CLK_FREQUENCY) - 10;
int const scaleClockDutyLimit = (int) (((unsigned long) (F_CPU/800)) * ((unsigned long) SCALE_CLK_DUTY) / (unsigned long) SCALE_CLK_FREQUENCY);
int const scaleClockFirstReadDelay = (int) ((unsigned long) F_CPU/4000000);

//variables that will store tach info and status
volatile unsigned long tachInterruptTimer;
volatile unsigned long tachInterruptRotationCount;

volatile unsigned long tachTimerStart;

//variables that will store the readout output
volatile unsigned long tachReadoutRotationCount;
volatile unsigned long tachReadoutMicrosec;
volatile unsigned long tachReadoutRpm;
unsigned long lastTachReadoutRpm;

#if TACH_AVERAGE_COUNT > 1
volatile unsigned long tachLastRead[TACH_AVERAGE_COUNT];
volatile int tachLastReadPosition;
#endif

volatile int tachUpdateFrequencyCounter;
volatile boolean sendTachData;

// variable to store the touch probe status.
volatile unsigned int probeReportedValue;

//variables that will store the DRO readout
volatile boolean tickTimerFlag;
volatile int updateFrequencyCounter;

// Axis count
#if SCALE_X_ENABLED > 0
volatile long xValue;
volatile long xReportedValue;
long xLastReportedValue;
long xZeroSetValue;
bool xAbsMode;
#endif
#if SCALE_X_AVERAGE_ENABLED > 0
volatile long axisLastReadX[AXIS_AVERAGE_COUNT];
volatile int axisLastReadPositionX;
volatile long axisAMAValueX;
#endif

#if SCALE_Y_ENABLED > 0
volatile long yValue;
volatile long yReportedValue;
long yLastReportedValue;
long yZeroSetValue;
bool yAbsMode;
#endif
#if SCALE_Y_AVERAGE_ENABLED > 0
volatile long axisLastReadY[AXIS_AVERAGE_COUNT];
volatile int axisLastReadPositionY;
volatile long axisAMAValueY;
#endif

#if SCALE_Z_ENABLED > 0
volatile long zValue;
volatile long zReportedValue;
long zLastReportedValue;
long zZeroSetValue;
bool zAbsMode;
#endif
#if SCALE_Z_AVERAGE_ENABLED > 0
volatile long axisLastReadZ[AXIS_AVERAGE_COUNT];
volatile int axisLastReadPositionZ;
volatile long axisAMAValueZ;
#endif

#if SCALE_W_ENABLED > 0
volatile long wValue;
volatile long wReportedValue;
long wLastReportedValue;
long wZeroSetValue;
bool wAbsMode;
#endif
#if SCALE_W_AVERAGE_ENABLED > 0
volatile long axisLastReadW[AXIS_AVERAGE_COUNT];
volatile int axisLastReadPositionW;
volatile long axisAMAValueW;
#endif

#if DRO_TYPE1_ENABLED > 0
volatile unsigned int encoderValue1;
volatile unsigned int encoderValue2;
volatile unsigned int encoderValue3;
volatile unsigned int encoderValue4;
#endif

//The setup function is called once at startup of the sketch
void setup() {
	cli();
	sendTachData = false;
	tickTimerFlag = false;
	updateFrequencyCounter = 0;

	loadSettings();

// Initialize DRO values
#if DRO_ENABLED > 0
	// use clock only for scale type 0
#if DRO_TYPE0_ENABLED	
	// clock pin should be set as output
	SCALE_CLK_DDR |= _BV(CLK_PIN_BIT);
	// set the clock pin to low
	SCALE_CLK_OUTPUT_PORT &= ~_BV(CLK_PIN_BIT);
#endif

	//data pins should be set as input for scale type 0 and as output for scale type 1
#if SCALE_X_ENABLED > 0
#if SCALE_X_TYPE == 0
	X_DDR &= ~_BV(X_PIN_BIT);
#elif SCALE_X_TYPE == 1
	X_DDR |= _BV(X_PIN_BIT);
#endif
	xValue = 0L;
	xReportedValue = 0L;
	xLastReportedValue = -99999L;
	xZeroSetValue = 0L;
	xAbsMode = true;
#if SCALE_X_AVERAGE_ENABLED > 0
	initializeAxisAverage(axisLastReadX, axisLastReadPositionX, axisAMAValueX);
#endif
#endif

#if SCALE_Y_ENABLED > 0
#if SCALE_Y_TYPE == 0
	Y_DDR &= ~_BV(Y_PIN_BIT);
#elif SCALE_Y_TYPE == 1
	Y_DDR |= _BV(Y_PIN_BIT);
#endif
	yValue = 0L;
	yReportedValue = 0L;
	yLastReportedValue = -99999L;
	yZeroSetValue = 0L;
	yAbsMode = true;
#if SCALE_Y_AVERAGE_ENABLED > 0
	initializeAxisAverage(axisLastReadY, axisLastReadPositionY, axisAMAValueY);
#endif
#endif

#if SCALE_Z_ENABLED > 0
#if SCALE_Z_TYPE == 0
	Z_DDR &= ~_BV(Z_PIN_BIT);
#elif SCALE_Z_TYPE == 1
	Z_DDR |= _BV(Z_PIN_BIT);
#endif
	zValue = 0L;
	zReportedValue = 0L;
	zLastReportedValue = -99999L;
	zZeroSetValue = 0L;
	zAbsMode = true;
#if SCALE_Z_AVERAGE_ENABLED > 0
	initializeAxisAverage(axisLastReadZ, axisLastReadPositionZ, axisAMAValueZ);
#endif
#endif

#if SCALE_W_ENABLED > 0
#if SCALE_W_TYPE == 0
	W_DDR &= ~_BV(W_PIN_BIT);
#elif SCALE_W_TYPE == 1
	W_DDR |= _BV(W_PIN_BIT);
#endif
	wValue = 0L;
	wReportedValue = 0L;
	wLastReportedValue = -99999L;
	wZeroSetValue = 0L;
	wAbsMode = true;
#if SCALE_W_AVERAGE_ENABLED > 0
	initializeAxisAverage(axisLastReadW, axisLastReadPositionW, axisAMAValueW);
#endif
#endif

// Initialize SPI and LS7366R registers for DRO type 1	
#if DRO_TYPE1_ENABLED	
	// SPI initialization
	SPI.begin();

#if SCALE_X_ENABLED > 0 && SCALE_X_TYPE == 1
	X_OUTPUT_PORT &= ~_BV(X_PIN_BIT);
	SPI.transfer(0x88); 
	SPI.transfer(0x03);
	X_OUTPUT_PORT |= _BV(X_PIN_BIT);
#endif

#if SCALE_Y_ENABLED > 0 && SCALE_Y_TYPE == 1
	Y_OUTPUT_PORT &= ~_BV(Y_PIN_BIT);
	SPI.transfer(0x88); 
	SPI.transfer(0x03);
	Y_OUTPUT_PORT |= _BV(Y_PIN_BIT);
#endif

#if SCALE_Z_ENABLED > 0 && SCALE_Z_TYPE == 1
	Z_OUTPUT_PORT &= ~_BV(Z_PIN_BIT);
	SPI.transfer(0x88); 
	SPI.transfer(0x03);
	Z_OUTPUT_PORT |= _BV(Z_PIN_BIT);
#endif

#if SCALE_W_ENABLED > 0 && SCALE_W_TYPE == 1
	W_OUTPUT_PORT &= ~_BV(W_PIN_BIT);
	SPI.transfer(0x88); 
	SPI.transfer(0x03);
	W_OUTPUT_PORT |= _BV(W_PIN_BIT);
#endif
#endif

#endif

	//initialize tach values
#if TACH_ENABLED > 0
	// Setup tach port for input
	TACH_DDR &= ~_BV(TACH_PIN_BIT);
	
#if OUTPUT_TACH_LED_ENABLED > 0	
	TACH_LED_DDR |= _BV(TACH_LED_PIN_BIT);
	// Set LED pin to LOW
	TACH_LED_OUTPUT_PORT &= ~_BV(TACH_LED_PIN_BIT);
#endif
	
	// Setup interrupt on tach pin
	PCICR |= _BV(TACH_INTERRUPT_REGISTER);
	TACH_INTERRUPT_MASK |= _BV(TACH_INTERRUPT_PIN);
	
	// Reset tach counter and timer
	tachInterruptRotationCount = 0;
	tachInterruptTimer = micros();
		
	tachTimerStart = tachInterruptTimer;

	tachReadoutRotationCount = 0;
	tachReadoutMicrosec = 0;
	
#if TACH_AVERAGE_COUNT > 1
	for (tachLastReadPosition = 0; tachLastReadPosition < (int) TACH_AVERAGE_COUNT; tachLastReadPosition++) {
		tachLastRead[tachLastReadPosition] = 0;
	}
	tachLastReadPosition = TACH_AVERAGE_COUNT - 1;
#endif
	tachUpdateFrequencyCounter = 0;
	lastTachReadoutRpm = -99999L;
#endif

	//initialize touch probe values
#if PROBE_ENABLED > 0
	// Setup tach port for input
	PROBE_DDR &= ~_BV(PROBE_PIN_BIT);
#if OUTPUT_PROBE_LED_ENABLED > 0
	PROBE_LED_DDR |= _BV(PROBE_LED_PIN_BIT);
	// Set LED pin to LOW
	PROBE_LED_OUTPUT_PORT &= ~_BV(PROBE_LED_PIN_BIT);
#endif
	// Set probe input to "not touching"
	probeReportedValue = 0;

#endif

	//initialize serial port
	Serial.begin(UART_BAUD_RATE);

	//initialize timers
	setupClkTimer();

	sei();	

	/* === Display DRO === */
	for (int i = 0; i < DISPLAY_COUNT; i++) {
		seven_seg.activateAllSegments();
		seven_seg.setIntensity(displayBrightness());
		seven_seg.clearMatrix();
	}

	for (int i = 0; i < DISPLAY_COUNT; i++) {
		for (int d = 0; d < 8; d++) {
			seven_seg.setChar(i, d, '-', false);
		}
	}

	for (int i = 0; i < ANALOG_BUTTON_COUNT; i++) {
		pinMode(buttonInputPins[i], INPUT);
	}

	currentState = show_values;
	lastState = invalid_state;
}
/* === Display DRO === */

/* formatDouble(value, width, precision) - format a number to fit on a display.
 *
 * formats the 'value' to fit in a character string of given 'width' with 
 * 'precision' numbers after the decimal point. 
 * 
 * Stores in a reverse format suitable for writing directly to seven segment display(s)
 * Requires 'width + 1' character buffer (for the trailing \0)
 * Inserts a leading zero when -1.0 < value < -1.0.
 * Sets 'high bit' for characters that should have the decimal point illuminated
 *
 * (2.33, 8, 3) => "____2.330"
 * (-0.001, 8, 3) => "___-0.001"
 * (-123.1, 8, 4) => "-123.1000"
 * (0.02, 8, 4) => "___0.0200"
 */
int formatDouble(double value, int width, int precision, char *buffer) {
	if (abs(precision) > width) {
		Serial.print("precision > width ");
		return 0;
	}

	// scale to a long integer and round off at precision.
	long scaled = 0L;
	if (precision > 0) {
		scaled = (long)round(value * pow(10.0, precision));
	} else {
		// if negative precision then we want to round off the last digit to 1/2's
		precision = -precision;
		scaled = (long)((round((value * pow(10, precision - 1)) * 2.0) / 2.0) * 10.0);
	}

	// check for overflow if a positive number
	if (scaled > 0 && scaled >= pow(10.0, width)) {
		Serial.print("overflow\n");
		return 0;
	}

	// save sign as a char (for later use) and get absolute value
	int sign = (value < 0) ? '-' : ' ';
	scaled = labs(scaled);

	// check for underflow if a negative number
	// Note we have one less digit for negative numbers, as we have a sign (-)
	if (sign == '-' && scaled >= pow(10.0, width - 1)) {
		Serial.print("underflow\n");
		return 0;
	}

	for (int i = 0; i < width; i++) {
		if (!scaled && i > precision) {
								// pad the string with spaces
			buffer[i] = sign;	// first time we append the sign character
			sign = ' ';			// every other time we will append space
		} else {
			buffer[i] = '0' + (scaled % 10);
			buffer[i] |= (char)((i == precision) ? 0x80 : 0x00);
			scaled /= 10;
		}
	}

	return 1;
}

int formatInteger(long value, int width, char *buffer) {
	// save sign as a char (for later use) and get absolute value
	int sign = (value < 0) ? '-' : ' ';
	value = labs(value);

	for (int i = 0; i < width; i++) {
		if (i && !value) {		// pad the string with spaces, always show at least 0
			buffer[i] = sign;	// first time we append the sign character
			sign = ' ';			// every other time we will append space
		} else {
			buffer[i] = '0' + (value % 10);
			value /= 10;
		}
	}

	return 1;
}

void showDoubleValue(double value, int displayAddress) {
	char buffer[10];
	if (formatDouble(value, DISPLAY_WIDTH, displayPrecision(), buffer)) {
		for (int i = 0; i < DISPLAY_WIDTH; i++) {
			seven_seg.setChar(displayAddress, i, (buffer[i] & 0x7f), (buffer[i] & 0x80));
		}
	}
}

void showIntValue(long value, int displayAddress) {
	char buffer[10];
	if (formatInteger(value, DISPLAY_WIDTH, buffer)) {
		for (int i = 0; i < DISPLAY_WIDTH; i++) {
			seven_seg.setChar(displayAddress, i, (buffer[i] & 0x7f), (buffer[i] & 0x80));
		}
	}
}

void showText(const char *text, int displayAddress) {
	for (int i = DISPLAY_WIDTH - 2; i >= 0; i--) {
		char displayChar = *text ? *(text++) : ' ';
		seven_seg.setChar(displayAddress, i, (displayChar & 0x7f), (displayChar & 0x80));
	}
}

void showMode(char mode, int displayAddress) {
	seven_seg.setChar(displayAddress, 7, mode, false);
}

void showMenu(const char *text) {
	showText(text, DISPLAY_MENU_ADDRESS);
}

void iFrameFilter() {
	if (millis() - iFrameLastTime > IFRAME_TIMEDELAY) {
		iFrameTrigger = true;
		iFrameLastTime = millis();
	} else {
		iFrameTrigger = false;
	}
}

/* debounce buttons on analog pin */
/* return 0x00 = no button, 1=button 1, 2=button 2, 3=button 3... */
int lastButtonValues[ANALOG_BUTTON_COUNT];

_interface_buttons debounceButtons() {
	unsigned int buttons = 0x0000;

	// // Hacky-debouncing in addition to the below debounce...	
	// if (millis() - lastButtonTime < ANALOG_IGNORE_TIME) {
	// 	return lastButtons;
	// }

	for (int i = 0; i < ANALOG_BUTTON_COUNT; i++) {
		int buttonValue = analogRead(buttonInputPins[i]);

		if (lastButtonValues[i] < ANALOG_LOW_SW_LEVEL && buttonValue < ANALOG_LOW_SW_LEVEL) {
			// Serial.println(buttonValue);
			buttons |= 0x01 << (i * 4);
		} else if (lastButtonValues[i] < ANALOG_MED_SW_LEVEL && buttonValue < ANALOG_MED_SW_LEVEL) {
			// Serial.println(buttonValue);
			buttons |= 0x02 << (i * 4);
		} else if (lastButtonValues[i] < ANALOG_HIGH_SW_LEVEL && buttonValue < ANALOG_HIGH_SW_LEVEL) {
			// Serial.println(buttonValue);
			buttons |= 0x04 << (i * 4);
		}

		lastButtonValues[i] = buttonValue;
	}

	return (_interface_buttons)buttons;
}

void zeroAllAxes() {
	xZeroSetValue = xReportedValue;
	xAbsMode = false;
	yZeroSetValue = yReportedValue;
	yAbsMode = false;
	zZeroSetValue = zReportedValue;
	zAbsMode = false;		
}

_state operatingState(_interface_buttons buttons) {
	_state nextState = show_values;

	if (lastState != show_values) {		// enter state
		showMenu("--------");
	}

	switch (buttons) {
		case button_none:	// no button press, ignore
			break;

		case button_x_zero:	// X0
			xZeroSetValue = xReportedValue;
			xAbsMode = false;
			break;

		case button_y_zero:	// Y0
			yZeroSetValue = yReportedValue;
			yAbsMode = false;
			break;

		case button_z_zero:	// Z0
			zZeroSetValue = zReportedValue;
			zAbsMode = false;
			break;

		case button_x_absinc:	// X abs/inc
			xAbsMode = !xAbsMode;
			break;

		case button_y_absinc:	// Y abs/inc
			yAbsMode = !yAbsMode;
			break;

		case button_z_absinc:	// Z abs/inc
			zAbsMode = !zAbsMode;
			break;

		case button_up:	// + set axis to 1/2 aka find center
			nextState = set_axis_half;
			break;

		case button_select: // > show menu
			nextState = show_menu;
			break;

		case button_down: // - zero all -
			zeroAllAxes();	
			break;

		default:	// fall through, invalid or no buttons, ignore
			break;
	}

	return nextState;
}

_state setAxisHalfState(_interface_buttons buttons) {
	_state nextState = set_axis_half;

	if (lastState != set_axis_half) {	// entering state
		showMode(MODE_CHAR_SEL, 0);
		showMode(MODE_CHAR_SEL, 1);
		showMode(MODE_CHAR_SEL, 2);
		showMenu("1-2 Axis");
	}

	switch (buttons) {
		case button_none:	// no button press, ignore
			break;

		case button_x_zero:	// X0
			xZeroSetValue = (xReportedValue + (xAbsMode ? 0 : xZeroSetValue) ) / 2;
			xAbsMode = false;
			nextState = show_values;
			break;

		case button_y_zero:	// Y0
			yZeroSetValue = (yReportedValue + (yAbsMode ? 0 : yZeroSetValue) ) / 2;
			yAbsMode = false;
			nextState = show_values;
			break;

		case button_z_zero:	// Z abs/inc
			zZeroSetValue = (zReportedValue + (zAbsMode ? 0 : zZeroSetValue) ) / 2;
			zAbsMode = false;
			nextState = show_values;
			break;

		default:	// any other button, cancel
			nextState = show_values;
			break;	// cancel and return to operating mode
	}

	return nextState;
}

_state reverseAxisState(_interface_buttons buttons) {
	_state nextState = reverse_axis;

	if (lastState != reverse_axis) {	// entering state
		showMode(MODE_CHAR_SEL, 0);
		showMode(MODE_CHAR_SEL, 1);
		showMode(MODE_CHAR_SEL, 2);
		showMenu("reverse");
	}

	switch (buttons) {
		case button_none:	// no button press
			break;

		case button_x_zero:	// X0
			setXAxisReversed(!xAxisReversed());
			nextState = show_values;
			break;

		case button_y_zero:	// Y0
			setYAxisReversed(!yAxisReversed());
			nextState = show_values;
			break;

		case button_z_zero:	// Z abs/inc
			setZAxisReversed(!zAxisReversed());
			nextState = show_values;
			break;

		default:	// any other button, cancel
			nextState = show_values;
			break;	// cancel and return to operating mode
	}

	return nextState;
}

_state setDisplayBrightnessState(_interface_buttons buttons) {
	_state nextState = set_brightness;

	showIntValue(displayBrightness(), 0);
	if (lastState != set_brightness) {	// entering state
		showMode(MODE_CHAR_SEL, 0);
		showMode(MODE_CHAR_SEL, 1);
		showMode(MODE_CHAR_SEL, 2);
		showMenu(" +up -dn");
	}

	switch (buttons) {
		case button_none:
			break;

		case button_up:	// +
			setDisplayBrightness(MIN((displayBrightness() + 1), 15));
			seven_seg.setIntensity(displayBrightness());
			break;

		case button_select: // >
			nextState = show_values;
			break;

		case button_down:	// -
			setDisplayBrightness(MAX((displayBrightness() - 1), 0));
			seven_seg.setIntensity(displayBrightness());
			break;

		default:	// fall through, invalid or no buttons, ignore
			break;
	}

	// Serial.print("I");
	// Serial.print(displayBrightness);
	// Serial.print(";");
	return nextState;
}

_state setDisplayPrecisionState(_interface_buttons buttons) {
	_state nextState = set_precision;

	if (lastState != set_precision) {	// entering state
		showMenu(" +up -dn");
	}

	switch (buttons) {
		case button_none:
			break;

		case button_up:	// +
			setDisplayPrecision(MIN(displayPrecision() + 1, 7));
			break;

		case button_select: // >
			nextState = show_values;
			break;

		case button_down:	// -
			setDisplayPrecision(MAX(displayPrecision() - 1, -7));
			break;

		default:	// fall through, invalid or no buttons, ignore
			break;
	}

	return nextState;
}

_state setAxisCPIState(_interface_buttons buttons) {
	_state nextState = show_values;

	return nextState;
}

_state setAxisValueState(_interface_buttons buttons) {
	_state nextState = show_values;

	return nextState;
}

_state setUnitsState(_interface_buttons buttons) {
	_state nextState = show_values;

	return nextState;
}

_state showDisplayMenuState(_interface_buttons buttons) {
	_state nextState = show_menu;

	switch (buttons) {
		case button_none:	// no button press, ignore
			break;

		case button_up:	// +
			menuSelected = MIN(++menuSelected, (sizeof(menu) / sizeof(_menu) - 1));
			break;

		case button_select: // >
			nextState = menu[menuSelected].state;
			break;

		case button_down:	// -
			menuSelected = MAX(--menuSelected, 0);
			break;

		default:	// fall through, invalid or no buttons, ignore
			// fall through to selected menu state
			switch (menu[menuSelected].state) {
				case set_axis_half:
					nextState = setAxisHalfState(buttons);
					break;

				case reverse_axis:
					nextState = reverseAxisState(buttons);
					break;
			}
			break;
	}

	showMenu(menu[menuSelected].title);
	return nextState;
}

void showState(_state state) {
	switch (state) {
		case invalid_state:
			Serial.print("Sinvalid;");
			break;

		case show_values:
			Serial.print("Sshow_values;");
			break;
		case show_menu:
			Serial.print("Sshow_menu;");
			break;

		case set_axis_count:
			Serial.print("Sset_axis_count;");
			break;

		case set_brightness:
			Serial.print("Sset_brightness;");
			break;

		case set_precision:
			Serial.print("Sset_precision;");
			break;

		case set_units:
			Serial.print("Sset_units;");
			break;

		case reverse_axis:
			Serial.print("Sreverse_axis;");
			break;

		case set_axis_half:
			Serial.print("Sset_axis_half;");
			break;

		case set_axis_value:
			Serial.print("Sset_axis_value;");
			break;

		case zero_all:
			Serial.print("Szero_all;");
			break;

		default:
			Serial.print("Sunknown;");
			break;
	}
}

bool checkSwitches() {
	bool updateDisplay = false;
	_state nextState = currentState;

	// if (currentState != lastState) {
	// 	showState(currentState);
	// }

	// Hacky-debouncing in addition to the below debounce...	
	if (millis() - lastButtonTime < ANALOG_IGNORE_TIME) {
		return false;
	}

	_interface_buttons buttons = debounceButtons();

	// // TODO: Implement 15 second menu timeout and return to operating mode
	// if (buttons == button_none && currentState != show_values && (millis() - lastButtonTime) > SUBMODE_TIMEOUT) {
	// 	nextState = show_values;
	// 	// Serial.println("timeout");
	// }

	if (buttons != lastButtons) {
		if (buttons != 0) {
			Serial.print("B");
			Serial.print(buttons);
			Serial.print(";");
		}

		switch (currentState) {
			case show_values:
				nextState = operatingState(buttons);
				break;

			case set_axis_half:
				nextState = setAxisHalfState(buttons);
				break;

			case show_menu:
				nextState = showDisplayMenuState(buttons);
				break;

			case set_brightness:
				nextState = setDisplayBrightnessState(buttons);
				break;

			case zero_all:
				zeroAllAxes();	
				nextState = show_values;
				break;

			case reverse_axis:
				nextState = reverseAxisState(buttons);
				break;

			case set_axis_value:
				// TODO: Implement set_axis_value state
				nextState = setAxisValueState(buttons);
				break;

			case set_axis_count:
				// TODO: Implement set_axis_count state
				nextState = setAxisCPIState(buttons);
				break;

			case set_precision:
				// TODO: Implement set_precision state
				nextState = setDisplayPrecisionState(buttons);
				break;

			case set_units:
				// TODO: Implement set_units state
				nextState = show_values;
				break;

			default:
				break;			
		}

		lastButtons = buttons;
		updateDisplay = true;	// signal a redraw when button state changes
	}

	lastState = currentState;
	if (nextState != currentState) {
		currentState = nextState;

		Serial.print("S");
		Serial.print(currentState);
		Serial.print(";");

		updateDisplay = true;	// signal a redraw when state changes
	}

	return updateDisplay;
}
/* === Display DRO === */

// Read touch probe status
#if PROBE_ENABLED > 0
inline unsigned int readProbeOutputData() {
	if (PROBE_INPUT_PORT & _BV(PROBE_PIN_BIT)) {
	// Return probe signal 
#if PROBE_INVERT == 0
#if OUTPUT_PROBE_LED_ENABLED > 0
		PROBE_LED_OUTPUT_PORT |= _BV(PROBE_LED_PIN_BIT);
#endif
		return 1;
#else
#if OUTPUT_PROBE_LED_ENABLED > 0
		PROBE_LED_OUTPUT_PORT &= ~_BV(PROBE_LED_PIN_BIT);
#endif
		return 0;
#endif
	} else {
#if PROBE_INVERT == 0
#if OUTPUT_PROBE_LED_ENABLED > 0
		PROBE_LED_OUTPUT_PORT &= ~_BV(PROBE_LED_PIN_BIT);
#endif
		return  0;
#else
#if OUTPUT_PROBE_LED_ENABLED > 0
		PROBE_LED_OUTPUT_PORT |= _BV(PROBE_LED_PIN_BIT);
#endif
		return 1;
#endif
	}
}
#endif

bool isValueState(_state state) {
	return (state == show_values) || (state = set_precision);
}

// The loop function is called in an endless loop
void loop() {
	if (tickTimerFlag) {
		tickTimerFlag = false;

		iFrameFilter();	// slow the dispatch of events for older tablets
		if (checkSwitches()) {
			iFrameTrigger = true;
		}

#if DRO_ENABLED > 0
#if DRO_TYPE1_ENABLED
		readEncoders();
#endif
		//print DRO positions to the serial port
#if SCALE_X_ENABLED > 0
#if SCALE_X_AVERAGE_ENABLED > 0
		scaleValueRounded(xReportedValue, axisLastReadX, axisLastReadPositionX, axisAMAValueX);
#endif
		if (xReportedValue != xLastReportedValue || iFrameTrigger) {
			Serial.print(F("X"));
			Serial.print((long)xReportedValue);
			Serial.print(F(";"));
			if (isValueState(currentState)) {
				double x = (double)(xReportedValue - (xAbsMode ? 0 : xZeroSetValue)) / xCountPerInch();
				// TODO: Factor out hardcoded display number for X Axis
				showDoubleValue((xAxisReversed() ? -x : x), 0);
				showMode(xAbsMode ? MODE_CHAR_ABS : MODE_CHAR_INC, 0);
			}
			xLastReportedValue = xReportedValue;
		}
#endif

#if SCALE_Y_ENABLED > 0
#if SCALE_Y_AVERAGE_ENABLED > 0
		scaleValueRounded(yReportedValue, axisLastReadY, axisLastReadPositionY, axisAMAValueY);
#endif
		if(yReportedValue != yLastReportedValue || iFrameTrigger) {
			Serial.print(F("Y"));
			Serial.print((long)yReportedValue);
			Serial.print(F(";"));
			if (isValueState(currentState)) {
				double y = (double)(yReportedValue - (yAbsMode ? 0 : yZeroSetValue)) / yCountPerInch();
				// TODO: Factor out hardcoded display number for Y Axis
				showDoubleValue((yAxisReversed() ? -y : y), 1);
				showMode(yAbsMode ? MODE_CHAR_ABS : MODE_CHAR_INC, 1);
			}
			yLastReportedValue = yReportedValue;
		}
#endif

#if SCALE_Z_ENABLED > 0
#if SCALE_Z_AVERAGE_ENABLED > 0
		scaleValueRounded(zReportedValue, axisLastReadZ, axisLastReadPositionZ, axisAMAValueZ);
#endif
		if(zReportedValue != zLastReportedValue || iFrameTrigger) {
			Serial.print(F("Z"));
			Serial.print((long)zReportedValue);
			Serial.print(F(";"));
			if (isValueState(currentState)) {
				double z = (double)(zReportedValue - (zAbsMode ? 0 : zZeroSetValue)) / zCountPerInch();
				// TODO: Factor out hardcoded display number for Z Axis
				showDoubleValue((zAxisReversed() ? -z : z), 2);
				showMode(zAbsMode ? MODE_CHAR_ABS : MODE_CHAR_INC, 2);
			}
			zLastReportedValue = zReportedValue;
		}
#endif

#if SCALE_W_ENABLED > 0
#if SCALE_W_AVERAGE_ENABLED > 0
		scaleValueRounded(wReportedValue, axisLastReadW, axisLastReadPositionW, axisAMAValueW);
#endif
		if(wReportedValue != wLastReportedValue || iFrameTrigger) {
			Serial.print(F("W"));
			Serial.print((long)wReportedValue);
			Serial.print(F(";"));
			if (isValueState(currentState)) {
				double w = (double)(wReportedValue - (wAbsMode ? 0 : wZeroSetValue)) / wCountPerInch();
				// TODO: Factor out hardcoded display number for W Axis
				showDoubleValue((wAxisReversed() ? -w : w), 3);
				showMode(wAbsMode ? MODE_CHAR_ABS : MODE_CHAR_INC, 3);
			}
			wLastReportedValue = wReportedValue;
		}
#endif

#endif

		// print Tach rpm to serial port
#if TACH_ENABLED > 0
		// Calculate tach data
		sendTachData = sendTachOutputData() || sendTachData;

		// Check tach reporting frequency
		tachUpdateFrequencyCounter++;
		if (tachUpdateFrequencyCounter >= tachUpdateFrequencyCounterLimit) {
			tachUpdateFrequencyCounter = 0;

			// Output tach data
			if (sendTachData && (lastTachReadoutRpm != tachReadoutRpm || iFrameTrigger)) {
				sendTachData = false;

				Serial.print(F("T"));
#if TACH_RAW_DATA_FORMAT > 0
				Serial.print((unsigned long)tachReadoutMicrosec);
				Serial.print(F("/"));
				Serial.print((unsigned long)tachReadoutRotationCount);
#else
				Serial.print((unsigned long)tachReadoutRpm);
#endif
				Serial.print(F(";"));
				if (isValueState(currentState) && currentState != set_precision) {
					// TODO: Factor out hardcoded display number for RPM
					showIntValue(tachReadoutRpm, 3);
				}
				lastTachReadoutRpm = tachReadoutRpm;
			}

		}
#endif

		// print Touch Probe data to serial port
#if PROBE_ENABLED > 0
		// Calculate tach data
		probeReportedValue = readProbeOutputData();

		Serial.print(F("P"));
		Serial.print((unsigned int)probeReportedValue);
		Serial.print(F(";"));
#endif
	}
}

//initializes clock timer
void setupClkTimer() {
	updateFrequencyCounter = 0;

	TCCR2A = 0;			// set entire TCCR2A register to 0
	TCCR2B = 0;			// same for TCCR2B

	// set compare match registers
#if DRO_TYPE0_ENABLED > 0
	OCR2A = scaleClockDutyLimit;			// default 44 = 22us
#else
	OCR2A = clockCounterLimit - 1;
#endif
	OCR2B = clockCounterLimit;			// default 222 = 111us

	// turn on Fast PWM mode
	TCCR2A |= _BV(WGM21) | _BV(WGM20);

	// Set CS21 bit for 8 prescaler //CS20 for no prescaler
	TCCR2B |= _BV(CS21);

	//initialize counter value to start at low pulse
#if DRO_TYPE0_ENABLED > 0
	TCNT2  = scaleClockDutyLimit + 1;
#else
	TCNT2  = 0;
#endif
	// enable timer compare interrupt A and B
	TIMSK2 |= _BV(OCIE2A) | _BV(OCIE2B);
}


/* Interrupt Service Routines */

// Timer 2 interrupt B ( Switches clock pin from low to high 21 times) at the end of clock counter limit
ISR(TIMER2_COMPB_vect) {
	// Set counter back to zero  
	TCNT2  = 0;  
#if DRO_TYPE0_ENABLED > 0
	// Only set the clock high if updateFrequencyCounter less than 21
	if (updateFrequencyCounter < SCALE_CLK_PULSES) {
		// Set clock pin high
		SCALE_CLK_OUTPUT_PORT |= _BV(CLK_PIN_BIT);
	}
#endif
}

// Timer 2 interrupt A ( Switches clock pin from high to low) at the end of clock PWM Duty counter limit
ISR(TIMER2_COMPA_vect)  {
#if DRO_TYPE0_ENABLED > 0
	// Control the scale clock for only first 21 loops
	if (updateFrequencyCounter < SCALE_CLK_PULSES) {
	
		// Set clock low if high and then delay 2us
		if (SCALE_CLK_OUTPUT_PORT & _BV(CLK_PIN_BIT)) {
			SCALE_CLK_OUTPUT_PORT &= ~_BV(CLK_PIN_BIT);
			TCNT2  = scaleClockDutyLimit - scaleClockFirstReadDelay;
			return;
		}

		// read the pin state and shift it into the appropriate variables
		// Logic by Les Jones:
		//	If data pin is HIGH set bit 20th of the axis value to '1'.  Then shift axis value one bit to the right
		//  This is called 20 times (for bits received from 0 to 19)
		if (updateFrequencyCounter < SCALE_CLK_PULSES - 1) {
#if SCALE_X_ENABLED > 0 && SCALE_X_TYPE == 0
			if (X_INPUT_PORT & _BV(X_PIN_BIT))
				xValue |= ((long)0x00100000 );
			xValue >>= 1;
#endif

#if SCALE_Y_ENABLED > 0 && SCALE_Y_TYPE == 0
			if (Y_INPUT_PORT & _BV(Y_PIN_BIT))
				yValue |= ((long)0x00100000 );
			yValue >>= 1;
#endif

#if SCALE_Z_ENABLED > 0 && SCALE_Z_TYPE == 0
			if (Z_INPUT_PORT & _BV(Z_PIN_BIT))
				zValue |= ((long)0x00100000 );
			zValue >>= 1;
#endif

#if SCALE_W_ENABLED > 0 && SCALE_W_TYPE == 0
			if (W_INPUT_PORT & _BV(W_PIN_BIT))
				wValue |= ((long)0x00100000 );
			wValue >>= 1;
#endif


		} else if (updateFrequencyCounter == SCALE_CLK_PULSES - 1) {

			//If 21-st bit is 'HIGH' inverse the sign of the axis readout
#if SCALE_X_ENABLED > 0 && SCALE_X_TYPE == 0
			if (X_INPUT_PORT & _BV(X_PIN_BIT))
				xValue |= ((long)0xfff00000);
			xReportedValue = xValue;
			xValue = 0L;
#endif

#if SCALE_Y_ENABLED > 0 && SCALE_Y_TYPE == 0
			if (Y_INPUT_PORT & _BV(Y_PIN_BIT))
				yValue |= ((long)0xfff00000);
			yReportedValue = yValue;
			yValue = 0L;
#endif

#if SCALE_Z_ENABLED > 0 && SCALE_Z_TYPE == 0
			if (Z_INPUT_PORT & _BV(Z_PIN_BIT))
				zValue |= ((long)0xfff00000);
			zReportedValue = zValue;
			zValue = 0L;
#endif

#if SCALE_W_ENABLED > 0 && SCALE_W_TYPE == 0
			if (W_INPUT_PORT & _BV(W_PIN_BIT))
				wValue |= ((long)0xfff00000);
			wReportedValue = wValue;
			wValue = 0L;
#endif
			// Tell the main loop, that it's time to sent data
			tickTimerFlag = true;

		}
	}
#else
	if (updateFrequencyCounter == 0) {
		// Tell the main loop, that it's time to sent data
		tickTimerFlag = true;
	}
#endif
	
	updateFrequencyCounter++;
	// Start of next cycle 
	if ( updateFrequencyCounter >= updateFrequencyCounterLimit) {
		updateFrequencyCounter = 0;
	}
}


#if DRO_TYPE1_ENABLED
inline void readEncoders() {

#if SCALE_X_ENABLED > 0 && SCALE_X_TYPE == 1
	X_OUTPUT_PORT &= ~_BV(X_PIN_BIT);
	readEncoderValue();
	X_OUTPUT_PORT |= _BV(X_PIN_BIT);
    xReportedValue= ((long)encoderValue1<<24) + ((long)encoderValue2<<16) + ((long)encoderValue3<<8) + (long)encoderValue4;
#endif

#if SCALE_Y_ENABLED > 0 && SCALE_Y_TYPE == 1
	Y_OUTPUT_PORT &= ~_BV(Y_PIN_BIT);
	readEncoderValue();
	Y_OUTPUT_PORT |= _BV(Y_PIN_BIT);
    yReportedValue= ((long)encoderValue1<<24) + ((long)encoderValue2<<16) + ((long)encoderValue3<<8) + (long)encoderValue4;
#endif

#if SCALE_Z_ENABLED > 0 && SCALE_Z_TYPE == 1
	Z_OUTPUT_PORT &= ~_BV(Z_PIN_BIT);
	readEncoderValue();
	Z_OUTPUT_PORT |= _BV(Z_PIN_BIT);
    zReportedValue= ((long)encoderValue1<<24) + ((long)encoderValue2<<16) + ((long)encoderValue3<<8) + (long)encoderValue4;
#endif

#if SCALE_W_ENABLED > 0 && SCALE_W_TYPE == 1
	W_OUTPUT_PORT &= ~_BV(W_PIN_BIT);
	readEncoderValue();
	W_OUTPUT_PORT |= _BV(W_PIN_BIT);
    wReportedValue= ((long)encoderValue1<<24) + ((long)encoderValue2<<16) + ((long)encoderValue3<<8) + (long)encoderValue4;
#endif
}

inline void readEncoderValue() {
	SPI.transfer(0x60); // Request count
	encoderValue1 = SPI.transfer(0x00); // Read highest order byte
	encoderValue2 = SPI.transfer(0x00);
	encoderValue3 = SPI.transfer(0x00);
	encoderValue4 = SPI.transfer(0x00); // Read lowest order byte
}

#endif


#if DRO_ENABLED > 0
#if SCALE_AVERAGE_ENABLED > 0
inline void	initializeAxisAverage(volatile long axisLastRead[], volatile int &axisLastReadPosition, volatile long &axisAMAValue) {
	for (axisLastReadPosition = 0; axisLastReadPosition < (int) AXIS_AVERAGE_COUNT; axisLastReadPosition++) {
		axisLastRead[axisLastReadPosition] = 0;
	}
	axisLastReadPosition = 0;
	axisAMAValue = 0;
}

inline void scaleValueRounded(volatile long &ReportedValue, volatile long axisLastRead[], volatile int &axisLastReadPosition, volatile long &axisAMAValue) {
	int last_pos; 
	int first_pos;
	int prev_pos;
	int filter_pos;

	long dir;
	long minValue = longMax;
	long maxValue = longMin;
	long volatility = 0;
	long valueRange;
	long ssc;
	long constant;
	long delta;

	// Save current read and increment position 
	axisLastRead[axisLastReadPosition] = ReportedValue;
	last_pos = axisLastReadPosition;

	axisLastReadPosition++;
	if (axisLastReadPosition == (int) AXIS_AVERAGE_COUNT) {
		axisLastReadPosition = 0;
	}
	first_pos = axisLastReadPosition;
	
    dir = (axisLastRead[first_pos] - axisLastRead[last_pos]) * ((long) 100);

    // Calculate the volatility in the counts by taking the sum of the differences
    prev_pos = first_pos;
    for (filter_pos = (first_pos + 1) % AXIS_AVERAGE_COUNT;
         filter_pos != first_pos;
         filter_pos = (filter_pos + 1) % AXIS_AVERAGE_COUNT) {
        minValue = MIN(minValue, axisLastRead[filter_pos]);
        maxValue = MAX(maxValue, axisLastRead[filter_pos]);
        volatility += ABS(axisLastRead[filter_pos] - axisLastRead[prev_pos]);
        prev_pos = filter_pos;
    }

    // Just return the read if there is no volatility to avoid divide by 0
    if (volatility == (long) 0) {
		axisAMAValue = axisLastRead[last_pos] * ((long) 100);
		return;
    }
	
    // If the last AMA is not within twice the sample range, then assume the position jumped
    // and reset the AMA to the current read
	maxValue = maxValue * ((long) 100);
	minValue = minValue * ((long) 100);
    valueRange = maxValue - minValue;
    if (axisAMAValue > maxValue + valueRange + ((long) 100) ||
        axisAMAValue < minValue - valueRange - ((long) 100)) {
		axisAMAValue = axisLastRead[last_pos] * ((long) 100);
		return;
    }

    // Calculate the smoothing constant
    ssc = (ABS(dir / volatility) * fastSc) + slowSc;
    constant = (ssc * ssc) / ((long) 10000);

    // Calculate the new average
	delta = axisLastRead[last_pos] - (axisAMAValue / ((long) 100));
	axisAMAValue = axisAMAValue + constant * delta; 

    ReportedValue = (axisAMAValue + ((long) 50)) / ((long) 100);
	return;
}

inline long MIN(long value1, long value2) {
	if (value1 > value2) {
		return value2;
	} else {
		return value1;
	}
}

inline long MAX(long value1, long value2) {
	if (value1 > value2) {
		return value1;
	} else {
		return value2;
	}
}

inline long ABS(long value) {
	if (value < 0) {
		return -value;
	} else {
		return value;
	}
}

#endif
#endif

// Calculate the tach rpm 
#if TACH_ENABLED > 0
inline boolean sendTachOutputData() {
	unsigned long microSeconds;
	unsigned long tachRotationCount;
	unsigned long tachTimer;
	unsigned long currentMicros;

	// Read data from the last interrupt (stop interrupts to read a pair in sync)
	cli();
	tachRotationCount = tachInterruptRotationCount;
	tachInterruptRotationCount = 0;
	tachTimer = tachInterruptTimer;
	sei();
		
	// reset values and ignore this readout if clock or rotation counter overlapses
	if (tachTimer < tachTimerStart) {
		tachTimerStart = tachTimer;
		return false;
	}
		
	
	if (tachRotationCount != 0) {
		// We have at least one tick on rpm sensor so calculate the time between ticks
		tachReadoutRotationCount = tachRotationCount;
		tachReadoutMicrosec = tachTimer - tachTimerStart;

		tachTimerStart = tachTimer;
	} else { 	
		// if no ticks on rpm sensor...
		currentMicros = micros();
		// reset timer if clock overlapses
		if (currentMicros < tachTimerStart) {
			tachTimerStart = 0;
			return false;
		} else {
			// if no pulses for longer than minRpmTime then set rpm to zero
			microSeconds = currentMicros - tachTimerStart;
			if (microSeconds > minRpmTime ) {
				tachReadoutRotationCount = 0;
				tachReadoutMicrosec = 0;
			} else {
				return false;
			}
		}
	}

#if TACH_RAW_DATA_FORMAT == 0
	// Calculate RPM 
	if (tachReadoutRotationCount == 0) {
		tachReadoutRpm = 0;
	} else {
		unsigned long averageTime = tachReadoutMicrosec/tachReadoutRotationCount;
		// Ignore when time is zero
		if (averageTime == 0) {
			return false;
		} else {
			tachReadoutRpm = ((unsigned long) 600000000 / averageTime);
			tachReadoutRpm = ((unsigned long) tachReadoutRpm/TACH_PRESCALE) + 5;
			tachReadoutRpm = ((unsigned long) tachReadoutRpm / 10);
		}
	}

#if TACH_AVERAGE_COUNT > 1
	// calculate Average RPM
	unsigned long tachReadSum;
	unsigned long tachLastReadRpm;
	int readCounter;
	int readCounted;

	// Save last reading
	tachLastReadRpm = tachLastRead[tachLastReadPosition];
	
	// Increment tachLastReadPosition
	tachLastReadPosition++;
	if (tachLastReadPosition == (int) TACH_AVERAGE_COUNT) {
		tachLastReadPosition = 0;
	}
	// Save current read 
	tachLastRead[tachLastReadPosition] = tachReadoutRpm;

	// At least two consecutive measurements must be valid to calculate average
	readCounted = 0;
	tachReadSum = 0;
	if (tachReadoutRpm != 0 && tachLastReadRpm != 0) { 
		// Calculate average read
		for (readCounter = 0; readCounter < (int) TACH_AVERAGE_COUNT; readCounter++) {
			if (tachLastRead[readCounter] != 0) {
				tachReadSum = tachReadSum + tachLastRead[readCounter];
				readCounted++;
			}
		}
	}
	if (readCounted != 0) {
		tachReadoutRpm = ((unsigned long) tachReadSum / ((int) readCounted));
	} else {
		tachReadoutRpm = 0;
	}
#endif

#if TACH_ROUND > 0
	// calculate Rounded RPM
	unsigned long tachReadRoundFactor;

	// fixed threasholds rounding
#if TACH_ROUND > 1
	if (tachReadoutRpm <200) {
		tachReadRoundFactor = 1;
	} else if (tachReadoutRpm <500) {
		tachReadRoundFactor = 2;
	} else if (tachReadoutRpm <2000) {
		tachReadRoundFactor = 5;
	} else if (tachReadoutRpm <5000) {
		tachReadRoundFactor = 10;
	} else if (tachReadoutRpm <20000) {
		tachReadRoundFactor = 20;
	} else if (tachReadoutRpm <50000) {
		tachReadRoundFactor = 50;
	} else {
		tachReadRoundFactor = 100;
	}

	// 1% rounding
#else
	// Determine rounding factor
	tachReadRoundFactor = (unsigned long) ((tachReadoutRpm * 10)/((int) 100));
	tachReadRoundFactor = ((unsigned long) tachReadRoundFactor/10);
	if (tachReadRoundFactor == 0) {
		tachReadRoundFactor = 1;
	}
#endif
	
	// Round result
	tachReadoutRpm = ((unsigned long) ((tachReadoutRpm * 10)/tachReadRoundFactor) + 5);
	tachReadoutRpm = ((unsigned long) tachReadoutRpm/10);
	tachReadoutRpm = ((unsigned long) tachReadoutRpm * tachReadRoundFactor);
#endif

#endif

	return true;
}
#endif


// Interrupt to read tach pin change
#if TACH_ENABLED > 0
ISR(TACH_INTERRUPT_VECTOR) {
	if (TACH_INPUT_PORT & _BV(TACH_PIN_BIT)) {
		// record timestamp of change in port input
		tachInterruptTimer = micros();
		tachInterruptRotationCount++;
#if OUTPUT_TACH_LED_ENABLED > 0	
	    TACH_LED_OUTPUT_PORT |= _BV(TACH_LED_PIN_BIT);
#endif
	} else {
#if OUTPUT_TACH_LED_ENABLED > 0	
		TACH_LED_OUTPUT_PORT &= ~_BV(TACH_LED_PIN_BIT);
#endif
	}
}
#endif
