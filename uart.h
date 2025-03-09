#ifndef UART_H
#define UART_H

void uart_init(void);
void uart_write(const char *s);
void uart_write_line(const char *s);

#endif /* UART_H */
