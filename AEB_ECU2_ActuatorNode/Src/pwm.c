#include "pwm.h"

void servo_init(void) {
	//enable clock access la GPIOA
	RCC->AHB1ENR |= (1U<<0);
	//PA0 in AF mode (MODER) si AF1 = TIM2_CH1 -> SET AND CLEAR
	GPIOA->MODER &=~ (0x3U << 0);
	GPIOA->MODER |= (1U<<1);
	GPIOA->AFR[0] &=~ (0xFU << 0);
	GPIOA->AFR[0] |= (1U<<0);
	//enable clock access la TIM2 (APB1ENR)
	RCC->APB1ENR |= (1U<<0);
	//PSC = 15 -> 16 MHz / (15 + 1) = 1 MHz -> timerul numara in pasi de 1 us
	TIM2->PSC = (15U<<0);
	//ARR = 19999 -> numara 0-19999 = 20 ms = perioada servo ului
	TIM2->ARR = (19999U<<0);
	//CCR1 = 1500 -> puls de 1.5 ms la pornire (servo la mijloc)
	TIM2->CCR1 = (1500U<<0);
	//CCMR1: canalul 1 in PWM mode 1 (campul OC1M) + preload pt CCR1 (OC1PE)
	TIM2->CCMR1 &=~ (0x7U<<4);
	TIM2->CCMR1 |= (0x3U<<5);
	TIM2->CCMR1 |= (1U<<3);
	//CCER: activam iesirea canalului 1 (CC1E)
	TIM2->CCER |= (1U<<0);
	//CR1: preload pt ARR (ARPE)
	TIM2->CR1 |= (1U<<7);
	//EGR: generam un update (UG) ca sa se incarce PSC si ARR inainte de pornire
	TIM2->EGR = (1U<<0);
	//CR1: pornim timerul (CEN)
	TIM2->CR1 |= (1U<<0);
}

void servo_set_pulse_us(uint32_t pulse) {
	//limitam valoarea intre 1400 si 2400 (ca sa nu fortam servo ul peste capete)
	if(pulse < 1400) {
		pulse = 1400;
	} else if (pulse > 2400) {
		pulse = 2400;
	}
	//scriem valoarea in CCR1
	TIM2->CCR1 = (pulse << 0);
}

