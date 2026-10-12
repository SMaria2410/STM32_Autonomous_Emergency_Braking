#ifndef I2C_H_
#define I2C_H_

#include "stm32f4xx.h"

void i2c_init(void);
uint8_t i2c_write(uint8_t sensor_addr, uint8_t sensor_write_reg, const uint8_t *data, uint8_t n);
uint8_t i2c_read(uint8_t sensor_addr, uint8_t sensor_read_reg, uint8_t *data, uint8_t n);

#endif
