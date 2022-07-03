/* Mega Drive switchless mod for Padauk ICs, to be compiled with SDCC.
 * 
 * made by screwbreaker
 * rev 1.0
 *
 * SYNOPSIS:
 * Yes, another switchless mod for Mega Drive
 * this version is very simple, it only allow to switch between the different regions, JP, USA, EU
 * there is no way to change only the region or the video mode
 *
 * HOW TO CHANGE REGION:
 * hold down the reset button
 * after 2 seconds the region led start to blink fast
 * the region shown change every second
 * to confirm the region release the reset button
 * the region led blink three times to confirm the selection
 *
 * NORMAL RESET:
 * press the reset button for less than 2 seconds
 *
 * MEGA CD MODE:
 * this mod expose two pins to control the switchless mod for the MCD
 * the pins output are inverted respect the language and video pins, this is for compatibility with other mods
 * is possible to configure the mod to reset the console automatically after a region switch
 * to enable the auto reset connect the MCD pin to ground
 *
 * when the MCD mode is enabled the console is set in reset before set the language and video pins
 * this is made to avoid unwanted access to the MCD bios meanwhile the eeprom is set to a different memory map
 * however the microcontroller can't keep the Mega Drive in reset by holding the reset signal
 * the reset signal is generated internally every time the button is pressed, the reset time is fixed and can't be extended
 * there are other reset singals inside the mega drive, this is used by games or expansions to control the reset signal
 * this mod expose a second reset pin, to allow to control other reset signal inside the console
 * this pin is an open drain, is intended to be put in parallel to the other devices
 * this feature is for future use or test only, do not use it at the moment
 *
 * LED DRIVE TYPE:
 * is possible to choose either a common anode or a common cathode LED
 * one pin is used to choose the led type, the pin is tied to VCC with the internal pull-up
 * if not connected the default choose is for a commmon anode led
 *
 * check the datasheet for the microcontroller to check the maximum current to drive the LEDS
 * LEDs generally needs a very small current, expecially if used as indicator like in this case
 * my suggestion is to drive them with the smallest current as possible, no more than 5ma
 */

/* includes */
#include "pdk.h"
#include "utils.h"

/* global defines */
#define DEFAULT_REGION EU	// This is the default region used on the first boot

// pin settings
#define RED				PA,7	// red LED
#define GREEN			PA,6	// green LED
#define BLUE			PA,5	// blue LED

#define VIDEO			PB,1	// video (50/60Hz)
#define LANGUAGE	PB,2	// language (ENG/JAP)
#define _VIDEO		PA,3	// inverted video (For Mega CD)
#define _LANGUAGE	PA,4	// inverted language (For Mega CD)
#define RESET			PA,0	// reset output
#define BUTTON		PB,0	// reset button in
#define MCD				PB,6	// if low enable the Mega CD mode, reset the console after a region change
#define LED_TYPE	PB,5	// HIGH = common anode, LOW = common cathode

/* do not modify anything under this line */

#define ledOn(led)		bools.led_type ? (clearBit(led)) : (setBit(led))
#define ledOff(led)		bools.led_type ? (setBit(led)) : (clearBit(led))

#define isBtnPressed	isPinLow
#define reset()				bools.reset = true; \
											delay = RESET_DELAY
#define next_region()	region = ((region + 1) % REGIONS)

#define RESET_DELAY			33	// about 270ms
#define DEBOUNCE_DELAY	5		// about 40ms
#define CHANGE_TIME			200	// about 1.6 sec with 8ms tick
#define LED_FAST_DELAY	16	// about 130ms with 8ms tick
#define LED_SLOW_DELAY	33	// about 270ms with 8ms tick
#define SHOW_TURNS			8		// about 1s
#define CONFIRM_TURNS		5		// 3 slow blinks

#define ALL_OFF REGIONS

/* global typedefs and structures */
/* global typedefs and structures */
typedef union {
	struct {
		unsigned tick						: 1;
		unsigned btn_event			: 1;
		unsigned show_regions		: 1;
		unsigned confirm_region	: 1;
		unsigned reset					: 1;
		unsigned led_state			: 1;
		unsigned mcd						: 1;
		unsigned led_type				: 1;
	};

	struct {
		unsigned BOOLS					: 8;
	};
} __Bools_t;

typedef enum {
	JP = 0x00,
	EU,
	US,
	REGIONS
} region_t;

/* functions prototypes */
void set_led(region_t region);
void set_region(void);
void set_mcd(void);

/* global variables */
volatile region_t region = DEFAULT_REGION;
volatile __Bools_t bools;
volatile uint8_t turns = 0;
volatile uint8_t delay = 0;
volatile uint8_t debounce_delay = 0;
volatile uint32_t btn_counter = 0;

// interrupt handler (kept as simple/minimalistic as possible)
void interrupt(void) __interrupt(0) {
	// monitor the MD reset button
	if (INTRQ & INTRQ_PB0) {
		if (!debounce_delay) {
			debounce_delay = DEBOUNCE_DELAY;
			bools.btn_event = true;
			INTRQ &= ~INTRQ_PB0;
		}
	}

	// when T16 'ticks'
	if (INTRQ & INTRQ_T16) {
		bools.tick = true;
		INTRQ &= ~INTRQ_T16;
	}
}

