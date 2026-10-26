#ifndef SPI_H_
#define SPI_H_

#include "stm32f4xx.h"

void spi_init(void);
void spi_transmit(const uint8_t *data, uint16_t n);

#endif
