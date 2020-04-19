# Arduino_DRO

Firmware for Arduino DRO with up to 4 Axis display, Tachometer, Touch Probe, and Bluetooth output suitable for [Touch DRO](https://www.yuriystoys.com).

This repository is heavily based on the v6.0 code from [Rysiu M](http://rysium.com/projects/197-arduino-dro-q) for connecting cheap digital scales (typically found on machine tools) to an Arduino and sending those data via Bluetooth or serial to a display like [TouchDRO](http://www.yuriystoys.com/p/android-dro.html). The display and interaction portions are influenced by the [People's DRO](https://www.chipmaker.ru/topic) on [chipmaker.ru](https://www.chipmaker.ru) (or at least that's what Google Translate told me the name was).

## Why Another Android DRO Project

I just couldn't find the one I was looking for. RysiuM's is great for working with Touch DRO, but does not have a local non-connected to an Android tablet display option. Oleg A.'s People's DRO has local display, but no ability to connect to Touch DRO. I wanted both. 

## User Guide

**NOTE**: This part is not working yet, it's a design specification.

Display

| A/I | x | x | x | x | x | x | x |
| A/I | x | x | x | x | x | x | x | 
| A/I | x | x | x | x | x | x | x | 
| x   | x | x | x | x | x | x | x |


| Button    | Function              | Notes |
| ---       | ---                   | ---   |
| X0        | Zero the X axis       |       |
| Y0        | Zero the Y axis       |       |
| Z0        | Zero the Z axis       |       |
| X         | Change abs/inc mode for X axis    | display changes "A" / "I" |
| Y         | Change abs/inc mode for Y axis    | display changes "A" / "I" |
| Z         | Change abs/inc mode for Z axis    | display changes "A" / "I" |
| ---       | ---                               | ---
| X0 long   | Enter offset entry for X axis     | display will blink, use dial |
| Y0 long   | Enter offset entry for Y axis     | display will blink, use dial |
| Z0 long   | Enter offset entry for Z axis     | display will blink, use dial |
| X long    |                                   |
| Y long    |                                   |
| Z long    |                                   |
| ---       | ---                               |
| Rotary press  | Enter menu / Select               |
| Rotary dial   | Change selection / digit / value  |
| ---           |
| 1/2           | divide axis by 2              | follow with X/Y/Z/W within 15 seconds |
| abs/inc       | toggle all abs/inc mode       | applies to all axes  |
| in/mm         | toggle in/mm units            | applies to all axes  |


## Ideas From

- use rotary encoder to change numbers
- Use last decimal point to show 'mm' no decimal point means inches
- Use uppermost digit to show 'A'bs or 'I'ncremental
- max 24" and 609mm on 24" scale -- what is max on smaller
- auto-adapt to significant digits? -- auto adapt to larger numbers change .
    - scale is only accurate to 0.001" and 0.01mm
    - `_-99.9999"` and `_-999.999mm` is overkill by 1

https://www.chipmaker.ru/topic/143676/?page=31
https://www.chipmaker.ru/topic/143676/?page=13


    - left button jumps by digits,
    - up / down buttons respectively increases / decreases the modulus of the number (active sign), from 0 to 9

    - on a sign discharge:

        - up button - makes the number positive and automatically exits the correction mode, if it was positive the sign of the number does not change ,

        - down button - accordingly makes the number negative and exit the correction mode, if it was negative the sign of the number does not change ,

    - At any time, you can exit the correction mode by pressing the first button of the axis to be corrected.

    x1  x2
    y1  y2 
    z1  z2
        +
    <   -

    - Button "X1", "Y1":
        first press - activate the axis setting (axis blinks)
        repeated pressing (within 15 seconds) reset the axis to 0 and exit to operating mode,
        inactivity 15 sec. - exit to operating mode 

    - Button "<" - each press during active axis setting - transition by digits (active digit blinks)
        inactivity 15 sec. - exit to operating mode

    - Button "+", "-" - increases / decreases the value of the active digit
        after passing all the digits (the dot and the “-” sign flash if the number is negative) - use the “+” , “-” buttons to set the positive / negative sign of the dialed / corrected value, and automatically exit to working mode.

        In the operating mode, you can also exit at any level - by pressing 1 button of the active axis.

        Inaction 15sec - exit to operating mode.

    - Button " X2", "Y2":
        - switches the axis between the ABS / INC measurement,
        - displayed in the leftmost digit as "A" , "b" (two independent measurements on each axis)


    - Button "Z1":
        not involved

    - Button "Z2" :
        - switches the display of readings revolutions / cutting speed, "A" - revolutions / minute, "b" - meters / minute
        - cutting speed is always tied to the readings of the "Y" axis .


* The modes "Radius", "Diameter" are displayed by the symbols "r", "d".

* The modes "Tachometer", "Cutting speed" are displayed by the symbols "t", "S".