unsigned char _sdcc_external_startup(void) {
	// setup 8MHz sysclock and let easypdk programmer do the calibration (after writing)
	EASY_PDK_INIT_SYSCLOCK_8MHZ();					//use 8MHz sysclock
	EASY_PDK_CALIBRATE_IHRC(8000000,5000);	//tune SYSCLK to 8MHz @ 5.000V
	EASY_PDK_CALIBRATE_BG();
	EASY_PDK_FUSE(FUSE_BOOTUP_FAST | FUSE_SECURITY_OFF);
	return 0;
}

// timers and pin setup
void setup() {
	setPinOutput(RED);
	setPinOutput(GREEN);
	setPinOutput(BLUE);

	setPinOutput(VIDEO);
	setPinOutput(LANGUAGE);
	setPinOutput(_VIDEO);
	setPinOutput(_LANGUAGE);

	setPinOutput(RESET);
	setPin(RESET);

	enableDigitalInput(LED_TYPE);
	setPinPullup(LED_TYPE);

	enableDigitalInput(MCD);
	setPinPullup(MCD);

	enableDigitalInput(BUTTON);
	setPinPullup(BUTTON);

	// enable T16 timer to generate an interrupt about every 8 milliseconds
	T16M = T16_CLK_IHRC | T16_CLK_DIV64 | T16_INTSRC_10BIT;	// Use IHRC clock source (16 MHz), /64, Interrupt on (rising) Bit 10
	INTEN = INTEN_T16| INTEN_PB0;
	INTEGS = INTEGS_T16_RISING | INTEGS_PB0_BOTH;

	// enable interrupts
	INTRQ = 0;
	__engint();	// enable global interrupts
} // setup()

void main(void) {
	// Setup pin and timers
	setup();

	// read the Mega CD mode pin
	bools.mcd = isPinLow(MCD);

	// read the type of led used
	bools.led_type = isPinHigh(LED_TYPE);

	// set default region
	set_region();
	set_led(DEFAULT_REGION);

	//reset(); // reset the MD to be sure region pins are read correctly // uncomment if there is trouble at boot

	while(true) {
		// handle the timers
		if (bools.tick) {
			bools.tick = false;

			if (bools.btn_event) {
				btn_counter++;
			}

			if (debounce_delay) {
				debounce_delay--;
			}

			if (delay) {
				delay--;
			}
		}

		// the RESET button is pressed / released
		if (bools.btn_event) {
			if (!debounce_delay) {
				if (isBtnPressed(BUTTON)) {
					if (btn_counter > CHANGE_TIME) {
						bools.show_regions = true;
					}
				} else {
					bools.btn_event = false;

					if (btn_counter > CHANGE_TIME) {
						turns = 0;
						bools.show_regions = false;
						bools.confirm_region = true;
					} else {
						reset();
					}

					btn_counter = 0;
				}
			}
		}

		// each led blink fast for a while
		if (bools.show_regions) {
			if (delay == 0) {
				if (bools.led_state) {
					set_led(region);
				} else {
					set_led(ALL_OFF);
				}

				if (turns > SHOW_TURNS) {
					next_region();
					turns = 0;
				} else {
					turns++;
				}

				bools.led_state = !bools.led_state;
				delay = LED_FAST_DELAY;
			}
		}

		// the LED of the choosed region make few slow blinks, to confirm the selection
		if (bools.confirm_region) {
			if (delay == 0) {
				if (bools.led_state) {
					set_led(region);
				} else {
					set_led(ALL_OFF);
				}

				if (turns > CONFIRM_TURNS) {
					turns = 0;
					delay = 0;
					bools.confirm_region = false;
					set_led(region);
					set_region();
				} else {
					turns++;
				}

				bools.led_state = !bools.led_state;
				delay = LED_SLOW_DELAY;
			}
		}

		// keep the reset singal low for a while, then release it
		if (bools.reset) {
			if (delay) {
				clearPin(RESET);
			} else {
				setPin(RESET);
				bools.reset = false;
			}
		}
	}
}

// set region
void set_region(void) {
	switch(region) {
		case JP:
			clearPin(LANGUAGE);
			setPin(VIDEO);
			break;
		case EU:
			setPin(LANGUAGE);
			clearPin(VIDEO);
			break;
		case US:
			setPin(LANGUAGE);
			setPin(VIDEO);
			break;
		default:
			break;
	}

	set_mcd();
} // set_region()

// set MCD bios
void set_mcd(void) {
	if (bools.mcd) {
		clearPin(RESET);
		reset();
	}

	switch(region) {
		case JP:
			setPin(_LANGUAGE);
			clearPin(_VIDEO);
			break;
		case EU:
			clearPin(_LANGUAGE);
			setPin(_VIDEO);
			break;
		case US:
			clearPin(_LANGUAGE);
			clearPin(_VIDEO);
			break;
		default:
			break;
	}
} // set_mcd()

// set LED
void set_led(region_t region) {
	switch(region) {
		case JP:
			ledOff(GREEN);
			ledOff(BLUE);
			ledOn(RED);
			break;
		case EU:
			ledOff(RED);
			ledOff(BLUE);
			ledOn(GREEN);
			break;
		case US:
			ledOff(GREEN);
			ledOff(RED);
			ledOn(BLUE);
			break;
		default:
			ledOff(RED);
			ledOff(GREEN);
			ledOff(BLUE);
			break;
	}
} // set_led()

