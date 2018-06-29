/*
 ArduinoDRO + Tach V4.1
 
 Reading Grizzly iGaging Digital Scales V2.1 Created 19 January 2012
 Updated 03 April 2013
 by Yuriy Krushelnytskiy
 http://www.yuriystoys.com
 
 Updated 06 August 2014 by Ryszard Malinowski
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

 Added support for tachometer on axis T (input pin 7) handled by interrupt routine
 Added option to send rpm data instead of row format data
 

 NOTE: This program supports hall-sensor to measure rpm.  The tach output format for Android DRO is T<time>/<retation>.
 Android DRO application must support this format for axis T.
 
 Configuration parameters:
	<n>AxisSupported
		Defines if DRO functionality on axis <n> should be supported.  
		If supported DRO scale should be connected to I/O pin defined in constant "<n>DataPin" and 
		DRO data is sent to serial port with corresponding axis prefix (X, Y, Z or W)
		Clock pin is common for all scales should be connected to I/O pin defined in constant "clockPin" 
		Possible values:
			true = DRO functionality on axis <n> is supported
			false = DRO functionality on axis <n> is not supported
		Default value = true

	tachSupported
		Defines if tach sensor functionality should be supported.  
		If supported tach sensor should be connected to I/O pin defined in constant "tachPin" and 
		rpm value is sent to serial port with axis prefix "T" 
		Possible values:
			true = tach sensor functionality is supported
			false = tach sensor functionality is not supported
		Default value = true

	tachRawDataFormat
		Defines the format of tach data sent to serial port.
		Possible values:
			true = tach data is sent in raw (two values) format: T<total_time>/<number_of_pulses>;
			false = tach data is sent in single value format: T<rpm>;
		Default value = false

	minRpmDelay
		Defines the delay (in milliseconds) in showing 0 when rotation stops.  If rpm is so low and time between tach pulse
		changes longer than this value, value zero rpm ("T0;") will be sent to the serial port.
		Note: this number will determine the slowest rpm that can be measured.  In order to measure smaller rpm I suggest 
		      to use a sensor with more than one "ticks per revolution" (for example hall sensor with two or more magnets).
		      The number of "ticks per revolution" should be set in tachometer setting in Android app.
		Possible values:
			any integer number > 0
		Default value = 1200 (the minimum rpm measured will be 50 rpm)
		
	clockPin
		Defines the I/O pin where clock signal for all DRO scales is connected
		Possible values:
			integer number between 2 and 13
		Default value = 2

	<n>DataPin
		Defines the I/O pin where DRO data signal for selected scale is connected
		Possible values:
			integer number between 2 and 13
		Default values = 3, 4, 5, 6 (for corresponding axis X, Y, Z and W)

	tachPin
		Defines the I/O pin where tach sensor signal is connected
		Possible values:
			integer number between 2 and 13
		Default value = 7

	tachLedFeedbackPin
		Defines the I/O pin where tach LED feedback is connected.  
		Tach LED feedback indicates the status of tachPin for debugging purposes
		Possible values:
			integer number between 2 and 13
		Default value = 13 (on-board LED)
	
 */
 
// DRO config (if axis is not connected change in the corresponding constant value from "true" to "false")
boolean const xAxisSupported = true;
boolean const yAxisSupported = true;
boolean const zAxisSupported = true;
boolean const wAxisSupported = true;

// System config (if Tach is not connected change in the corresponding constant value from "true" to "false")
boolean const tachSupported = true;
// Tach data format
boolean const tachRawDataFormat = false;			// single value format: T<rpm>;

// Tach RPM config
long const minRpmDelay = 1200;			// 1.2 sec calculates to low range = 50 rpm.
 
// I/O ports config (change pin numbers if DRO, Tach sensor or Tach LED feedback is connected to different ports)
int const clockPin = 2;

int const xDataPin = 3;
int const yDataPin = 4;
int const zDataPin = 5;
int const wDataPin = 6;

int const tachPin = 7;
int const tachLedFeedbackPin = 13;


//---END OF CONFIGURATION PARAMETERS ---


boolean const droSupported = (xAxisSupported || yAxisSupported || zAxisSupported || wAxisSupported);
boolean const tachInterrupt2 = (tachPin >= 2 && tachPin <= 7);
boolean const tachInterrupt0 = (tachPin >= 8 && tachPin <= 13);
unsigned long const minRpmTime = (minRpmDelay * 1000);


