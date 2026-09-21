#include "bsp.h"
#include "stm32f4xx.h"

/*void led_init(void) {
	//enable clock pt GPIOA
	RCC->AHB1ENR |= (1U<<0);
	//setam PA5 mode = output mode (bit 10 = 1; bit 11 = 0)
	GPIOA->MODER |= (1U<<10);
	GPIOA->MODER &=~ (1U<<11);
}

void led_on(void) {
	//setam PA5 high
	GPIOA->ODR |= (1U<<5);
}

void led_off(void) {
	//setam PA5 low
	GPIOA->ODR &=~ (1U<<5);

}
*/

void buzzer_init(void) {
	//enable clock pt GPIOA
	RCC->AHB1ENR |= (1U<<0);
	//setam PA6 mode = output mode (bit 12 = 1, bit 13 = 0)
	GPIOA->MODER |= (1U<<12);
	GPIOA->MODER &=~ (1U<<13);
}

void buzzer_on(void) {
	//setam BSRR ul high pt PA6 => punem 0 in bsrr la 6+16
	GPIOA->BSRR = (1U<<(6 + 16));
}

void buzzer_off(void) {
	//setam BSRR ul low pt PA6 => punem 1 in bsrr la 6
	GPIOA->BSRR = (1U<<6);
}

void button_init(void) {
	//enable clock pt PORTC
	RCC->AHB1ENR |= (1U<<2);
	//setam PC13 ca pin de input (bit 26 = 0; bit 27 = 0)
	GPIOC->MODER &=~ (1U<<26);
	GPIOC->MODER &=~ (1U<<27);

}

bool get_btn_state(void) {
	//verificam daca butonul este apasat:
	//daca e high => butonul nu e apasat => ret false
	if(GPIOC->IDR & (1U<<13)) {
		return false;
	} else {
		return true;
	}
	//daca e low => butonul e apasat => ret true

}
