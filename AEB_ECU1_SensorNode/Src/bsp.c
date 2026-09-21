#include "bsp.h"
#include "stm32f4xx.h"

#define GPIOAEN (1U<<0)
#define GPIOCEN (1U<<2)

#define PIN5 (1U<<5)
#define PIN13 (1U<<13)

/*void led_init(void) {
	//enable clock pt GPIOA
	RCC->AHB1ENR |= GPIOAEN;
	//setam PA5 mode = output mode (bit 10 = 1; bit 11 = 0)
	GPIOA->MODER |= (1U<<10);
	GPIOA->MODER &=~ (1U<<11);
}

void led_on(void) {
	//setam PA5 high
	GPIOA->ODR |= PIN5;
}

void led_off(void) {
	//setam PA5 low
	GPIOA->ODR &=~ PIN5;

}
*/



void button_init(void) {
	//enable clock pt PORTC
	RCC->AHB1ENR |= GPIOCEN;
	//setam PC13 ca pin de input (bit 26 = 0; bit 27 = 0)
	GPIOC->MODER &=~ (1U<<26);
	GPIOC->MODER &=~ (1U<<27);

}

bool get_btn_state(void) {
	//verificam daca butonul este apasat:
	//daca e high => butonul nu e apasat => ret false
	if(GPIOC->IDR & PIN13) {
		return false;
	} else {
		return true;
	}
	//daca e low => butonul e apasat => ret true

}
