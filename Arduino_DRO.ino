/*
 ArduinoDRO + Tach V5.5
 
 iGaging/AccuRemote Digital Scales Controller V3.3
 Created 5 July 2014
 Update 15 July 2014
 Copyright (C) 2014 Yuriy Krushelnytskiy, http://www.yuriystoys.com
 
 
 Updated 17 August 2014 by Ryszard Malinowski
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
 Version 5.2 - Added option to prescale tach reading compensating for more than one tach pulse per rotation.
 Version 5.3 - Added option to average and round tach output values.
 Version 5.3 - Added option to select max tach update frequency
 Version 5.4 - Replace Yuriy's method of clocking scales with method written by Les Jones
 Version 5.5 - Optimizing the scale reading logic using method written by Les Jones
 
 
 NOTE: This program supports pulse sensor to measure rpm.  
 
 Configuration parameters:
	SCALE_<n>_ENABLED
		Defines if DRO functionality on axis <n> should be supported.  
		If supported DRO scale should be connected to I/O pin defined in constant "<n>DataPin" and 
		DRO data is sent to serial port with corresponding axis prefix (X, Y, Z or W)
		Clock pin is common for all scales should be connected to I/O pin defined in constant "clockPin" 
		Possible values:
			1 = DRO functionality on axis <n> is supported
			0 = DRO functionality on axis <n> is not supported
		Default value = 1

	SCALE_CLK_PIN
		Defines the I/O pin where clock signal for all DRO scales is connected
		Possible values:
			integer number between 2 and 13
		Default value = 2

	SCALE_<n>_PIN
		Defines the I/O pin where DRO data signal for selected scale is connected
		Possible values:
			integer number between 2 and 13
		Default values = 3, 4, 5, 6 (for corresponding axis X, Y, Z and W)

	TACH_ENABLED
		Defines if tach sensor functionality should be supported.  
		If supported tach sensor should be connected to I/O pin defined in constant "tachPin" and 
		rpm value is sent to serial port with axis prefix "T" 
		Possible values:
			1 = tach sensor functionality is supported
			0 = tach sensor functionality is not supported
		Default value = 1

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
		Note: This value is not used when TACH_RAW_DATA_FORMAT is enabled 
		Possible values:
			1 = exact measured tach reading is sent
			any integer number greater than 1 - average tach reading is sent 
		Default value = 6

	TACH_ROUND
		Defines if tach reading should be rounded to the nearest 1% of current measured rpm.   
		If rounding is enabled the reading is rounded by 1% of current rpm.  
		For example measured rpm between 950 and 1049 will be rounded to the nearest 10 rpm (reporting 950, 960, 970 etc.)
		Note: This value is not used when TACH_RAW_DATA_FORMAT is enabled 
		Possible values:
			0 = exact measured tach reading is sent
			1 = tach reading is rounded to the nearest 1% of measured rpm
		Default value = 1

	TACH_RAW_DATA_FORMAT
		Defines the format of tach data sent to serial port.
		Note: when rad data format is used, then TACH_PRESCALE, TACH_AVERAGE_COUNT and TACH_ROUND are ignored 
		Possible values:
			1 = tach data is sent in raw (two values) format: T<total_time>/<number_of_pulses>;
			0 = tach data is sent in single value format: T<rpm>;
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

	INPUT_TACH_PIN
		Defines the I/O pin where tach sensor signal is connected
		Possible values:
			integer number between 2 and 13
		Default value = 7

	TACH_LED_PIN
		Defines the I/O pin where the tach LED feedback is connected.  
		Tach LED feedback indicates the status of tachPin for debugging purposes
		Possible values:
			integer number between 2 and 13
		Default value = 13 (on-board LED)

	UART_BAUD_RATE
		Defines the serial port boud rate.  Make sure it matches the Bluetooth module's boud rate.
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
		Default value = 8
	
 */
 

// DRO config (if axis is not connected change in the corresponding constant value from "1" to "0")
#define SCALE_X_ENABLED 1
#define SCALE_Y_ENABLED 1
#define SCALE_Z_ENABLED 1
#define SCALE_W_ENABLED 1

// I/O ports config (change pin numbers if DRO, Tach sensor or Tach LED feedback is connected to different ports)
#define SCALE_CLK_PIN 2

#define SCALE_X_PIN 3
#define SCALE_Y_PIN 4
#define SCALE_Z_PIN 5
#define SCALE_W_PIN 6

// System config (if Tach is not connected change in the corresponding constant value from "1" to "0")
#define TACH_ENABLED 1

// Tach prescale value (number of tach sensor pulses per revolution)
#define  TACH_PRESCALE 1

// Number of tach measurments to average 
#define TACH_AVERAGE_COUNT 6

