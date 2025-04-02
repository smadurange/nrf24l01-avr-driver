#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"

#define SPI_SS             PB2
#define SPI_SCK            PB5
#define SPI_MISO           PB4
#define SPI_MOSI           PB3
#define SPI_DDR            DDRB
#define SPI_PORT           PORTB

#define NRF_CE             PB1  
#define NRF_CE_DDR         DDRB  
#define NRF_CE_PORT        PORTB  

#define NRF_IRQ            PD7
#define NRF_IRQ_DDR        DDRD
#define NRF_IRQ_PORT       PORTD
#define NRF_IRQ_PCIE       PCIE2
#define NRF_IRQ_PCINT      PCINT23
#define NRF_IRQ_PCMSK      PCMSK2
#define NRF_IRQ_PCINTVEC   PCINT2_vect

#define NRF_NOP            0xFF
#define NRF_R_REGISTER     0x1F
#define NRF_W_REGISTER     0x20

#define NRF_PWR_UP            1
#define NRF_PRIM_RX           0

#define MAXPDLEN   32
#define LEN(a)     (sizeof(a) / sizeof(a[0]))

const char *bittab[16] = {
	[ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
	[ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
	[ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
	[12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

static inline uint8_t read_reg(uint8_t reg)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = reg & NRF_R_REGISTER;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = NRF_NOP;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);
	return SPDR;
}

static inline void write_reg(uint8_t reg, uint8_t val)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = (reg & 0x1F) | NRF_W_REGISTER;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = val;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);
}

static inline void read_reg_bulk(uint8_t reg, uint8_t *data, uint8_t n)
{
	uint8_t i;

	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = reg & NRF_R_REGISTER;
	while (!(SPSR & (1 << SPIF)))
		;
	for (i = 0; i < n; i++) {
		SPDR = NRF_NOP;
		while (!(SPSR & (1 << SPIF)))
			;
		data[i] = SPDR;
	}
	SPI_PORT |= (1 << SPI_SS);
}

static inline void setaddr(uint8_t reg, const uint8_t addr[ADDRLEN])
{
	uint8_t i;

	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = (reg & 0x1F) | NRF_W_REGISTER;
	while (!(SPSR & (1 << SPIF)))
		;
	for (i = ADDRLEN - 1; i >= 0; i--) {
		SPDR = addr[i];
		while (!(SPSR & (1 << SPIF)))
			;
	}
	SPI_PORT |= (1 << SPI_SS);
}

static inline void reset_irqs(void)
{
	rv = read_reg(0x07);
	if (rv != 0b00001110)
		write_reg(0x07, 0b01111110);
}

static inline void enable_tx(void)
{
	uint8_t rv;

	rv = read_reg(0x00);
	if ((rv & 0x03) != 0x02) {
		rv |= (1 << NRF_PWR_UP);
		rv &= ~(1 << NRF_PRIM_RX);
		write_reg(0x00, rv);
		_delay_ms(2);
	}
}

static inline void flush_tx_fifo(void)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = 0b11100001;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);
}

void radio_print_config(void)
{
	char s[22];
	uint8_t i, rv, addr[ADDRLEN];

	uint8_t regs[] = { 
		0x00, 0x01, 0x02, 0x03, 0x04, 
		0x05, 0x06, 0x07, 0x11, 0x1C, 0x1D 
	};

	uart_write_line("NRF24L01 config:");

	for (i = 0; i < LEN(regs); i++) {
		rv = read_reg(regs[i]);
		snprintf(s, LEN(s), "\t0x%02X: 0x%02X  %s%s", 
		    regs[i], rv, bittab[rv >> 4], bittab[rv & 0x0F]);
		uart_write_line(s);
	}

	read_reg_bulk(0x0A, addr, ADDRLEN);
	snprintf(s, LEN(s), "\r\n\t0x0A: %d.%d.%d", addr[2], addr[1], addr[0]);
	uart_write_line(s);
}

void radio_init(const uint8_t rxaddr[ADDRLEN])
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);

	NRF_CE_DDR |= (1 << NRF_CE);
	NRF_CE_PORT &= ~(1 << NRF_CE);

	NRF_IRQ_DDR &= ~(1 << NRF_IRQ);
	NRF_IRQ_PORT &= ~(1 << NRF_IRQ); 
	PCICR |= (1 << NRF_IRQ_PCIE);
	NRF_IRQ_PCMSK |= (1 << NRF_IRQ_PCINT);

	_delay_ms(110); /* power on reset delay */

	write_reg(0x00, 0b00111100);  /* use 2-byte CRC, enable only the rx interrupt  */
	write_reg(0x01, 0b00111111);  /* enable auto ack on all pipes */
	write_reg(0x02, 0b00000001);  /* enable rx address on pipe 0 */
	write_reg(0x03, 0b00000001);  /* set address width to 3 bytes */
	write_reg(0x04, 0b00101111);  /* 750uS retransmission delay, 15 tries */
	write_reg(0x05, 0b01110011);  /* use 2.515GHz channel */
	write_reg(0x06, 0b00001110);  /* set data rate to 1Mbps */
	write_reg(0x1D, 0b00000100);  /* enable dynamic payload length */
	write_reg(0x1C, 0b00000001);  /* enable dynamic payload length for pipe 0 */

	reset_irqs();
	setaddr(0x0A, rxaddr);
}

void radio_sendto(const uint8_t addr[ADDRLEN], const void *msg, uint8_t n)
{
	uint8_t i, j, j0, jmax;

	enable_tx();
	reset_irqs();
	flush_tx_fifo();

	setaddr(0x10, addr);
	setaddr(0x0A, addr);

	jmax = n - 1;

	for (i = 0; i < n; i += MAXPDLEN) {
		SPI_PORT &= ~(1 << SPI_SS);
		SPDR = 0b10100000;
		while (!(SPSR & (1 << SPIF)))
			;
		j0 = i + MAXPDLEN - 1;
		for (j = j0 < jmax ? j0 : jmax; j >= i; j--) {
			SPDR = ((uint8_t *)msg)[j];
			while (!(SPSR & (1 << SPIF)))
				;
		}
		SPI_PORT |= (1 << SPI_SS);

		NRF_CE_PORT |= (1 << NRF_CE);
		_delay_us(12);
		NRF_CE_PORT &= ~(1 << NRF_CE);

		// todo: check success
	}
}

