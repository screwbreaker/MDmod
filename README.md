# MDmod
Yes, another Mega Drive switchless mod

![Board](https://raw.githubusercontent.com/screwbreaker/MDmod/main/pictures/PCBs.png?token=GHSAT0AAAAAABWHLXQKYUN6ILYQPUOUOWLEYWBTTYQ)

### Summary
This is another version of the classic switchless MOD for Mega Drive/Genesis.
There are two version of this MOD.
One for Padauk microcontrollers. And one for the old PIC16F630.
For each micro there is a little PCB for help in the installation.

### History
Everything started because I got some Padauk microcontrollers, and I wanted to play with them.
I choose to implement the Mega Drive switchless mod, because it seem very appropriate as a basic program for a microcontroller.
Later, since SDCC also handle PIC microcontrollers, I thought it was nice to port the code for the old PIC16F630.
And then I made also some little PCB, to make the installation simpler.
That's it, that's why now there is another, or better, onother two switchless mod for Mega Drive.

### How it work
This version is very simple, it only allow to switch between the different regions, JP, USA, EU
There is no way to change only the region or the video mode.

## How to change region
hold down the reset button
after 2 seconds the region led start to blink fast
the region shown change every second
to confirm the region release the reset button
the region led blink three times to confirm the selection

## Normal reset
press the reset button for less than 2 seconds

### BOM
Only few components, the micro, a decoupling capacitor, and a resistor for the LEDs.
- C: 100 nF 0805 capacitor
- R: 0805 resistor

### Warning
The PCB are not the same!
The two microcontrolers have the same package but a different pinout.
So, each micro have his own board, don't mix them!

Do not close both CC and CA jumpers!

### LED
For the LED resistor I suggest a 10K resistor, but the value can be choose according to the particular LED used.
Just beware to do not exceed the maximum current for the microcontroller pin.
For more information, read the datasheet of the microcontroller.

The mode is writed for an RGB LED.
Other mods use two color led, this is not the case. You must use an RGB led.
Or, another way is to keep the original red led, and place antoher led near it.

The mod allow to use both common anode, or common cathode LEDs.
There is a little jumper on the board that allow to choose the LED type, the jumpers are marked as CA (common anode), CC (common cathode).
Do not close both jumpers!!!

The COM pad is the common, wich can be either VCC or GND, depens on which jumper is set.

### Mega CD mode
This mod expose two pins to control the switchless mod for the MCD.
The pins output are inverted respect the language and video pins, this is for compatibility with other mods.
Is possible to configure the mod to reset the console automatically after a region switch.
To enable the auto reset close the MCD jumper.

When the MCD mode is enabled the console is set in reset before set the language and video pins.
This is made to avoid unwanted access to the MCD bios meanwhile the eeprom is set to a different memory map.
However the microcontroller can't keep the Mega Drive in reset by holding the reset signal.
The reset signal is generated internally every time the button is pressed, the reset time is fixed and can't be extended.

The MCD mode can be also used to make enable the auto console reset after the region change.

### Reset issue
Sometime during a region change, an unwanted reset can happen.
The mod use a debounce to avoid this. But this is also dependant from the lenght of the wire from the reset button.
If you have such isse, increase the debounce delay in the firmware. Or, put a 100nF capacitor between the reset singal and GND (as close as possible to the micro).

### Padauk VS PIC
No differences, but the PIC also have an internal EPROM, so it can save the last mode used.
The padauk can't. The micro always start with the default region. The default region must be set in the firmware.

### Installation
Exactly like any other switchless MOD.
There are plenty guide online, they are well made, and cover all the Mega Drive/Genesis variants.
So, I'm not going to rewrite a guide.

Bot the boards are singe side, so the PCB back can be put in cotact with the Mega Drive board without issue.

### License
The PCBs are under the CERN OHL license.
The firmwares, under the GPL3.

This is an OPEN project.
So, like any other project like this, the hope is that others can improve it an reshare with the community.
Don't go out this basic guideline.

### Disclaimer
The hardware and the software are provided to you 'as is' and without any express or implied warranties whatsoever with respect to its functionality, operability or use, including, without limitation, any implied warranties of merchantability, fitness for a particular purpose or infringement. We expressly disclaim any liability whatsoever for any direct, indirect, consequential, incidental or special damages, including, without limitation, lost revenues, lost profits, losses resulting from business interruption or loss of data, regardless of the form of action or legal theory under which the liability may be asserted, even if advised of the possibility or likelihood of such damages.

### Thanks
- Thanks to the guys of the [Free PDK](https://free-pdk.github.io/) project. They are really smart and awesome!