// This is 1% rounding for tachometer display (set to 0 to disable)
#define TACH_ROUND 1

// Tach data format
#define TACH_RAW_DATA_FORMAT 0			// single value format: T<rpm>;

// Tach RPM config
#define MIN_RPM_DELAY 1200				// 1.2 sec calculates to low range = 50 rpm.

#define INPUT_TACH_PIN 7

#define TACH_LED_PIN 13

// General Settings
#define UART_BAUD_RATE 9600				//  Set this so it matches the BT module's BAUD rate 
#define UPDATE_FREQUENCY 24				//  Frequency in Hz (number of timer per second the scales are read and the data is sent to the application)
#define TACH_UPDATE_FREQUENCY 8			//  Max Frequency in Hz (number of timer per second) the tach output is sent to the application

//---END OF CONFIGURATION PARAMETERS ---


//---DO NOT CHANGE THE CODE BELOW UNLESS YOU KNOW WHAT YOU ARE DOING ---

/* iGaging Clock Settins (do not change) */
#define SCALE_CLK_PULSES 21				//iGaging and Accuremote sclaes use 21 bit format
#define SCALE_CLK_FREQUENCY 9000		//iGaging scales run at about 9-10KHz
#define SCALE_CLK_DUTY 20				// iGaging scales clock run at 20% PWM duty (22us = ON out of 111us cycle)


#if (SCALE_X_ENABLED > 0) || (SCALE_Y_ENABLED > 0) || (SCALE_Z_ENABLED > 0) || (SCALE_W_ENABLED > 0)
#define DRO_ENABLED 1
#else
#define DRO_ENABLED 0
#endif

// Define registers and pins for ports
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
#else
#define X_PIN_BIT (SCALE_X_PIN - 8)
#define X_DDR DDRB
#define X_INPUT_PORT PINB
#endif

#if SCALE_Y_PIN < 8 
#define Y_PIN_BIT SCALE_Y_PIN
#define Y_DDR DDRD
#define Y_INPUT_PORT PIND
#else
#define Y_PIN_BIT (SCALE_Y_PIN - 8)
#define Y_DDR DDRB
#define Y_INPUT_PORT PINB
#endif

#if SCALE_Z_PIN < 8 
#define Z_PIN_BIT SCALE_Z_PIN
#define Z_DDR DDRD
#define Z_INPUT_PORT PIND
#else
#define Z_PIN_BIT (SCALE_Z_PIN - 8)
#define Z_DDR DDRB
#define Z_INPUT_PORT PINB
#endif

#if SCALE_W_PIN < 8 
#define W_PIN_BIT SCALE_W_PIN
#define W_DDR DDRD
#define W_INPUT_PORT PIND
#else
#define W_PIN_BIT (SCALE_W_PIN - 8)
#define W_DDR DDRB
#define W_INPUT_PORT PINB
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

#if TACH_LED_PIN < 8 
#define LED_PIN_BIT TACH_LED_PIN
#define LED_DDR DDRD
#define LED_OUTPUT_PORT PORTD
#else
#define LED_PIN_BIT (TACH_LED_PIN - 8)
#define LED_DDR DDRB
#define LED_OUTPUT_PORT PORTB
#endif

// Some constants calculated here
unsigned long const minRpmTime = (((long) MIN_RPM_DELAY) * ((long) 1000));

#if TACH_UPDATE_FREQUENCY == UPDATE_FREQUENCY
int const tachUpdateFrequencyCounterLimit = 0;
#else
int const tachUpdateFrequencyCounterLimit = (((long) UPDATE_FREQUENCY) / ((long) TACH_UPDATE_FREQUENCY));
#endif

int const updateFrequencyCounterLimit = (int) (((unsigned long) SCALE_CLK_FREQUENCY) /((unsigned long) UPDATE_FREQUENCY));
int const clockCounterLimit = (int) (((unsigned long) 2000000) / (unsigned long) SCALE_CLK_FREQUENCY) - 10;
#if DRO_ENABLED > 0
int const scaleClockDutyLimit = (int) (((unsigned long) 20000) * ((unsigned long) SCALE_CLK_DUTY) / (unsigned long) SCALE_CLK_FREQUENCY);
#endif

//variables that will store tach info and status
volatile unsigned long tachInterruptTimer;
volatile unsigned long tachInterruptRotationCount;

volatile unsigned long tachTimerStart;

//variables that will store the readout output
volatile unsigned long tachReadoutRotationCount;
volatile unsigned long tachReadoutMicrosec;
volatile unsigned long tachReadoutRpm;

#if TACH_AVERAGE_COUNT > 1
volatile unsigned long tachLastRead[TACH_AVERAGE_COUNT];
volatile int tachLastReadPosition;
#endif

volatile int tachUpdateFrequencyCounter;

