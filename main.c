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

struct radio_cfg {
	uint8_t netid;
	uint8_t nodeid;
	uint8_t pipeid;
};

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

static inline void write_reg_check(uint8_t reg, uint8_t val, uint8_t ref)
{
	while (read_reg(reg) != ref) {
		SPI_PORT &= ~(1 << SPI_SS);
		SPDR = (reg & 0x1F) | 0x20;
		while (!(SPSR & (1 << SPIF)))
			;
		SPDR = val;
		while (!(SPSR & (1 << SPIF)))
			;
		SPI_PORT |= (1 << SPI_SS);
	}
}

static inline void write_reg(uint8_t reg, uint8_t val)
{
	write_reg_check(reg, val, val);
}

static inline void read_reg_bulk(uint8_t reg, uint8_t *data, uint8_t n)
{
	uint8_t i;

	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = reg & 0x1F;
	while (!(SPSR & (1 << SPIF)))
		;
	for (i = 0; i < n; i++) {
		SPDR = 0xFF;
		while (!(SPSR & (1 << SPIF)))
			;
		data[i] = SPDR;
	}
	SPI_PORT |= (1 << SPI_SS);
}

static inline void write_reg_bulk(uint8_t reg, uint8_t *data, uint8_t n)
{
	uint8_t i;

	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = (reg & 0x1F) | 0x20;
	while (!(SPSR & (1 << SPIF)))
		;
	for (i = 0; i < n; i++) {
		SPDR = data[i];
		while (!(SPSR & (1 << SPIF)))
			;
	}
	SPI_PORT |= (1 << SPI_SS);
}

static inline void print_config(void)
{
	char s[22];
	uint8_t i, rv;

	uart_write_line("NRF24L01 configuration:");

	for (i = 0x00; i <= 0x17; i++) {
		rv = read_reg(i);
		snprintf(s, (sizeof(s) / sizeof(s[0])), 
		    "\t0x%02X: 0x%02X  %s%s", i, rv, 
		    bittab[rv >> 4], bittab[rv & 0x0F]);
		uart_write_line(s);
	}
}

void radio_init(struct radio_cfg *cfg)
{
	uint8_t rx_addr[3];

	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);

	NRF_CE_DDR |= (1 << NRF_CE);
	NRF_CE_PORT &= ~(1 << NRF_CE);

	_delay_ms(110); /* power on reset delay */

	write_reg(0x00, 0b00001100);  /* set 2-byte CRC */
	write_reg(0x01, 0b00111111);  /* enable auto ack on all pipes */
	write_reg(0x02, 0b00000001);  /* enable rx address on pipe 0 */
	write_reg(0x03, 0b00000001);  /* set address width to 3 bytes */
	write_reg(0x04, 0b00101111);  /* 750uS retransmission delay, 15 tries */
	write_reg(0x05, 0b01110011);  /* use 2.515GHz channel */
	write_reg(0x06, 0b00001110);  /* set data rate to 1Mbps */

	write_reg_check(0x07, 0b01110000, 0b00001110);  /* clear rx, tx, max_rt interrupts */

	rx_addr[0] = cfg->pipeid;
	rx_addr[1] = cfg->nodeid;
	rx_addr[2] = cfg->netid;
	write_reg_bulk(0x0A, rx_addr, (sizeof(rx_addr) / sizeof(rx_addr[0])));
}

int main(void)
{
	struct radio_cfg cfg;
	cfg.netid = 0;
	cfg.nodeid = 0;
	cfg.pipeid = 0;

	uart_init();
	radio_init(&cfg);
	print_config();

	return 0;
}
