/*
 ArduinoDRO + Tach V3
 
 Reading Grizzly iGaging Digital Scales V2.1 Created 19 January 2012
 Updated 03 April 2013
 by Yuriy Krushelnytskiy
 http://www.yuriystoys.com
 
 Updated 01 June 2014 by Ryszard Malinowski
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

 Added support for tachometer on axis T (input pin 7)

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

	minRpmDelay
		Defines the delay (in milliseconds) in showing 0 when rotation stops.  If rpm is so low and time between tach pulse
		changes longer than this value, value zero rpm ("T0;") will be sent to the serial port.
		Note: this number will determine the slowest rpm that can be measured.  In order to measure smaller rpm I suggest 
		      to use a sensor with more than one "ticks per revolution" (for example hall sensor with two or more magnets).
		      The number of "ticks per revolution" should be set in tachometer setting in Android app.
		Possible values:
			any integer number > 0
		Default value = 1200 (the minimum rpm measured will be 50 rpm)
	
 */
 

// DRO config (if axis is not connected change in the corresponding constant value from "true" to "false")
boolean const xAxisSupported = true;
boolean const yAxisSupported = true;
boolean const zAxisSupported = true;
boolean const wAxisSupported = true;

// System config (if Tach is not connected change in the corresponding constant value from "true" to "false")
boolean const tachSupported = true;
 
// I/O ports config (change pin numbers if DRO, Tach sensor or Tach LED feedback is connected to different ports)
int const clockPin = 2;

int const xDataPin = 3;
int const yDataPin = 4;
int const zDataPin = 5;
int const wDataPin = 6;

int const tachPin = 7;
int const tachLedFeedbackPin = 13;

// Tach RPM config
long const minRpmDelay = 1200;			// 1.2 sec calculates to low range = 50 rpm.

//---END OF CONFIGURATION PARAMETERS ---




boolean const droSupported = (xAxisSupported || yAxisSupported || zAxisSupported || wAxisSupported);
long const minRpmTime = (minRpmDelay * 1000);

//variables that will store tach info and status
volatile int lastTachPinStatus;
volatile long tachTimerStart;
volatile long tachTimerStop;
volatile long tachRotationCount;

//variables that will store the readout output
volatile long tachReadoutRotationCount;
volatile long tachReadoutMicrosec;
volatile boolean tachReadoutSendData;


//variables that will store the readout
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

	if (tachSupported) {
		pinMode(tachPin, INPUT);
		pinMode(tachLedFeedbackPin, OUTPUT);
	}


	//initialize serial port
	Serial.begin(9600);

	//initialize tach values
	if (tachSupported) {
		lastTachPinStatus = digitalRead(tachPin);
		tachTimerStop = micros();
		tachTimerStart = tachTimerStop;
		tachRotationCount = 0;
		tachReadoutRotationCount = 0;
		tachReadoutMicrosec = 0;
		tachReadoutSendData = false;
	}

}


// The loop function is called in an endless loop
void loop()
{

//readTach() is called so often to provide the greatest accuracy
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

			if (tachSupported)
				readTach();
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

		if (tachSupported)
			readTach();
	
		//print DRO positions to the serial port
		if (xAxisSupported){
			Serial.print("X");
			if (tachSupported)
				readTach();
			Serial.print((long)xCoord);
			if (tachSupported)
				readTach();
			Serial.print(";");
			if (tachSupported)
				readTach();
		}
		if (yAxisSupported){
			Serial.print("Y");
			if (tachSupported)
				readTach();
			Serial.print((long)yCoord);
			if (tachSupported)
				readTach();
			Serial.print(";");
			if (tachSupported)
				readTach();
		}
		if (zAxisSupported){
			Serial.print("Z");
			if (tachSupported)
				readTach();
			Serial.print((long)zCoord);
			if (tachSupported)
				readTach();
			Serial.print(";");
			if (tachSupported)
				readTach();
		}
		if (wAxisSupported){
			Serial.print("W");
			if (tachSupported)
				readTach();
			Serial.print((long)wCoord);
			if (tachSupported)
				readTach();
			Serial.print(";");
			if (tachSupported)
				readTach();
		}
	}

	// print Tach rpm to serial port
	if (tachSupported) {
		// Format tach average
		formatTachOutput();
		readTach();

	// output tach data
		if (tachReadoutSendData) {
			Serial.print("T");
			readTach();
			Serial.print((long)tachReadoutMicrosec);
			readTach();
			Serial.print("/");
			readTach();
			Serial.print((long)tachReadoutRotationCount);
			readTach();
			Serial.print(";");
			tachReadoutSendData = false;
		}

	//give the scales time to become ready again but continue reading tach input;
		readTachLoop(50);
	} else {
		delay(50);
	}
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


// Check tach port and record time if port input is changed
inline void readTach()
{
	if (tachSupported) {

		// read tach port and output it to LED
		int tachPinStat = digitalRead(tachPin);
		long tachTimer = micros();
		digitalWrite(tachLedFeedbackPin, tachPinStat);

		// record timestamp of change in port input
		if (lastTachPinStatus != tachPinStat) {
			lastTachPinStatus = tachPinStat;
			// count pulses till full rotation
			if (tachPinStat == LOW) {
				tachTimerStop = tachTimer;
				tachRotationCount++;
				// reset timer if clock overlapses
				if (tachTimerStop < tachTimerStart) {
					tachTimerStart = tachTimerStop;
					tachRotationCount = 0;
				}
			}
		}
	}
}


// Loop specified time (in milliseconds) just checking tach port
inline void readTachLoop(unsigned long delay)
{
	if (tachSupported) {
		unsigned long timerStart = millis();
		do {
			readTach();
			// reset timer if clock overlapses
			if (timerStart > millis())
				timerStart = millis();
		} while (timerStart + delay > millis());
	}
}


// Calculate the tach rpm 
inline void formatTachOutput()
{
	if (tachSupported) {
		long microSeconds;

		// We have at least one tick on rpm sensor so calculate the average time between ticks
		if (tachRotationCount != 0) {
			tachReadoutRotationCount = tachRotationCount;
			tachReadoutMicrosec = tachTimerStop - tachTimerStart;
			tachReadoutSendData = true;
			
			tachTimerStart = tachTimerStop;
			tachRotationCount = 0;

		// if no ticks on rpm sensor for long time set rpm to zero
		} else {
			microSeconds = micros() - tachTimerStart;
			if (microSeconds > minRpmTime ) {
				tachReadoutRotationCount = 0;
				tachReadoutMicrosec = 0;
				tachReadoutSendData = true;

				tachRotationCount = 0;

			// reset timer if clock overlapses
			} else if (microSeconds < 0) {
				tachTimerStart = 0;
			}
		}
	}
}
