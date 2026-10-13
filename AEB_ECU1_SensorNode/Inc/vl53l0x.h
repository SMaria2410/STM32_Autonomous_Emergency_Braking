#ifndef VL53L0X_H_
#define VL53L0X_H_

#include <stdint.h>
#include <stdbool.h>

bool     vl53l0x_init(void);
void     vl53l0x_set_timeout(uint16_t timeout);
void     vl53l0x_start_continuous(uint32_t period_ms);
uint16_t vl53l0x_read_range_mm(void);
bool     vl53l0x_timeout_occurred(void);

#endif
