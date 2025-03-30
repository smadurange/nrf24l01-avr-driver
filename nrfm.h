#ifndef NRFM_H
#define NRFM_H

#include <stdint.h>

void radio_init(uint8_t rxaddr[3]);
void radio_print_config(void);
void radio_send(uint8_t rxaddr[ADDRLEN], uint8_t *data, uint8_t n);

#endif /* NRFM_H */