//variables that will store the DRO readout
volatile boolean tickTimerFlag;
volatile int updateFrequencyCounter;

// Axis count
#if SCALE_X_ENABLED > 0
volatile long xValue;
volatile long xReportedValue;
#endif
#if SCALE_Y_ENABLED > 0
volatile long yValue;
volatile long yReportedValue;
#endif
#if SCALE_Z_ENABLED > 0
volatile long zValue;
volatile long zReportedValue;
#endif
#if SCALE_W_ENABLED > 0
volatile long wValue;
volatile long wReportedValue;
#endif





//The setup function is called once at startup of the sketch
void setup()
{
	cli();
	tickTimerFlag = false;
	updateFrequencyCounter = 0;

// Initialize DRO values
#if DRO_ENABLED > 0
	
	// clock pin should be set as output
	SCALE_CLK_DDR |= _BV(CLK_PIN_BIT);
	// set the clock pin to low
	SCALE_CLK_OUTPUT_PORT &= ~_BV(CLK_PIN_BIT);

	//data pins should be set as inputs
#if SCALE_X_ENABLED > 0
		X_DDR &= ~_BV(X_PIN_BIT);
	xValue = 0L;
	xReportedValue = 0L;
#endif
#if SCALE_Y_ENABLED > 0
		Y_DDR &= ~_BV(Y_PIN_BIT);
	yValue = 0L;
	yReportedValue = 0L;
#endif
#if SCALE_Z_ENABLED > 0
		Z_DDR &= ~_BV(Z_PIN_BIT);
	zValue = 0L;
	zReportedValue = 0L;
#endif
#if SCALE_W_ENABLED > 0
		W_DDR &= ~_BV(W_PIN_BIT);
	wValue = 0L;
	wReportedValue = 0L;
#endif

#endif
	

	//initialize tach values
#if TACH_ENABLED > 0
	// Setup tach port for input
	TACH_DDR &= ~_BV(TACH_PIN_BIT);
	
	LED_DDR |= _BV(LED_PIN_BIT);
	// Set LED pin to LOW
	LED_OUTPUT_PORT &= ~_BV(LED_PIN_BIT);
	
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
	tachLastReadPosition = 0;
#endif
	tachUpdateFrequencyCounter = 0;

#endif

	//initialize serial port
	Serial.begin(UART_BAUD_RATE);

	//initialize timers
	setupClkTimer();

	sei();	

}


// The loop function is called in an endless loop
void loop()
{

	if (tickTimerFlag) {
		tickTimerFlag = false;

#if DRO_ENABLED > 0
		//print DRO positions to the serial port
#if SCALE_X_ENABLED > 0
		Serial.print(F("X"));
		Serial.print((long)xReportedValue);
		Serial.print(F(";"));
#endif
#if SCALE_Y_ENABLED > 0
		Serial.print(F("Y"));
		Serial.print((long)yReportedValue);
		Serial.print(F(";"));
#endif
#if SCALE_Z_ENABLED > 0
		Serial.print(F("Z"));
		Serial.print((long)zReportedValue);
		Serial.print(F(";"));
#endif
#if SCALE_W_ENABLED > 0
		Serial.print(F("W"));
		Serial.print((long)wReportedValue);
		Serial.print(F(";"));
#endif

#endif

		// print Tach rpm to serial port
#if TACH_ENABLED > 0

		// Check tach reporting frequency
		tachUpdateFrequencyCounter++;
		if (tachUpdateFrequencyCounter >= tachUpdateFrequencyCounterLimit) {
			tachUpdateFrequencyCounter = 0;

			// Output tach data
			if (sendTachOutputData()) {
				Serial.print(F("T"));
#if TACH_RAW_DATA_FORMAT > 0
				Serial.print((unsigned long)tachReadoutMicrosec);
				Serial.print(F("/"));
				Serial.print((unsigned long)tachReadoutRotationCount);
#else
				Serial.print((unsigned long)tachReadoutRpm);
#endif
				Serial.print(F(";"));
			}
		}
#endif
	}
}


//initializes clock timer
void setupClkTimer()
{
	updateFrequencyCounter = 0;

	TCCR2A = 0;			// set entire TCCR2A register to 0
	TCCR2B = 0;			// same for TCCR2B

	// set compare match registers
#if DRO_ENABLED > 0
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
#if DRO_ENABLED > 0
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
#if DRO_ENABLED > 0
	// Only set the clock high if updateFrequencyCounter less than 21
	if (updateFrequencyCounter < SCALE_CLK_PULSES) {
		// Set clock pin high
		SCALE_CLK_OUTPUT_PORT |= _BV(CLK_PIN_BIT);
	}
#endif
}


