# Arduino_DRO
Firmware for Arduino DRO with 4 Axis, Tachometer, Touch Probe (http://www.rysium.com/rysium.docs/)

This repository is a mirror of the latest code from [Rysiu M](http://rysium.com/projects/196-arduino-dro) for connecting cheap digital scales (typically found on machine tools) to an Arduino and then sending those data via Bluetooth or serial to a display like [TouchDRO](http://www.yuriystoys.com/p/android-dro.html).

---
# Arduino DRO V5_11 (with Touch probe support, Hall tach support and "smart" rpm output, smooth DRO readings)

The new version of Arduino sketch (Arduino DRO V5.11) is partially based on Yuriy's ArduinoDRO_V3_3 but has been enhanced by precision tach readout and flexible configuration. This code is optimized (it is smaller and faster over the version 4_1) and uses direct access to Arduino ports instead slower "high level" functions. The code uses new logic to time and read iGaging scales correctly - the logic given to me by Les Jones with additional fix to the problem with "jumping" readings on axis X. DRO now has an option to "smooth" readings from scales reducing "flickering" and "jumping". The logic uses weighted average with automatic smoothing factor.

Thanks to using interrupts on tach pin the rpm data is more accurate and supports much higher pulse rate (higher rpm or multiple pulses per rotation). This version can read rpm data from 50 rpm to over 6 millions rpm with error less then 1%. The output can be rounded to the nearest 1% or using "smart rounding" (rounding to to the nearest "round" number by 1, 2, 5, 10, 20, 50, 100). The tach output is compatible with Yuriy's original Touch DRO including the latest version 2.0.2 Build 11/22/2014. This version of sketch supports also Arduino boards with clock frequency other than 16MHz (for example Arduino Pro Mini 3.3V).

This version is compatible with Touch DRO 2.5.x (the newest version Yuriy's Touch DRO supporting touch probe output). Note, touch probe is not supported by Touch DRO 2.0.x.

Arduino DRO is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/.

Download the source of [Arduino DRO V5_11 (with Touch probe support, Hall tach support and "smart" rpm output and smooth DRO readings).](http://www.rysium.com/rysium.docs/ArduinoDRO_Tach_V5_11.ino) 

New/changed functionality over the Arduino DRO V5_10:

* Add "touch probe" support for Touch DRO 2.5.x. 

Note: This is recommended version to work with Yuriy's Tuch DRO

Configuration parameters:

__SCALE_<n>_ENABLED__
  
    Defines if DRO functionality on axis <n> should be supported. 
    If supported DRO scale should be connected to I/O pin defined in constant "<n>DataPin" and 
    DRO data is sent to serial port with corresponding axis prefix (X, Y, Z or W)
    Clock pin is common for all scales should be connected to I/O pin defined in constant "clockPin" 
  
    Possible values:
        1 = DRO functionality on axis <n> is supported
        0 = DRO functionality on axis <n> is not supported

    Default value = 1

__SCALE_CLK_PIN__

    Defines the I/O pin where clock signal for all DRO scales is connected
  
    Possible values:
        integer number between 2 and 13

    Default value = 2

__SCALE_<n>_PIN__
  
    Defines the I/O pin where DRO data signal for selected scale is connected
  
    Possible values:
        integer number between 2 and 13

    Default values = 3, 4, 5, 6 (for corresponding axis X, Y, Z and W)

__SCALE_<n>_AVERAGE_ENABLED__ (Introduced in 5.7)

    Defines if DRO reading should be averaged using weighted average calculation with automating smoothing factor.
    If average is enabled the reading is much more stable without "jumping" and "flickering" when the scale "can't decide" on the value.

    Note: This value is not used when corresponding SCALE_<n>_ENABLED is 0
  
    Possible values:
        0 = exact measured from the scale on axis <n> is sent
        1 = scale reading from axis <n> is averaged using weighted average calculation with automatic smoothing factor

    Default value = 1

__AXIS_AVERAGE_COUNT__ (Introduced in 5.8, Changed in 5.9)

    Defines the number of last DRO readings that will be used to calculate weighted average for DRO.
    For machines with power feed on any axis change this value to lower number i.e. 8 or 16.
  
    Possible values:
        integer number between 4 and 32
    
    Recommended values:
        16 for machines with power feed
        32 for all manual machines
    
    Default value = 24

__TACH_ENABLED__ (Introduced in 2.b)

    Defines if tach sensor functionality should be supported. 
    If supported tach sensor should be connected to I/O pin defined in constant "tachPin" and 
    rpm value is sent to serial port with axis prefix "T" 
  
    Possible values:
        1 = tach sensor functionality is supported
        0 = tach sensor functionality is not supported
    
    Default value = 1

__TACH_PRESCALE__ (Introduced in 5.2)

    Defines how many tach pulses per one revolution the sensor sends.
    For example if tach sensor uses two magnets on the shaft the sensor will generate two pulses per revolution.
    This can be used to get better resolution and faster response time for very low rpm
    Note: This value is not used when TACH_RAW_DATA_FORMAT is enabled 
    
    Possible values:
        any integer number greater than 0
      
    Default value = 1

__TACH_AVERAGE_COUNT__ (Introduced in 5.3)

    Defines the number of last tach readings that will be used to calculate average tach rpm.
    If you want to send measured rpm instead of average rpm set this value to 1.
    Note: This value is not used when TACH_RAW_DATA_FORMAT is enabled.
        It is recommended to set this value 2 times or more of TACH_PRESCALE value.
        For example: if TACH_PRESCALE = 4, set TACH_AVERAGE_COUNT = 8
        
    Possible values:
        1 = exact measured tach reading is sent
        any integer number greater than 1 - average tach reading is sent
      
    Default value = 6

__TACH_ROUND__ (Introduced in 5.3 and 5.10)
    Defines how tach reading should be rounded.
    If rounding is enabled the reading can be rounded either by 1% of current rpm or to the fixed "round" number with predefined RPM thresholds ("smart rounding").
    For example with 1% rounding if measured rpm is between 980rpm and 1020 rpm the display will show numbers rounded to 9 and 10 (i.e. 981, 990, 999, 1000, 1010, 1020 etc.).
    With "smart rounding" the measured rpm is rounded to the nearest 1, 2, 5, 10, 20, 50 and 100 depends on measured RPM (change at predefined thresholds).
    For example with "smart rounding" all measured rpm is between 500pm and 2000 rpm the display will show numbers rounded to the nearest 5 (i.e. 980, 985, 990, 995, 1000, 1005 etc.).
    Note: This value is not used when TACH_RAW_DATA_FORMAT is enabled

    Possible values:
        0 = exact measured tach reading is sent
        1 = tach reading is rounded to the nearest 1% of measured rpm (1% rounding)
        2 = tach reading is rounded to the nearest "round" number with fixed thresholds ("smart rounding")

    Default value = 2

__TACH_RAW_DATA_FORMAT__ (Introduced in 3)

    Defines the format of tach data sent to serial port.
    Note: when rad data format is used, then TACH_PRESCALE is ignored

    Possible values:
        1 = tach data is sent in raw (two values) format: T<total_time>/<number_of_pulses>
        0 = tach data is sent in single value format: T<rpm>

    Default value = 0

__MIN_RPM_DELAY__ (Introduced in 2.b)

    Defines the delay (in milliseconds) in showing 0 when rotation stops. If rpm is so low and time between tach pulse
    changes longer than this value, value zero rpm ("T0;") will be sent to the serial port.
    Note: this number will determine the slowest rpm that can be measured. In order to measure smaller rpm I suggest
    to use a sensor with more than one "ticks per revolution" (for example hall sensor with two or more magnets).
    The number of "ticks per revolution" should be set in tachometer setting in Android app.

    Possible values:
        any integer number greater than 0
  
    Default value = 1200 (the minimum rpm measured will be 50 rpm)

__INPUT_TACH_PIN__ (Introduced in 2.b)

    Defines the I/O pin where tach sensor signal is connected

    Possible values:
        integer number between 2 and 13

    Default value = 7

__OUTPUT_TACH_LED_PIN__ (Introduced in 2.b, Renamed in 5.11)

    Defines the I/O pin where the tach LED feedback is connected. 
    Tach LED feedback indicates the status of tachPin for debugging purposes

    Possible values:
        integer number between 2 and 13

    Default value = 13 (on-board LED)

__UART_BAUD_RATE__ (Introduced in 5.2)

    Defines the serial port baud rate. Make sure it matches the Bluetooth module's baud rate.

    Recommended value:
        1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200

    Default value = 9600

__UPDATE_FREQUENCY__ (Introduced in 5.2)

    Defines the Frequency in Hz (number of timer per second the scales are read and the data is sent to the application).

    Possible values:
        any integer number between 1 and 64 
  
    Default value = 24

__TACH_UPDATE_FREQUENCY__ (Introduced in 5.3, Changed in 5.9)

    Defines the max Frequency in Hz (number of timer per second) the tach output is sent to the application.
    Note: This value must be a divider of UPDATE_FREQUENCY that would result zero reminder.
    For example for UPDATE_FREQUENCY = 24 valid TACH_UPDATE_FREQUENCY are: 1, 2, 3, 4, 6, 8, 12 and 24 

    Possible values:
        any integer number between 1 and UPDATE_FREQUENCY

    Default value = 4

__PROBE_ENABLED__ (Introduced in 5.11)
    
    Defines if touch probe sensor functionality should be supported.
    If supported touch probe should be connected to I/O pin defined in constant "probePin".

    Possible values:
        1 = touch probe functionality is supported
        0 = touch probe functionality is not supported
    
    Default value = 1

__INPUT_PROBE_PIN__ (Introduced in 5.11)

    Defines the I/O pin where touch probe signal is connected.

    Possible values:
        integer number between 2 and 13

    Default value = 8

__PROBE_INVERT__ (Introduced in 5.11)

    Defines if the touch probe input pin signal needs to be inverted (enter the signal level when touch probe is not touching).

    Possible values:
        0 = touch probe input pin signal is LOW (logical Zero) when touch probe is in "normal open" status (not touching)
        1 = touch probe input pin signal is HIGH (logical One) when touch probe is in "normal open" status (not touching)
    
    Default value = 0

__OUTPUT_PROBE_LED_PIN__ (Introduced in 5.11)
  
    Defines the I/O pin where the touch probe LED feedback is connected.
    Touch probe LED feedback indicates the status of Touch probe for debugging purposes.
    Note: If Tach Led is not used you may chnage this value to 13 in order to use on-board LED for Touch probe indication.

    Possible values:
        integer number between 2 and 13
  
    Default value = 12

I recommend using the latest Arduino DRO V5 over the older versions 

__NOTE__: I recommend using the latest version of ArduinoDRO_Tach sketch and the latest Yuriy's version of Touch DRO - they work well together with all the functionality supported. For your reference the old versions of Arduino sketch are still available for download below. The following downloads and information is for your reference only and should be used with caution.

* Arduino DRO V3 (with Hall tach support) 
* Arduino DRO V4 (with Hall tach support) 
* Arduino DRO V4.1 (with Hall tach support and rpm output)
* Arduino DRO V5 (!!! Do not use as it has a bug in scale reading. Use Arduino DRO V5_2 instead)
* Arduino DRO V5_1 (!!! Do not use as it still has a bug in scale reading. Use Arduino DRO V5_2 instead)
* Arduino DRO V5_2 (This is working. The bug in scale reading is fixed (number of clock pulses is 21 and frequency is 9000 Hz)
* Arduino DRO V5_3 (Added option to average and round tach output, added option to select max frequency of tach output)
* Arduino DRO V5_4 (Replace Yuriy's method of clocking scales with method written by Les Jones. The code should be more compatible with iGaging scales reducing "jumping")
* Arduino DRO V5_5 (Optimizing the scale reading logic using method written by Les Jones. The code spends less time inside the interrupt routine resulting better accuracy of the tach reading)
* Arduino DRO V5_6 (Add 4.75us delay between changing the clock signal and reading the first axis. This fixes a "jumping" on axis X.)
* Arduino DRO V5_7 (!!! Do not use as it has a bug in reading scale X. Use Arduino DRO V5_8 instead!!! Add option to "smooth" readings from scales reducing "flickering" and "jumping". The logic uses weighted average with automatic smoothing factor.)
* Arduino DRO V5_8 (Correction to calculate average for scale X. Weighted average sample size is parameter now (default value is 32))
* Arduino DRO V5_9 (Reduce flickering on RPM display. Remove long delay in RPM displaying Zero after the rotation stops.)
* Arduino DRO V5_10 (Add "smart rounding" on tach display. Fix 1% tach rounding. Support processors running at 8MHz clock)
 
