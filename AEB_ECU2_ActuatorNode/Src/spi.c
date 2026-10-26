#include "spi.h"

static volatile uint16_t ovr_clear_var;

//PB13 = SCK (ceasul), PB15 = MOSI (datele) -> amandoi pe AF5
void spi_init(void) {
	//enable clock pt GPIOB (RCC->AHB1ENR)
	RCC->AHB1ENR |= (1U<<1);
	//PB13 si PB15 in mod alternate function
	GPIOB->MODER &=~ (0x3U<<26);
	GPIOB->MODER |= (1U<<27);
	GPIOB->MODER &=~ (0x3U<<30);
	GPIOB->MODER |= (1U<<31);
	//viteza fast pt PB13 si PB15
	GPIOB->OSPEEDR &=~ (0x3U<<26);
	GPIOB->OSPEEDR |= (1U<<27);
	GPIOB->OSPEEDR &=~ (0x3U<<30);
	GPIOB->OSPEEDR |= (1U<<31);
	//AF5 pt PB13 si PB15
	GPIOB->AFR[1] &=~ (0xFU << 20);
	GPIOB->AFR[1] |= (1U<<20) | (1U<<22);
	GPIOB->AFR[1] &=~ (0xFU << 28);
	GPIOB->AFR[1] |= (1U<<28) | (1U<<30);
	//enable clock pt SPI2
	RCC->APB1ENR |= (1U<<14);
	//configuram SPI2->CR1 cu SPE inca 0 (configuratia se face cu SPI oprit)
	SPI2->CR1 &=~ (0xFFFFU<<0);
	//CPOL = 0, CPHA = 0 -> mode 0 (ceasul sta jos, datele citite pe frontul crescator)
	//SSM = 1 si SSI = 1 -> NSS gestionat din software
	SPI2->CR1 |= (1U<<9);
	SPI2->CR1 |= (1U<<8);
	//MSTR = 1 -> noi suntem master
	SPI2->CR1 |= (1U<<2);
	//BR = 001 -> fPCLK/4 = 16/4 = 4 MHz
	SPI2->CR1 |= (1U<<3);
	//LSBFIRST = 0 (MSB first), DFF = 0 (cadre de 8 biti)

	//BIDIMODE = 0, RXONLY = 0 -> full duplex normal
	//pornim SPI
	SPI2->CR1 |= (1U<<6);
}

void spi_transmit(const uint8_t *data, uint16_t n) {
	//pt fiecare octet din data
	for(int i = 0; i < n; i++) {
		//asteptam TXE = 1 in SPI2->SR (bufferul de transmisie e gol, putem pune urmatorul octet)
		while((SPI2->SR & (1U<<1)) == 0U) {}
		//scriem octetul in SPI2->DR
		SPI2->DR = (uint8_t)data[i];
	}
	//dupa ultimul octet:
	//asteptam TXE = 1 (ultimul octet a trecut din buffer in shift register)
	while((SPI2->SR & (1U<<1)) == 0U) {}
	//apoi asteptam BSY = 0 (s a terminat si de iesit pe fir, bit cu bit)
	while((SPI2->SR & (1U<<7)) != 0U) {}
	//stergem OVR
	ovr_clear_var = (uint8_t)SPI2->DR;
	ovr_clear_var = SPI2->SR;
}
