#include <stdio.h>
#include "stm32f4xx.h"
#include "fpu.h"
#include "uart.h"
#include "timebase.h"
#include "bsp.h"
#include "adc.h"
#include "can_driver.h"
#include "i2c.h"
#include "vl53l0x.h"
#include "spi.h"
#include "ssd1306.h"

//buffer in care retinem datele pt transmit
//uint8_t tx_data[5];
//instanta a headerului de transmit
can_tx_header_typedef tx_header;
//mailbox ul de transmit
uint32_t tx_mailbox[3];

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

	//initializam modulul can
	can_gpio_init();
	can_params_init(1); //setam normal mode
	can_start();

	//initializam i2c
	i2c_init();
	//initializam senzorul ToF:
	//setam timeout ul pt citiri la 500 ms (daca senzorul nu raspunde, functiile nu raman blocate)
	vl53l0x_set_timeout(500);
	//initializam senzorul; daca vl53l0x_init returneaza false -> afisam un mesaj de eroare
	if(vl53l0x_init() == false) {
		printf("Sensor initialization error!\r\n");
	}
	//pornim masurarea continua (cu 0 = masuratori una dupa alta, fara pauza)
	vl53l0x_start_continuous(0);

	spi_init();
	ssd1306_init();

	//TEST DISPLAY: chenar pe marginea ecranului
	/*ssd1306_clear();
	//sus si jos
	for(int x = 0; x < 128; x++) {
		ssd1306_set_pixel(x, 0, 1);
		ssd1306_set_pixel(x, 63, 1);
	}
 	//stanga si dreapta
	for(int y = 0; y < 64; y++) {
		ssd1306_set_pixel(0, y, 1);
		ssd1306_set_pixel(127, y, 1);
	}
	//ssd1306_draw_string(10, 3, "test");
	ssd1306_update();*/

	/*
	//TEST PING SENZOR(write)
	//trimitem doar adresa senzorului + registrul 0xC0, fara date (n = 0, data = NULL)
	uint8_t sensor_result0 = i2c_write(0x29, 0xC0, NULL, 0);
	//afisam ce returneaza functia (0 = senzorul a raspuns cu ACK, 1 = nu a raspuns)
	printf("rezultat ping: %d\r\n", sensor_result0);

	//TESTE PT 1/2/3 octeti
	//declaram un buffer de 3 octeti pt rezultat
	uint8_t sensor_read[3];
	//citim 1 octet de la senzorul 0x29, registrul 0xC0 -> afisam ce returneaza functia si octetul (asteptat: 0 si EE)
	uint8_t sensor_result1 = i2c_read(0x29, 0xC0, sensor_read, 1);
	printf("rezultat test 1 octet: %d\r\n", sensor_result1);
	printf("octetul citit: %X\r\n", sensor_read[0]);
	//citim 2 octeti de la acelasi registru -> afisam (asteptat: 0 EE AA)
	uint8_t sensor_result2 = i2c_read(0x29, 0xC0, sensor_read, 2);
	printf("rezultat test 2 octeti: %d\r\n", sensor_result2);
	printf("octetii cititi: %X %X\r\n", sensor_read[0], sensor_read[1]);
	//citim 3 octeti de la acelasi registru -> afisam (asteptat: 0 EE AA 10)
	uint8_t sensor_result3 = i2c_read(0x29, 0xC0, sensor_read, 3);
	printf("rezultat test 3 octeti: %d\r\n", sensor_result3);
	printf("octetii cititi: %X %X %X\r\n", sensor_read[0], sensor_read[1], sensor_read[2]);
*/

	//vrem sa transmitem 2 octeti
	tx_header.dlc = 2;
	//NU folosim extended id
	tx_header.ext_id = 0;
	//folosim standard id
	tx_header.ide = 0;
	//setam rtr ul(vrem sa transmitem => 0)
	tx_header.rtr = 0;
	//id ul transmitterului
	tx_header.std_id = 0x244;
	//setam timpul la 0
	tx_header.transmit_global_time = 0;

	//setam valorile pe care vrem sa le transmitem in bufferul de tx
	/*tx_data[0] = 0x01;
	tx_data[1] = 0x02;
	tx_data[2] = 0x03;
	tx_data[3] = 0x04;
	tx_data[4] = 0x05;*/

	uint8_t tx_msg_pack[8] = {0};

	printf("Transmitter ready...\r\n");

	//un rand de ecran are 128 / 6 = 21 de caractere, + 1 pt '\0'
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
		//printf("TSR: 0x%08lX | ESR: 0x%08lX\r\n", CAN1->TSR, CAN1->ESR);

		//citim distanta in mm
		uint16_t dist = vl53l0x_read_range_mm();
		//daca a fost timeout (vl53l0x_timeout_occurred) -> afisam un mesaj de timeout
		/*if(vl53l0x_timeout_occurred() == true) {
			printf("Timeout!\r\n");
		}*/
		/*//altfel afisam distanta
		else {
			printf("Distance: %u mm\r\n", dist);
		}*/

		ssd1306_clear();
		//titlul pe pagina 0: "ECU1 - SENSOR"
		ssd1306_draw_string_centered(0, "ECU1 - SENSOR");
		//pe pagina 3, in functie de distanta:
		//daca a fost timeout       -> "SENSOR TIMEOUT"
		if(vl53l0x_timeout_occurred() == true) {
			ssd1306_draw_string_centered(3, "SENSOR TIMEOUT");
		}
		//altfel daca dist >= 8190  -> "OUT OF RANGE"
		else if(dist >= 8190) {
			ssd1306_draw_string_centered(3, "OUT OF RANGE");
		}
		else {
			snprintf(text, sizeof(text), "DIST: %u mm", dist);
			ssd1306_draw_string_centered(3, text);
		}
		ssd1306_update();

		//punem distanta in mesaj: [0] = octetul mare, [1] = octetul mic (ca la writeReg16Bit)
		tx_msg_pack[0] = (uint8_t)(dist >> 8);
		tx_msg_pack[1] = (uint8_t)(dist >> 0);

		//apelam functia de transmit
		can_add_tx_message(&tx_header, tx_mailbox, &tx_msg_pack[0]);
		//vrem sa transmitem cu delay de 0.05 secunde (50 de milisecunde)
		delay(50);

	}
}
