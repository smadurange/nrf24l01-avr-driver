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

#define LOCK_PIN       PC5
#define LOCK_DDR       DDRC
#define LOCK_PORT      PORTC
#define LOCK_PCIE      PCIE1
#define LOCK_PCINT     PCINT13 
#define LOCK_PCMSK     PCMSK1
#define LOCK_PCINTVEC  PCINT1_vect

static volatile int rxdr = 0;
static volatile int lock = 0;

static inline void init_rx(void)
{
	RX_DDR &= ~(1 << RX_PIN);
	RX_PORT |= (1 << RX_PIN); 
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);
}

static inline void init_btns(void)
{
	LOCK_DDR &= ~(1 << LOCK_PIN);
	LOCK_PORT |= (1 << LOCK_PIN);
	PCICR |= (1 << LOCK_PCIE);
	LOCK_PCMSK |= (1 << LOCK_PCINT);
}

static inline int is_btn_pressed(unsigned char btn)
{
	if (!((PINC >> btn) & 0x01)) {
		_delay_us(2000);
		return !((PINC >> btn) & 0x01);
	}
	
	return 0;
}

int main(void)
{
	uint8_t n;
	char buf[MAXPDLEN + 1];

	uint8_t rxaddr[ADDRLEN] = { 194, 178, 82 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 83 };

	init_rx();
	init_btns();

	//wdt_stop();
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
			if (strncmp(buf, "ACK", 3) == 0) {
				uart_write("INFO: ");
				uart_write_line(buf);
			}
		}

		if (lock) {
			radio_sendto(txaddr, "SYN", 3);
			lock = 0;
		}
	}

	return 0;
}

ISR(RX_PCINTVEC)
{
	rxdr = 1;
}

ISR(LOCK_PCINTVEC)
{
	if (is_btn_pressed(LOCK_PIN))	
		lock = 1;
}
