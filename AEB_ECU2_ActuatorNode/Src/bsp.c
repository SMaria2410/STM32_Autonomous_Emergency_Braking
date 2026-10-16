#include "bsp.h"
#include "stm32f4xx.h"

void led_init(void) {
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

void led_toggle(void) {
	//dam toggle la led
	GPIOA->ODR ^= (1U<<5);
}

/*void button_init(void) {
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
}*/

//pt buzzer
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

//pt led urile de status
void status_leds_init(void) {
	//enable clock access la GPIOB
	RCC->AHB1ENR |= (1U<<1);
	//setam PB5 (verde), PB4 (galben), PB10 (rosu) ca output
	GPIOB->MODER &=~ (0x3U << 10); //clear
	GPIOB->MODER &=~ (0x3U << 8);
	GPIOB->MODER &=~ (0x3U << 20);
	GPIOB->MODER |= (1U << 10); //set
	GPIOB->MODER |= (1U << 8);
	GPIOB->MODER |= (1U << 20);
	//le stingem pe toate la inceput
	GPIOB->BSRR = (1U << (5 + 16));
	GPIOB->BSRR = (1U << (4 + 16));
	GPIOB->BSRR = (1U << (10 + 16));
}


void status_leds_show(system_state_t state) {
	//in functie de stare:
	//STATE_SAFE -> verde aprins, galben si rosu stinse
	if(state == STATE_SAFE) {
		GPIOB->BSRR = (1U << 5) | (1U << (4 + 16)) | (1U << (10 + 16));
	}
	//STATE_WARNING -> galben aprins, verde si rosu stinse
	else if(state == STATE_WARNING) {
		GPIOB->BSRR = (1U << 4) | (1U << (5 + 16)) | (1U << (10 + 16));
	}
	//STATE_DANGER -> rosu aprins, verde si galben stinse
	else if(state == STATE_DANGER) {
		GPIOB->BSRR = (1U << 10) | (1U << (5 + 16)) | (1U << (4 + 16));
	}
	//STATE_OFF -> toate stinse
	else if(state == STATE_OFF) {
		GPIOB->BSRR = (1U << (5 + 16)) | (1U << (4 + 16)) | (1U << (10 + 16));
	}
}


