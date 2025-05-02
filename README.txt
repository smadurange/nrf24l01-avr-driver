NRF24L01+ RF MODULE

This is a driver for NRF24L01+ RF module from Nordic Semiconductor. The driver
targets AVR microcontrollers, and is written in C. I've tested this with
ATmega328P. Note that this uses Enhanced ShockBurst™ feature.

The module is 5V tolerant. You can directly interface it with the ATmega328P's
SPI port. 

The ding.c and dong.c are the applications. Upload them to two MCUs, each
connected to a RF module for them to talk to each other. Ground INT0 or INT1
from ding.c MCU to start signalling.

nrfm.c and nrfm.h contain the driver code.

