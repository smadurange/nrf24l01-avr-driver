#ifndef NRFM_H
#define NRFM_H

#include <stdint.h>

#define ADDRLEN     3
#define MAXPDLEN   32

void radio_init(const uint8_t rxaddr[ADDRLEN]);

void radio_print_config(void);

void radio_listen(void);

uint8_t radio_recv(char *buf, uint8_t n);

void radio_sendto(const uint8_t addr[ADDRLEN], const char *msg, uint8_t n);

#endif /* NRFM_H */
