#include "adc.h"
#include "stm32f4xx.h"

#define GPIOAEN (1U<<0)
#define ADC1EN (1U<<8)

void pa1_adc_init(void) {
	//configuram pinul GPIO -> ADC:
	//dam enable la clock lui GPIOA
	RCC->AHB1ENR |= GPIOAEN;
	//setam PA1 mode = analog mode (11)->bitii 2 si 3
	GPIOA->MODER |= (1U<<2);
	GPIOA->MODER |= (1U<<3);
	//configuram modulul ADC:
	//dam enable la clock modulului ADC
	RCC->APB2ENR |= ADC1EN;
	//setam startul secventei de conversie
	ADC1->SQR3 = (1U<<0); //-> "=" simplu pt 00001
	//setam lungimea secventei de conversie -> vrem o singura conversie
	ADC1->SQR1 = 0x00;
	//enable ADC
	ADC1->CR2 |= (1U<<0);
}

void start_conversion(void) {
	//dam enable la conversia continua
	ADC1->CR2 |= (1U<<1);
	//pornim conversia ADC
	ADC1->CR2 |= (1U<<30);
}

uint32_t adc_read(void) {
	//asteptam sa fie completata conversia -> citim status register si trebuie sa fie 1 ca sa terminam asteptarea
	while(!(ADC1->SR & (1U<<1))) ; //polling cu busy waiting
	//citim valoarea din conversie
	uint32_t converted_value = ADC1->DR;
	return converted_value;
}
