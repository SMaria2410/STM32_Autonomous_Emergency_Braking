#ifndef PWM_H
#define PWM_H

#include "stm32f4xx.h"

//prototip: initializarea servo ului (timer + pin)
void servo_init(void);
//prototip: functia care seteaza latimea pulsului in microsecunde (1000-2000)
void servo_set_pulse_us(uint32_t pulse);

#endif
