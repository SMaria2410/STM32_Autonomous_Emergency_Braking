#include <stdio.h>
#include "stm32f4xx.h"
#include "fpu.h"
#include "uart.h"
#include "timebase.h"
#include "bsp.h"
#include "adc.h"
#include "can_driver.h"

//buffer in care retinem datele din receive
uint8_t rx_data[9];
//instanta a headerului de receive
can_rx_header_typedef rx_header;
//definim fifo0
#define CAN_RX_FIFO0 0x00
//contor pt debugging(cate mesaje am primit)
uint8_t count = 0;

void CAN1_RX0_IRQHandler(void) {
	//daca avem message pending in FIFO 0
	if((CAN1->RF0R & (0x03)) != 0) { //citim primii 2 biti din reg si verificam sa nu fie 0
		can_get_rx_message(&rx_header, CAN_RX_FIFO0, rx_data);
		//refacem distanta din cei 2 octeti primiti: [0] = octetul mare, [1] = octetul mic (ca la readReg16Bit)
		uint16_t dist = ((uint16_t)rx_data[0] << 8) | ((uint16_t)rx_data[1] << 0);
		//afisam distanta primita in mm
		printf("Distance: %u mm\r\n", dist);
		//dam toggle la led
		led_toggle();
		count++;
	}
}

bool btn_state;
//uint32_t sensor_value;
int main()
{
	//enable fpu
	fpu_enable();

	//initializam debug UART
	debug_uart_init();

	//intializam timebase ul
	timebase_init();

	//initializam led ul
	led_init();
	//initializam butonul
	button_init();

	//initializam modulul ADC
	//pa1_adc_init();
	//pornim modulul ADC
	//start_conversion();

	//initializam buzzerul si incepem cu el off
	buzzer_init();
	buzzer_off();

	/*if ((GPIOB->IDR & (1U << 8)) == 0) {
	    printf("PB8 e blocat la gnd\r\n");
	} else {
	    printf("PB8 e high\r\n");
	}*/

	//initializam modulul can
	can_gpio_init();
	can_params_init(1); //setam normal mode
	can_filter_config(0x244, 14, 0, 0);
	can_start();

	printf("Receiver ready...\n\r");

	while(1) {
		//printf("Hello from STM32\n\r");
		//delay(1); //vrem sa dam print la mesaj in fiecare secunda
		//pornim led ul
		//led_on();
		//verificam starea butonului
		/*btn_state = get_btn_state();
		if(btn_state == true) {
			//daca butonul este apasat, pornim buzzerul
			buzzer_on();
		} else {
			buzzer_off();
		}*/
		//citim valoarea digitala rezultata
		//sensor_value = adc_read();

		//DEBUG
		//printf("RF0R: 0x%08lX | ESR: 0x%08lX\r\n", CAN1->RF0R, CAN1->ESR);
		//delay(1);
		// Verificam daca a intrat vreun mesaj in FIFO 0
	 }
}
