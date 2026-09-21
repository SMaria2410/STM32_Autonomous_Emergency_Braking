#include <stdint.h>
#include "uart.h"

#define GPIOAEN (1U<<0)
#define UART2EN (1U<<17)

#define DBG_UART_BAUDRATE 115200
#define SYS_FREQ 16000000
#define APB1_CLK SYS_FREQ
#define CR1_TE (1U<<3)
#define CR1_UE (1U<<13)
#define SR_TXE (1U<<7)

static uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate);
static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate);
static void uart_write(int ch);

int __io_putchar(int ch) {
	uart_write(ch);
	return ch;
}

void debug_uart_init(void){
	//pt a sti pasii ne uitam in alternate function table pag 58 din datasheet
	//registrele le luam din reference manual

	//enable clock access la GPIOA
	RCC->AHB1ENR |= GPIOAEN;
	//dam set la mode ul portului PA2 = alternate function mode
	GPIOA->MODER &=~(1U<<4); //setam bitul 4 la 0
	GPIOA->MODER |= (1U<<5); //setam bitul 5 la 1
	//dam set la alternate function type = AF7(UART2_TX)
	//AFR[0] pt ca lucram cu afr low, daca aveam registru 8-15 lucram cu AFR[1]
	GPIOA->AFR[0] |= (1U<<8);
	GPIOA->AFR[0] |= (1U<<9);
	GPIOA->AFR[0] |= (1U<<10);
	GPIOA->AFR[0] &=~ (1U<<11);

	//enable clock access la UART2
	RCC->APB1ENR |= UART2EN;
	//configuram uart baudrate
	uart_set_baudrate(APB1_CLK, DBG_UART_BAUDRATE);
	//configuram directia de transfer (ori tx, ori rx, ori ambele)
	USART2->CR1 = CR1_TE; //setam doar bitul 3 la 1, restul setam 0 (vrem doar tx)
	//enable UART Module
	USART2->CR1 |= CR1_UE;

}

static void uart_write(int ch) {
	//ne asiguram ca registrul transmit data e empty
	//cu & citim bitul din registru (daca returneaza 1, bitul din SR e 1)
	while(!(USART2->SR & SR_TXE)) {
		//daca e 0, asteptam
	}
	//scriem in registrul transmit data
	USART2->DR = (ch & 0xFF);
}

static uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate) {
	return ((periph_clk + (baudrate/2U))/baudrate);
}

static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate) {
	USART2->BRR = compute_uart_bd(periph_clk, baudrate);
}
