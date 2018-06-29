/*
 ArduinoDRO + Tach V5
 
 iGaging/AccuRemote Digital Scales Controller V3.3
 Created 5 July 2014
 Update 15 July 2014
 Copyright (C) 2014 Yuriy Krushelnytskiy, http://www.yuriystoys.com
 
 Updated 08 August 2014 by Ryszard Malinowski
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

 Added support for tachometer on axis T with accurate timing
 Added option to send rpm row data (time and count)

 
 NOTE: This program supports hall-sensor to measure rpm.  The tach output format for Android DRO is T<time>/<retation>.
 Android DRO application must support this format for axis T.
 
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

	TACH_RAW_DATA_FORMAT
		Defines the format of tach data sent to serial port.
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
		Defines the Frequency in Hz (number of timer per second the scales are read and the data is sent to the application).
		Possible values:
			any integer number between 1 and 64 
		Default value = 24
		
	
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

// Tach data format
#define TACH_RAW_DATA_FORMAT 0			// single value format: T<rpm>;

// Tach RPM config
#define MIN_RPM_DELAY 1200				// 1.2 sec calculates to low range = 50 rpm.

#define INPUT_TACH_PIN 7

#define TACH_LED_PIN 13

// General Settings
#define UART_BAUD_RATE 9600				//  Set this so it matches the BT module's BAUD rate 
#define UPDATE_FREQUENCY 24				//  Frequency in Hz (number of timer per second the scales are read and the data is sent to the application)

//---END OF CONFIGURATION PARAMETERS ---


//---DO NOT CHANGE THE CODE BELOW UNLESS YOU KNOW WHAT YOU ARE DOING ---

/* iGaging Clock Settins (do not change) */
#define SCALE_CLK_PULSES 21				//iGaging and Accuremote sclaes use 21 bit format
#define SCALLE_CLK_FREQUENCY 9000		//iGaging scales run at about 9-10KHz


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

unsigned long const minRpmTime = (((long) MIN_RPM_DELAY) * ((long) 1000));


//variables that will store tach info and status
volatile unsigned long tachInterruptTimer;
volatile unsigned long tachInterruptRotationCount;

volatile unsigned long tachTimerStart;

//variables that will store the readout output
volatile unsigned long tachReadoutRotationCount;
volatile unsigned long tachReadoutMicrosec;
volatile unsigned long tachReadoutRpm;
volatile boolean tachReadoutSendData = false;


//variables that will store the DRO readout
volatile boolean tickTimerFlag;
byte bitOffset ;
byte clockPinHigh;

volatile long xValue;			//X axis count
volatile long yValue;			//Y axis count
volatile long zValue;			//Z axis count
volatile long wValue;			//W axis count


//The setup function is called once at startup of the sketch
void setup()
{
	cli();
	tickTimerFlag = false;

#if DRO_ENABLED > 0
	// initialize DRO valuse
	bitOffset = 0;
	clockPinHigh = 0;
	xValue = 0L;
	yValue = 0L;
	zValue = 0L;
	wValue = 0L;
	
	//clock pin should be set as output
	SCALE_CLK_DDR |= _BV(CLK_PIN_BIT);

	//data pins should be set as inputs
#if SCALE_X_ENABLED > 0
		X_DDR &= ~_BV(X_PIN_BIT);
#endif
#if SCALE_Y_ENABLED > 0
		Y_DDR &= ~_BV(Y_PIN_BIT);
#endif
#if SCALE_Z_ENABLED > 0
		Z_DDR &= ~_BV(Z_PIN_BIT);
#endif
#if SCALE_W_ENABLED > 0
		W_DDR &= ~_BV(W_PIN_BIT);
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
	tachReadoutSendData = false;
#endif


	//initialize serial port
	Serial.begin(UART_BAUD_RATE);

	//initialize timers
	startTickTimer(UPDATE_FREQUENCY);			//init and start the "system tick" timer

#if DRO_ENABLED > 0
	setupClkTimer(SCALLE_CLK_FREQUENCY);			//init the scale clock timer (don't start it yet)
#endif

	sei();	

}


// The loop function is called in an endless loop
void loop()
{

	if (tickTimerFlag) {
		tickTimerFlag = false;

#if DRO_ENABLED > 0
		//reset the reader for the next batch (tachometer should not be cleared here)
		bitOffset = 0;
		//print DRO positions to the serial port
#if SCALE_X_ENABLED > 0
		Serial.print(F("X"));
		Serial.print((long)xValue);
		Serial.print(F(";"));
		xValue = 0;
#endif
#if SCALE_Y_ENABLED > 0
		Serial.print(F("Y"));
		Serial.print((long)yValue);
		Serial.print(F(";"));
			yValue = 0;
#endif
#if SCALE_Z_ENABLED > 0
		Serial.print(F("Z"));
		Serial.print((long)zValue);
		Serial.print(F(";"));
		zValue = 0;
#endif
#if SCALE_W_ENABLED > 0
		Serial.print(F("W"));
		Serial.print((long)wValue);
		Serial.print(F(";"));
		wValue = 0;
#endif

#endif

		// print Tach rpm to serial port
#if TACH_ENABLED > 0

		// Format tach data
		formatTachOutput();

		// output tach data
		if (tachReadoutSendData) {
			Serial.print(F("T"));
			if (TACH_RAW_DATA_FORMAT > 0) {
				Serial.print((unsigned long)tachReadoutMicrosec);
				Serial.print(F("/"));
				Serial.print((unsigned long)tachReadoutRotationCount);
			} else {
				Serial.print((unsigned long)tachReadoutRpm);
			}
			Serial.print(F(";"));
			tachReadoutSendData = false;
		}
#endif

#if DRO_ENABLED > 0
		//start reading again
		startClkTimer();
#endif
	}
}


