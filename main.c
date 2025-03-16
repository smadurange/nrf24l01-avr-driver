#include <stdint.h>

#include "nrfm.h"
#include "uart.h"

int main(void)
{
	uint8_t rxaddr[] = { 82, 178, 194 };

	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	return 0;
}
