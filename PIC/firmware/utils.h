#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdbool.h>
#include <stdint.h>

#define HIGH	1
#define LOW	0

#define NOP() __asm nop __endasm
// ----------------------------------------------------------------------
#define SetFuses(fuses)	__code uint16_t __at (_CONFIG) __configword = fuses

#define PORT_EX(pin)	_CONCAT(R,pin)
#define TRIS_EX(pin)	_CONCAT(TRIS,pin)
#define WPU_EX(pin)	_CONCAT(WPU,pin)

#define setPinOutput(pinspec)	TRIS_EX(pinspec) = 0
#define setPinInput(pinspec)	TRIS_EX(pinspec) = 1

#define setPinPullup(pinspec)	WPU_EX(pinspec) = 1
#define clearPinPullup(pinspec)	WPU_EX(pinspec) = 0

#define setPin(pinspec)		PORT_EX(pinspec) = 1
#define clearPin(pinspec)	PORT_EX(pinspec) = 0

#define isPinHigh(pinspec)	PORT_EX(pinspec)
#define isPinLow(pinspec)	!(PORT_EX(pinspec))

// ----------------------------------------------------------------------

#endif /* __UTILS_H__ */
