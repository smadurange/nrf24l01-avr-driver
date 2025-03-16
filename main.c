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

const char *bittab[16] = {
	[ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
	[ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
	[ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
	[12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

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

static inline void print_config(void)
{
	char s[21];
	uint8_t i, rv;

	uart_write_line("NRF24L01 configuration:");

	for (i = 0x00; i <= 0x17; i++) {
		rv = read_reg(i);
		sprintf(s, "\t0x%02X: 0x%02X  %s%s", i, rv, bittab[rv >> 4], bittab[rv & 0x0F]);
		uart_write_line(s);
	}
}

void radio_init(void)
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);

	SPCR |= (1 << SPE) | (1 << MSTR);

	NRF_CE_DDR |= (1 << NRF_CE);
	NRF_CE_PORT &= ~(1 << NRF_CE);

	_delay_ms(100); /* power on reset delay */

	write_reg(0x00, 0b00001100);  /* use 2-byte CRC */
	write_reg(0x01, 0b00000000);  /* disable auto ack on all pipes */
	write_reg(0x02, 0b00000001);  /* enable rx address on pipe 0 */
	write_reg(0x03, 0b00000001);  /* set address width to 3 bytes */
	write_reg(0x04, 0b00000000);  /* disable auto retransmission */
	write_reg(0x05, 0b01110011);  /* use 2.515GHz channel */
	write_reg(0x06, 0b00000110);  /* set data rate to 1Mbps */
}

int main(void)
{
	uart_init();
	radio_init();
	print_config();

	return 0;
}
