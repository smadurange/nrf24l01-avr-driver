#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

#include "uart.h"

#define SPI_SS         PB2
#define SPI_SCK        PB5
#define SPI_MISO       PB4
#define SPI_MOSI       PB3
#define SPI_DDR        DDRB
#define SPI_PORT       PORTB

#define NRF_CE         PB1  
#define NRF_CE_DDR     DDRB  
#define NRF_CE_PORT    PORTB  

#define NRF_IRQ        PD7  

#define NRF_RST_DELAY_MS  100

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

static inline void write_reg(uint8_t reg, uint8_t val)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = (reg & 0x1F) | 0x20;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = val;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);
}

void radio_init(void)
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);

	NRF_CE_DDR |= (1 << NRF_CE);
	NRF_CE_PORT &= ~(1 << NRF_CE);

	_delay_ms(NRF_RST_DELAY_MS);

	// Change the default 1-byte CRC code to 2-byte CRC code
	write_reg(0x00, 0b00001100);
}

int main(void)
{
	char s[15];
	uint8_t rv;

	uart_init();
	radio_init();

	for (int i = 0; i < 10; i++) {
		rv = read_reg(0x00);
		sprintf(s, "config: 0x%x", rv);
		uart_write_line(s);
	}

	return 0;
}
