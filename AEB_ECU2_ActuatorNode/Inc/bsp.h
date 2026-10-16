#ifndef __BSP_H__
#define __BSP_H__

#include <stdint.h>
#include <stdbool.h>

void led_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);
void button_init(void);
bool get_btn_state(void);
void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);

//starile sistemului: STATE_OFF, STATE_SAFE, STATE_WARNING, STATE_DANGER
typedef enum {
	STATE_OFF, STATE_SAFE, STATE_WARNING, STATE_DANGER
}system_state_t;
//prototip: initializarea LED urilor de stare
void status_leds_init(void);
//prototip: functia care primeste starea si aprinde LED ul corespunzator
void status_leds_show(system_state_t state);

#endif