// Timer 2 interrupt A ( Switches clock pin from high to low) at the end of clock PWM Duty counter limit
ISR(TIMER2_COMPA_vect) 
{
#if DRO_ENABLED > 0

	// Contorl the scale clock for only first 21 loops
	if (updateFrequencyCounter < SCALE_CLK_PULSES) {
	
		// Set clock low
		SCALE_CLK_OUTPUT_PORT &= ~_BV(CLK_PIN_BIT);

		// read the pin state and shift it into the appropriate variables
		// Logic by Les Jones:
		//	If data pin is HIGH set bit 20th of the axis value to '1'.  Then shift axis value one bit to the right
		//  This is called 20 times (for bits received from 0 to 19)
		if (updateFrequencyCounter < SCALE_CLK_PULSES - 1) {
#if SCALE_X_ENABLED > 0
			if (X_INPUT_PORT & _BV(X_PIN_BIT))
				xValue |= ((long)0x00100000 );
			xValue >>= 1;
#endif

#if SCALE_Y_ENABLED > 0
			if (Y_INPUT_PORT & _BV(Y_PIN_BIT))
				yValue |= ((long)0x00100000 );
			yValue >>= 1;
#endif

#if SCALE_Z_ENABLED > 0
			if (Z_INPUT_PORT & _BV(Z_PIN_BIT))
				zValue |= ((long)0x00100000 );
			zValue >>= 1;
#endif

#if SCALE_W_ENABLED > 0
			if (W_INPUT_PORT & _BV(W_PIN_BIT))
				wValue |= ((long)0x00100000 );
			wValue >>= 1;
#endif


		} else if (updateFrequencyCounter == SCALE_CLK_PULSES - 1) {

			//If 21-st bit is 'HIGH' inverse the sign of the axis readout
#if SCALE_X_ENABLED > 0
			if (X_INPUT_PORT & _BV(X_PIN_BIT))
				xValue |= ((long)0xfff00000);
			xReportedValue = xValue;
			xValue = 0L;
#endif

#if SCALE_Y_ENABLED > 0
			if (Y_INPUT_PORT & _BV(Y_PIN_BIT))
				yValue |= ((long)0xfff00000);
			yReportedValue = yValue;
			yValue = 0L;
#endif

#if SCALE_Z_ENABLED > 0
			if (Z_INPUT_PORT & _BV(Z_PIN_BIT))
				zValue |= ((long)0xfff00000);
			zReportedValue = zValue;
			zValue = 0L;
#endif

#if SCALE_W_ENABLED > 0
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


// Calculate the tach rpm 
#if TACH_ENABLED > 0
inline boolean sendTachOutputData()
{
	unsigned long microSeconds;
	unsigned long tachRotationCount;
	unsigned long tachTimer;
	unsigned long currentMicros;


	// Read data from the last interrupt (stop interupts to read a pair in sync)
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
		
	// We have at least one tick on rpm sensor so calculate the time between ticks
	if (tachRotationCount != 0) {
		tachReadoutRotationCount = tachRotationCount;
		tachReadoutMicrosec = tachTimer - tachTimerStart;

		tachTimerStart = tachTimer;

	// if no ticks on rpm sensor...
	} else {
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
	int readCounter;
	// Rotate tachLastReadPosition
	if (tachLastReadPosition == (int) TACH_AVERAGE_COUNT) {
		tachLastReadPosition = 0;
	}
	// Save current read and increment position 
	tachLastRead[tachLastReadPosition] = tachReadoutRpm;
	tachLastReadPosition++;
	// Calculate average read
	tachReadSum = 0;
	for (readCounter = 0; readCounter < (int) TACH_AVERAGE_COUNT; readCounter++) {
		tachReadSum = tachReadSum + tachLastRead[readCounter];
	}
	tachReadoutRpm = ((unsigned long) tachReadSum / ((int) TACH_AVERAGE_COUNT));
#endif

#if TACH_ROUND > 0
	// calculate Rounded RPM
	unsigned long tachReadRoundFactor;

	// Determine rounding factor
	tachReadRoundFactor = (unsigned long) ((tachReadoutRpm * 10)/((int) 100) + 5);
	tachReadRoundFactor = ((unsigned long) tachReadRoundFactor/10);
	if (tachReadRoundFactor == 0) {
		tachReadRoundFactor = 1;
	}
	
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
ISR(TACH_INTERRUPT_VECTOR)
{
	if (TACH_INPUT_PORT & _BV(TACH_PIN_BIT)) {
		// record timestamp of change in port input
		tachInterruptTimer = micros();
		tachInterruptRotationCount++;
	    LED_OUTPUT_PORT |= _BV(LED_PIN_BIT);
	} else {
	// read tach port and output it to LED
		LED_OUTPUT_PORT &= ~_BV(LED_PIN_BIT);
	}
}
#endif

