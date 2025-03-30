#include <stdint.h>
#include <string.h>

#include "nrfm.h"
#include "uart.h"

int main(void)
{
	const char *s = "hello world!";

	uint8_t rxaddr[] = { 194, 178, 82 };
	uint8_t txaddr[] = { 194, 178, 83 };

	uart_init();
	radio_init(rxaddr);
	radio_print_config();
	radio_sendto(txaddr, s, strlen(s));

	return 0;
}
