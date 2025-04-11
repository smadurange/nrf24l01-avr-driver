#include <stdint.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

#define RX_PIN         PD7
#define RX_DDR         DDRD
#define RX_PORT        PORTD
#define RX_PCIE        PCIE2
#define RX_PCINT       PCINT23
#define RX_PCMSK       PCMSK2
#define RX_PCINTVEC    PCINT2_vect

static volatile int rxdr = 0;

int main(void)
{
	uint8_t n;
	char buf[MAXPDLEN + 1];

	uint8_t rxaddr[ADDRLEN] = { 194, 178, 82 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 83 };

	//wdt_stop();
	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	sei();
	radio_listen();

	_delay_ms(2000);
	radio_sendto(txaddr, "SYN", 3);

	for (;;) {
		if (rxdr) {
			n = radio_recv(buf, MAXPDLEN);
			buf[n] = '\0';
			rxdr = 0;
			if (strncmp(buf, "ACK", 3) == 0) {
				uart_write("INFO: ");
				uart_write_line(buf);
			}
		} else {
			uart_write_line("No IRQ");
		}

		_delay_ms(2000);
		radio_sendto(txaddr, "SYN", 3);
	}

	return 0;
}

ISR(RX_PCINTVEC)
{
	rxdr = 1;
}
