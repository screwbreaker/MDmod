/* Mega Drive switchless mod for PIC microcontroller, to be compiled with SDCC.
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
#include <pic14regs.h>
#include "utils.h"

/* global defines */
#define DEFAULT_REGION EU	// This is the default region used on the first boot // not used on PIC

// pin settings
#define RED		C5	// red LED
#define GREEN		C4	// green LED
#define BLUE		C3	// blue LED

#define VIDEO		A1	// video (50/60Hz)
#define LANGUAGE	A0	// language (ENG/JAP)
#define INV_VID		C2	// inverted video (For Mega CD)
#define INV_LANG	C1	// inverted language (For Mega CD)
#define RESET		C0	// reset output
#define BUTTON		A2	// reset button in -- an interrupt capable pin must be used
#define MCD		A4	// if low, enable the Mega CD mode. reset the console after a region change
#define LED_TYPE	A5	// HIGH = common anode, LOW = common cathode

/* do not modify anything under this line */

#define RESET_DELAY	16	// about 260ms
#define DEBOUNCE_DELAY	3	// about 50ms
#define CHANGE_TIME	152	// about 2.5 sec with 16ms tick
#define LED_FAST_DELAY	8	// about 130ms with 16ms tick
#define LED_SLOW_DELAY	16	// about 260ms with 16ms tick
#define SHOW_TURNS	8	// about 2s
#define CONFIRM_TURNS	5	// 3 slow blinks

#define ledOn(led)	bools.led_type ? (clearPin(led)) : (setPin(led))
#define ledOff(led)	bools.led_type ? (setPin(led)) : (clearPin(led))

#define isBtnPressed	isPinLow
#define reset()		bools.reset = true; \
			delay = RESET_DELAY
#define next_region()	region = ((region + 1) % REGIONS)

// start the timer counter from 0xBFFF to get a 16ms tick
#define TMR1_reset()	TMR1H = 0xBF; \
			TMR1L = 0xFF

#define COMPARATOR_OFF()	CM2 = 1; \
				CM1 = 1; \
				CM0 = 1

#define ALL_OFF REGIONS

// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN),
// disable watchdog,
// and disable low voltage programming.
// The rest of fuses are left as default.
SetFuses(_INTRC_OSC_NOCLKOUT & _WDTE_OFF & _PWRTE_OFF & _MCLRE_OFF);

/* global typedefs and structures */
typedef union {
	struct {
		unsigned tick		: 1;
		unsigned btn_event	: 1;
		unsigned show_regions	: 1;
		unsigned confirm_region	: 1;
		unsigned reset		: 1;
		unsigned led_state	: 1;
		unsigned mcd		: 1;
		unsigned led_type	: 1;
	};

	struct {
		unsigned BOOLS		: 8;
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
void _save(void);
void _load(void);

/* global variables */
region_t region = DEFAULT_REGION;
volatile __Bools_t bools;
volatile uint8_t turns = 0;
volatile uint8_t delay = 0;
volatile uint8_t debounce_delay = 0;
volatile uint32_t btn_counter = 0;

// interrupt handler (kept as simple/minimalistic as possible)
static void isr(void) __interrupt(0) {
	// monitor the MD reset button
	if (INTF) {
		if (!debounce_delay) {
			debounce_delay = DEBOUNCE_DELAY;
			bools.btn_event = true;
		}
		INTF = 0;
	}

	// when T16 'ticks'
	if (TMR1IF) {
		bools.tick = true;
		TMR1IF = 0;
		TMR1_reset();	// set the counter to the start timer
	}

	// EEPROM write complete
	if (EEIF) {
		WREN = 0;
		EEIF = 0;
	}
}

// timers and pin setup
void setup() {
	COMPARATOR_OFF();

	setPinOutput(RED);
	setPinOutput(GREEN);
	setPinOutput(BLUE);

	setPinOutput(VIDEO);
	setPinOutput(LANGUAGE);
	setPinOutput(INV_VID);
	setPinOutput(INV_LANG);

	setPinOutput(RESET);
	setPin(RESET);

	setPinInput(LED_TYPE);
	setPinPullup(LED_TYPE);

	setPinInput(MCD);
	setPinPullup(MCD);

	setPinInput(BUTTON);
	setPinPullup(BUTTON);

	NOT_RAPU = 0; // enable pullups on PORTA

	bools.tick		= false;
	bools.btn_event		= false;
	bools.show_regions	= false;
	bools.confirm_region	= false;
	bools.reset		= false;
	bools.led_state		= true;
	bools.mcd		= false;

	// enable T16 timer to generate an interrupt about every 16 milliseconds
	TMR1CS			= 0;	// timer clock source internal clock
	NOT_T1SYNC		= 1;	// ingore synchronization, internal clock is used
	T1OSCEN			= 0;	// LP oscillator off
	T1CKPS0			= 0;	// no prescaler
	T1CKPS1			= 0;	// no prescaler
	TMR1GE			= 0;	// gate enable off
	TMR1IF			= 0;	// clear the overflow register
	TMR1_reset();			// set the counter to the start timer
	TMR1ON			= 1;	// timer 1 ON

	// Enable interrupts
	INTEDG			= 0;	// Interrupt on falling edge
	INTE			= 1;	// PA2 external interrupt enabled
	RAIE			= 0;	// PORTA change interrupt disabled
	PEIE			= 1;	// peripheral interrupt enabled
	TMR1IE			= 1;	// TIMER1 overflow interrupt enabled
	EEIE			= 1;	// EEPROM write complete interrupt enabled
	GIE			= 1;	// global interrupt enabled
} // setup()

void main(void) {
	// Setup pin and timers
	setup();

	// read the Mega CD mode pin
	bools.mcd = isPinLow(MCD);

	// read the type of led used
	bools.led_type = isPinHigh(LED_TYPE);

	// load last region
	_load();

	// set default region
	set_region();
	set_led(region);

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
					_save();
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
			setPin(VIDEO);
			clearPin(LANGUAGE);
			break;
		case EU:
			clearPin(VIDEO);
			setPin(LANGUAGE);
			break;
		case US:
			setPin(VIDEO);
			setPin(LANGUAGE);
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
			setPin(INV_LANG);
			clearPin(INV_VID);
			break;
		case EU:
			clearPin(INV_LANG);
			setPin(INV_VID);
			break;
		case US:
			clearPin(INV_LANG);
			clearPin(INV_VID);
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

// save current region
void _save(void) {
	EEDATA	= region;	// data to save
	EEADR	= 0;		// address where save data
	WREN	= 1;		// enable writes to data EEPROM
	GIE	= 0;		// disable interrupts
	EECON2	= 0x55;		// required for EEPROM WRITE
	EECON2	= 0x0AA;	// required for EEPROM WRITE
	WR	= 1;		// start writing
	GIE	= 1;		// enable interrupts
} //_save()

// restore last region
void _load(void) {
	EEADR = 0;
	RD = 1;
	region = (EEDATA % REGIONS);
} //_load()