//variables that will store tach info and status
volatile unsigned long tachInterruptTimer;
volatile unsigned long tachInterruptRotationCount;

volatile unsigned long tachTimerStart;

//variables that will store the readout output
volatile unsigned long tachReadoutRotationCount;
volatile unsigned long tachReadoutMicrosec;
volatile unsigned long tachReadoutRpm;
volatile boolean tachReadoutSendData;


//variables that will store the DRO readout
volatile long xCoord;
volatile long yCoord;
volatile long zCoord;
volatile long wCoord;


//The setup function is called once at startup of the sketch
void setup()
{
	//clock pin should be set as output
	if (droSupported)
		pinMode(clockPin, OUTPUT);

	//data pins should be set as inputs
	if (xAxisSupported)
		pinMode(xDataPin, INPUT);

	if (yAxisSupported)
		pinMode(yDataPin, INPUT);

	if (zAxisSupported)
		pinMode(zDataPin, INPUT);

	if (wAxisSupported)
		pinMode(wDataPin, INPUT);

	//initialize tach values
	if (tachSupported) {
		pinMode(tachPin, INPUT);
		pinMode(tachLedFeedbackPin, OUTPUT);
		// switch interrupts off while messing with their settings  
		cli();
		tachInterruptRotationCount = 0;
		tachInterruptTimer = micros();
		
		tachTimerStart = tachInterruptTimer;

		tachReadoutRotationCount = 0;
		tachReadoutMicrosec = 0;
		tachReadoutSendData = false;

		// Setup interrupt on tach pin
		if (tachPin == 2) {
			PCICR |= _BV(PCIE2);	// Enable interrupt PCINT2
			PCMSK2 |= _BV(PCINT18); //Trigger on change of Pin D2
		} else if (tachPin == 3) {
			PCICR |= _BV(PCIE2);	// Enable interrupt PCINT2
			PCMSK2 |= _BV(PCINT19); //Trigger on change of Pin D3
		} else if (tachPin == 4) {
			PCICR |= _BV(PCIE2);	// Enable interrupt PCINT2
			PCMSK2 |= _BV(PCINT20); //Trigger on change of Pin D4
		} else if (tachPin == 5) {
			PCICR |= _BV(PCIE2);	// Enable interrupt PCINT2
			PCMSK2 |= _BV(PCINT21); //Trigger on change of Pin D5
		} else if (tachPin == 6) {
			PCICR |= _BV(PCIE2);	// Enable interrupt PCINT2
			PCMSK2 |= _BV(PCINT22); //Trigger on change of Pin D6
		} else if (tachPin == 7) {
			PCICR |= _BV(PCIE2);	// Enable interrupt PCINT2
			PCMSK2 |= _BV(PCINT23); //Trigger on change of Pin D7
		} else if (tachPin == 8) {
			PCICR |= _BV(PCIE0);	// Enable interrupt PCINT0
			PCMSK2 |= _BV(PCINT0); //Trigger on change of Pin D8
		} else if (tachPin == 9) {
			PCICR |= _BV(PCIE0);	// Enable interrupt PCINT0
			PCMSK2 |= _BV(PCINT1); //Trigger on change of Pin D9
		} else if (tachPin == 10) {
			PCICR |= _BV(PCIE0);	// Enable interrupt PCINT0
			PCMSK2 |= _BV(PCINT2); //Trigger on change of Pin D10
		} else if (tachPin == 11) {
			PCICR |= _BV(PCIE0);	// Enable interrupt PCINT0
			PCMSK2 |= _BV(PCINT3); //Trigger on change of Pin D11
		} else if (tachPin == 12) {
			PCICR |= _BV(PCIE0);	// Enable interrupt PCINT0
			PCMSK2 |= _BV(PCINT4); //Trigger on change of Pin D12
		} else if (tachPin == 13) {
			PCICR |= _BV(PCIE0);	// Enable interrupt PCINT0
			PCMSK2 |= _BV(PCINT5); //Trigger on change of Pin D13
		}
		sei();	
	}


	//initialize serial port
	Serial.begin(9600);


}


