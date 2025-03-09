#include <avr/io.h>
#include <util/setbaud.h>

#include "uart.h"

void uart_init(void)
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
#if USE_2X
	UCSR0A |= (1 << U2X0);
#else
	UCSR0A &= ~(1 << U2X0);
#endif
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static inline void uart_write_char(char c)
{
	while (!(UCSR0A & (1 << UDRE0)))
		;
	UDR0 = c;
}

void uart_write(const char *s)
{
	for (; *s; s++)
		uart_write_char(*s);
}

void uart_write_line(const char *s)
{
	uart_write(s);
	uart_write_char('\r');
	uart_write_char('\n');
}
