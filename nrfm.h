#ifndef NRFM_H
#define NRFM_H

#include <stdint.h>

#define ADDRLEN     5

void radio_init(const uint8_t rxaddr[ADDRLEN]);
void radio_print_config(void);
void radio_sendto(const uint8_t addr[ADDRLEN], const void *msg, uint8_t n);

#endif /* NRFM_H */
