#include <avr/wdt.h>

#include "util.h"

void wdt_init(void)
{
	wdt_reset();
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = (1 << WDE) | (1 << WDP3) | (1 << WDP0);
}
