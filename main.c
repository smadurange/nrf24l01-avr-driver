#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

#include "uart.h"

#define SPI_SS     PB2
#define SPI_SCK    PB5
#define SPI_MISO   PB4
#define SPI_MOSI   PB3
#define SPI_DDR    DDRB
#define SPI_PORT   PORTB

#define NRF_CE     PB1  
#define NRF_IRQ    PD7  

static inline uint8_t read_reg(uint8_t reg)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = reg & 0x1F;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = 0xFF;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);
	return SPDR;
}

void radio_init(void)
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);

	// CE=0 for standby-i mode
	DDRB |= (1 << NRF_CE);
	PORTB &= ~(1 << NRF_CE);

	_delay_ms(500);
}

int main(void)
{
	char s[5];

	uart_init();
	radio_init();

	for (int i = 0; i < 10; i++) {
		uint8_t val = read_reg(0x00);
		sprintf(s, "%x", val);

		uart_write("CONFIG: ");
		uart_write_line(s);
	}

	return 0;
}
