#ifndef NRFM_H
#define NRFM_H

#include <stdint.h>

void radio_init(uint8_t rxaddr[3]);
void radio_print_config(void);

#endif
