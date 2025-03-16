#include <stdint.h>

#include "nrfm.h"
#include "uart.h"

int main(void)
{
	uint8_t rxaddr[] = { 194, 178, 82 };

	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	return 0;
}
