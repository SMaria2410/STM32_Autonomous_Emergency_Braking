#ifndef CAN_DRIVER_H_
#define CAN_DRIVER_H_

#include "stm32f4xx.h"

void can_gpio_init(void);
void can_params_init(uint8_t mode);
void can_start(void);

#endif