// The loop function is called in an endless loop
void loop()
{

	if (droSupported)
	{
		xCoord = 0;
		yCoord = 0;
		zCoord = 0;
		wCoord = 0;

		int bitOffset;

		//read the first 20 bits
		for(bitOffset = 0; bitOffset<21; bitOffset++)
		{
			tickTock();

			//read the pin state and shift it into the appropriate variables
			if (xAxisSupported)
				xCoord |= ((long)digitalRead(xDataPin)<<bitOffset);
			if (yAxisSupported)
				yCoord |= ((long)digitalRead(yDataPin)<<bitOffset);
			if (zAxisSupported)
				zCoord |= ((long)digitalRead(zDataPin)<<bitOffset);
			if (wAxisSupported)
				wCoord |= ((long)digitalRead(wDataPin)<<bitOffset);

		}

		tickTock();

		//read the last bit (signified the sign)
		//if it's high, fill 11 leftmost bits with "1"s
		if (xAxisSupported){
			if(digitalRead(xDataPin)==HIGH)
				xCoord |= ((long)0x7ff << 21);
		}

		if (yAxisSupported){
			if(digitalRead(yDataPin)==HIGH)
				yCoord |= ((long)0x7ff << 21);
		}

		if (zAxisSupported){
			if(digitalRead(zDataPin)==HIGH)
				zCoord |= ((long)0x7ff << 21);
		}

		if (wAxisSupported){
			if(digitalRead(wDataPin)==HIGH)
				wCoord |= ((long)0x7ff << 21);
		}

	
		//print DRO positions to the serial port
		if (xAxisSupported){
			Serial.print("X");
			Serial.print((long)xCoord);
			Serial.print(";");
		}
		if (yAxisSupported){
			Serial.print("Y");
			Serial.print((long)yCoord);
			Serial.print(";");
		}
		if (zAxisSupported){
			Serial.print("Z");
			Serial.print((long)zCoord);
			Serial.print(";");
		}
		if (wAxisSupported){
			Serial.print("W");
			Serial.print((long)wCoord);
			Serial.print(";");
		}
	}

	// print Tach rpm to serial port
	if (tachSupported) {
		// Format tach average
		formatTachOutput();

	// output tach data
		if (tachReadoutSendData) {
			Serial.print("T");
			if (tachRawDataFormat) {
				Serial.print((unsigned long)tachReadoutMicrosec);
				Serial.print("/");
				Serial.print((unsigned long)tachReadoutRotationCount);
			} else {
				Serial.print((unsigned long)tachReadoutRpm);
			}
			Serial.print(";");
			tachReadoutSendData = false;
		}

	//give the scales time to become ready again but continue reading tach input;
	}
	delay(50);
}


// Clock DRO scales
inline void tickTock()
{
	//tick
	digitalWrite(clockPin, HIGH);

	//If the scale output is floating all over the place comment lines 99-102 and uncomment line 106.

	//Alternative 1: Use "software" delay (works better on UNO)
	//give the scales a few microseconds to think about it
	for(byte i = 0; i<20; i++)
	{
		__asm__("nop\n\t");
	}

	//Alternative 2: Use proper delay
	//give the scales a few microseconds to think about it
	//delayMicroseconds(2);

	//tock
	digitalWrite(clockPin, LOW);
}


// Calculate the tach rpm 
inline void formatTachOutput()
{
	if (tachSupported) {
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
		
		if (!tachRawDataFormat) {
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
}


// Interrupt to read tach pin change (tach pin 2 - 7)
ISR(PCINT2_vect)
{
	if (tachSupported && tachInterrupt2) {
		if (digitalRead(tachPin) == HIGH) {
			// record timestamp of change in port input
			tachInterruptTimer = micros();
			tachInterruptRotationCount++;
			digitalWrite(tachLedFeedbackPin, HIGH);
		} else {
		// read tach port and output it to LED
			digitalWrite(tachLedFeedbackPin, LOW);
		}
	}
}


// Interrupt to read tach pin change (tach pin 8 - 13)
ISR(PCINT0_vect)
{
	if (tachSupported && tachInterrupt0) {
		if (digitalRead(tachPin) == HIGH) {
			// record timestamp of change in port input
			tachInterruptTimer = micros();
			tachInterruptRotationCount++;
			digitalWrite(tachLedFeedbackPin, HIGH);
		} else {
		// read tach port and output it to LED
			digitalWrite(tachLedFeedbackPin, LOW);
		}
	}
}
