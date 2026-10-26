#include <stdio.h>
#include "stm32f4xx.h"
#include "fpu.h"
#include "uart.h"
#include "timebase.h"
#include "bsp.h"
#include "adc.h"
#include "can_driver.h"
#include "pwm.h"
#include "spi.h"
#include "ssd1306.h"

//pragurile de distanta in mm
//sub 500 mm -> atentie (WARNING)
#define WARNING_DISTANCE_MM 500
//sub 200 mm -> pericol (DANGER)
#define DANGER_DISTANCE_MM 200
//valoarea pt eroare la senzor
#define SENSOR_ERROR_VALUE 65535

//pozitiile servo ului(latimea pulsului in us)
#define BRAKE_RELEASED_US 1400
#define BRAKE_APPLIED_US 2400

//buffer in care retinem datele din receive
uint8_t rx_data[9];
//instanta a headerului de receive
can_rx_header_typedef rx_header;
//definim fifo0
#define CAN_RX_FIFO0 0x00
//contor pt debugging(cate mesaje am primit)
uint8_t count = 0;

//distanta primita ultima data (volatile pt ca o modifica intreruperea si o citeste main ul)
volatile uint16_t last_dist;
//momentul (get_tick) la care am primit ultimul mesaj, tot volatile -> il folosim mai tarziu la timeout
volatile uint32_t last_msg_time;
//flag: 1 = a venit o distanta noua pe care main ul inca nu a procesat o, tot volatile
volatile uint8_t new_dist_flag = 0;

void CAN1_RX0_IRQHandler(void) {
	//daca avem message pending in FIFO 0
	if((CAN1->RF0R & (0x03)) != 0) { //citim primii 2 biti din reg si verificam sa nu fie 0
		can_get_rx_message(&rx_header, CAN_RX_FIFO0, rx_data);

		//refacem distanta in variabila globala (in loc de variabila locala dist)
		last_dist = ((uint16_t)rx_data[0] << 8) | ((uint16_t)rx_data[1] << 0);
		//salvam momentul primirii cu get_tick()
		last_msg_time = get_tick();
		//punem flag ul pe 1
		new_dist_flag = 1;

		/*//refacem distanta din cei 2 octeti primiti: [0] = octetul mare, [1] = octetul mic (ca la readReg16Bit)
		uint16_t dist = ((uint16_t)rx_data[0] << 8) | ((uint16_t)rx_data[1] << 0);
		//afisam distanta primita in mm
		printf("Distance: %u mm\r\n", dist);*/

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
	//button_init();

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

	//initializam LED urile de stare si le stingem
	status_leds_init();

	spi_init();
	ssd1306_init();

	//TEST SERVO
	servo_init();
	/*//intr o parte
	servo_set_pulse_us(800);
	delay(1000);
	//la mijloc
	servo_set_pulse_us(1400);
	delay(1000);
	//in cealalta parte
	servo_set_pulse_us(2000);
	delay(1000);*/
	//pornim cu frana eliberata
	servo_set_pulse_us(BRAKE_RELEASED_US);

	//initializam modulul can
	can_gpio_init();
	can_params_init(1); //setam normal mode
	can_filter_config(0x244, 14, 0, 0);
	can_start();

	printf("Receiver ready...\n\r");

	char text[22];

	while(1) {
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
		//delay(1); //verificam daca a intrat vreun mesaj in FIFO 0

		//daca flag ul e 1 (a venit o distanta noua):
		if(new_dist_flag == 1) {
			//punem flag ul inapoi pe 0
			new_dist_flag = 0;
			//facem o copie locala a distantei, ca sa nu se schimbe in timpul comparatiilor (fiind volatile)
			uint16_t last_dist_cpy = last_dist;
			//afisam distanta
			printf("Distance: %u mm \r\n", last_dist_cpy);

			ssd1306_clear();
			//pagina 0: "ECU2 - BRAKE" (centrat)
			ssd1306_draw_string_centered(0, "ECU2 - BRAKE");

			//alegem starea in functie de distanta:
			//buzzer: pornit pt WARNING si DANGER, oprit pt SAFE
			system_state_t state;
			//daca distanta e 65535 (ECU1 ne spune ca senzorul a dat timeout) -> DANGER (stare sigura)
			if(last_dist_cpy == SENSOR_ERROR_VALUE) {
				state = STATE_DANGER;
				servo_set_pulse_us(BRAKE_APPLIED_US);
				buzzer_on();
				ssd1306_draw_string_centered(5, "DANGER");

			}
			//altfel daca distanta < pragul de pericol -> DANGER
			else if(last_dist_cpy < DANGER_DISTANCE_MM) {
				state = STATE_DANGER;
				servo_set_pulse_us(BRAKE_APPLIED_US);
				buzzer_on();
				ssd1306_draw_string_centered(5, "DANGER");
			}
			//altfel daca distanta < pragul de atentie -> WARNING
			else if(last_dist_cpy < WARNING_DISTANCE_MM) {
				state = STATE_WARNING;
				servo_set_pulse_us(BRAKE_RELEASED_US);
				buzzer_on();
				ssd1306_draw_string_centered(5, "WARNING");
			}
			//altfel -> SAFE (inclusiv 8190 = nimic in raza)
			else {
				state = STATE_SAFE;
				servo_set_pulse_us(BRAKE_RELEASED_US);
				buzzer_off();
				ssd1306_draw_string_centered(5, "SAFE");
			}
			//aprindem LED ul pt starea aleasa
			status_leds_show(state);

			//pagina 2: distanta:
			//daca e SENSOR_ERROR_VALUE -> "SENSOR ERROR"
			if(last_dist_cpy == SENSOR_ERROR_VALUE) {
				ssd1306_draw_string_centered(2, "SENSOR ERROR");
			}
			//altfel daca >= 8190 -> "OUT OF RANGE"
			else if(last_dist_cpy >= 8190) {
				ssd1306_draw_string_centered(2, "OUT OF RANGE");
			}
			//altfel -> snprintf "DIST: %u mm", ca la ECU1
			else {
				snprintf(text, sizeof(text), "DIST: %u mm", last_dist_cpy);
				ssd1306_draw_string_centered(2, text);
			}
			ssd1306_update();
		}
	 }
}
