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

	uint8_t rxaddr[] = { 194, 178, 83 };
	uint8_t txaddr[] = { 194, 178, 82 };

	RX_DDR &= ~(1 << RX_PIN);
	RX_PORT |= (1 << RX_PIN); 
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);

	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	sei();
	radio_listen();

	for (;;) {
		if (rxdr) {
			n = radio_recv(buf, MAXPDLEN);
			buf[n] = '\0';
			rxdr = 0;
			if (n > 0) {
				uart_write("INFO: ");
				uart_write_line(buf);
			}
		}

		//_delay_ms(1000);
	}

	return 0;
}

ISR(RX_PCINTVEC)
{
	rxdr = 1;
}
