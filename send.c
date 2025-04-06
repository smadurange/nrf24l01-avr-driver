#include <stdint.h>
#include <string.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"

int main(void)
{
	const char *s = "hello world!";
	uint8_t slen = strlen(s);

	uint8_t rxaddr[ADDRLEN] = { 194, 178, 82 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 83 };

	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	for (;;) {
		radio_sendto(txaddr, s, slen);
		_delay_ms(2000);
	}


	return 0;
}