void startTickTimer(int frequency) 
{
	//set timer1 interrupt
	TCCR1A = 0;			// set entire TCCR1A register to 0
	TCCR1B = 0;			// same for TCCR1B
	TCNT1  = 0;			//initialize counter value to 0

	// set compare match register
	OCR1A = F_CPU / (1024 * frequency) - 1;		//must be <65536

	// turn on CTC mode
	TCCR1B |= (1 << WGM12);

	// Set CS10 and CS12 bits for 1024 prescaler
	TCCR1B |= (1 << CS12) | (1 << CS10);

	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);
}


//initializes the timer for the scale clock but does not start it
#if DRO_ENABLED > 0
void setupClkTimer(int frequency)
{
	bitOffset = 0;

	TCCR2A = 0;			// set entire TCCR2A register to 0
	TCCR2B = 0;			// same for TCCR2B

	// set compare match register
	OCR2A = F_CPU / (8 * frequency) - 1; 	// 160 - 1;

	// turn on CTC mode
	TCCR2A |= (1 << WGM21);

	// Set CS21 bit for 8 prescaler //CS20 for no prescaler
	TCCR2B |= (1 << CS21);
}
#endif

//starts scale clock timer
#if DRO_ENABLED > 0
void startClkTimer()
{
	//initialize counter value to 0
	TCNT2  = 0;
	// enable timer compare interrupt
	TIMSK2 |= (1 << OCIE2A);
}
#endif

//stops scale clock timer
#if DRO_ENABLED > 0
void stopClkTimer()
{
	// disable timer compare interrupt
	TIMSK2 &= ~(1 << OCIE2A);
}
#endif


/* Interrupt Service Routines */

//timer 1 compare interrupt service routine (provides system tick)
ISR(TIMER1_COMPA_vect) 
{
	tickTimerFlag = 1;
}

//Timer 2 interrupt (scale clock driver)
#if DRO_ENABLED > 0
ISR(TIMER2_COMPA_vect) 
{
	//scale reading happens here
	if (!clockPinHigh) {
		SCALE_CLK_OUTPUT_PORT |= _BV(CLK_PIN_BIT);
		clockPinHigh = 1;
	} else {
		SCALE_CLK_OUTPUT_PORT &= ~_BV(CLK_PIN_BIT);
		clockPinHigh = 0;

		//read the pin state and shift it into the appropriate variables
#if SCALE_X_ENABLED > 0
		xValue |= ((long)(X_INPUT_PORT & _BV(X_PIN_BIT) ? 1 : 0) << bitOffset);
#endif

#if SCALE_Y_ENABLED > 0
		yValue |= ((long)(Y_INPUT_PORT & _BV(Y_PIN_BIT) ? 1 : 0) << bitOffset);
#endif

#if SCALE_Z_ENABLED > 0
		zValue |= ((long)(Z_INPUT_PORT & _BV(Z_PIN_BIT) ? 1 : 0) << bitOffset);
#endif

#if SCALE_W_ENABLED > 0
		wValue |= ((long)(W_INPUT_PORT & _BV(W_PIN_BIT) ? 1 : 0) << bitOffset);
#endif

		//increment the bit offset
		bitOffset++;

		if (bitOffset >= SCALE_CLK_PULSES) {
			//stop the timer after the predefined number of pulses
			stopClkTimer();
#if SCALE_X_ENABLED > 0
			if (X_INPUT_PORT & _BV(X_PIN_BIT))
				xValue |= ((long)0x7ff << 21);
#endif

#if SCALE_Y_ENABLED > 0
			if (Y_INPUT_PORT & _BV(Y_PIN_BIT))
				yValue |= ((long)0x7ff << 21);
#endif

#if SCALE_Z_ENABLED > 0
			if (Z_INPUT_PORT & _BV(Z_PIN_BIT))
				zValue |= ((long)0x7ff << 21);
#endif

#if SCALE_W_ENABLED > 0
			if (W_INPUT_PORT & _BV(W_PIN_BIT))
				wValue |= ((long)0x7ff << 21);
#endif
		}
	}
}
#endif


// Calculate the tach rpm 
#if TACH_ENABLED > 0
inline void formatTachOutput()
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
		return;
	}
		
	// We have at least one tick on rpm sensor so calculate the time between ticks
	if (tachRotationCount != 0) {
		tachReadoutRotationCount = tachRotationCount;
		tachReadoutMicrosec = tachTimer - tachTimerStart;

		// Ignore readout that is too low
		if (tachReadoutMicrosec <= minRpmTime) {
			tachReadoutSendData = true;
		}

		tachTimerStart = tachTimer;

	// if no ticks on rpm sensor...
	} else {
		currentMicros = micros();
		// reset timer if clock overlapses
		if (currentMicros < tachTimerStart) {
			tachTimerStart = 0;
			return;
		} else {
			// if no pulses for longer than minRpmTime then set rpm to zero
			microSeconds = currentMicros - tachTimerStart;
			if (microSeconds > minRpmTime ) {
				tachReadoutRotationCount = 0;
				tachReadoutMicrosec = 0;
				tachReadoutRpm = 0;
				tachReadoutSendData = true;
				return;
			}
		}
	}
		
	if (TACH_RAW_DATA_FORMAT == 0) {
		if (tachReadoutSendData) {
			unsigned long averageTime = tachReadoutMicrosec/tachReadoutRotationCount;
			if (averageTime != 0) {
				tachReadoutRpm = ((unsigned long) 600000000 / averageTime) + 5;
				tachReadoutRpm = ((unsigned long) tachReadoutRpm / 10);
			} else {
				tachReadoutSendData = false;
			}
		}
	}
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

