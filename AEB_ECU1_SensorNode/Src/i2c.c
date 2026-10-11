#include "i2c.h"
#include "timebase.h"

//define pt timeout ul de asteptare a unui flag (10 ms)
#define I2C_TIMEOUT_MS 10U

//variabila volatila in care citim SR1 si SR2
static volatile uint32_t addr_var;

//functie de initializare pt i2c
void i2c_init(void) {
	//enable clock access la GPIOB
	RCC->AHB1ENR |= (1U<<1);
	//setam PB6 (SCL) si PB7 (SDA) in AF mode (MODER)
	GPIOB->MODER &=~ (0xFU<<12); //clear
	GPIOB->MODER |= (1U<<13) | (1U<<15); //set la bitii 13 si 15
	//setam PB6 si PB7 ca open drain (OTYPER)
	GPIOB->OTYPER |= (3U<<6);
	//activam pull up pe PB6 si PB7 (PUPDR)
	GPIOB->PUPDR &=~ (0xFU<<12); //clear
	GPIOB->PUPDR |= (1U<<12) | (1U<<14); //set la bitii 12 si 14
	//setam AF4 = I2C1 pe PB6 si PB7 (AFR[0]) -> SET AND CLEAR, ca la CAN
	GPIOB->AFR[0] &=~ (0xFFU<<24); //clear
	GPIOB->AFR[0] |= (1U<<26) | (1U<<30);
	//enable clock access la I2C1 (APB1ENR)
	RCC->APB1ENR |= (1U<<21);
	//dam reset software la I2C1
	I2C1->CR1 |= (1U<<15);
	I2C1->CR1 &=~ (1U<<15);
	//scriem frecventa APB1 in MHz in campul FREQ din CR2
	I2C1->CR2 |= (16U<<0);
	//configuram CCR pt standard mode 100 kHz: CCR = Fpclk1 / (2 * 100 kHz)
	I2C1->CCR |= (0x50U<<0);
	//configuram TRISE pt standard mode: TRISE = Fpclk1 in MHz + 1
	I2C1->TRISE &=~ (0x3FU<<0); //clear(TRISE dupa reset e 0x02)
	I2C1->TRISE |= (0x11U<<0); //set
	//pornim perifericul (PE din CR1)
	I2C1->CR1 |= (1U<<0);
}

//functie pentru asteptare bus liber
static uint8_t free_bus_waiting(void) {
	//retinem tick ul de start
	uint32_t start_tick = get_tick();
	//cat timp BUSY din SR2 e 1(nu e libera magistrala)
	while((I2C1->SR2 & (1U<<1)) != 0) {
		//daca a trecut timeout ul -> return 1
		if((get_tick() - start_tick) > I2C_TIMEOUT_MS) {
			return 1;
		}
	}
	return 0;
}

//functie pentru asteptare flag din SR1
//primeste ca parametru masca flag ului (SB, ADDR, TXE, RXNE sau BTF)
static uint8_t status_flag_waiting(uint32_t flag_mask) {
	//retinem tick ul de start
	uint32_t start_tick = get_tick();
	//cat timp flag ul din SR1 e 0(operatia inca nu s a intamplat)
	while((I2C1->SR1 & flag_mask) == 0U) {
		//daca AF din SR1 e 1 (slave ul nu a dat ACK)
		if((I2C1->SR1 & (1U<<10)) != 0) {
			//generam stop
			I2C1->CR1 |= (1U<<9);
			//stergem AF(ack failure) din status register 1
			I2C1->SR1 &=~ (1U<<10);
			//returnam 1 pt eroare
			return 1;
		}
		//daca a trecut timeout ul
		if((get_tick() - start_tick) > I2C_TIMEOUT_MS) {
			//generam STOP
			I2C1->CR1 |= (1U<<9);
			//returnam 1 pt eroare
			return 1;
		}
	}
	return 0;
}

//functie pentru stergerea ADDR
static void addr_clear(void) {
	//ADDR se sterge citind SR1 si apoi SR2 (citim intr o variabila volatile)
	addr_var = I2C1->SR1;
	addr_var = I2C1->SR2;
}

//functia de write
uint8_t i2c_write(uint8_t sensor_addr, uint8_t sensor_write_reg, const uint8_t *data, uint8_t n) {
	//asteptam bus liber
	//while((I2C1->SR2 & (1U<<1)) != 0) { }
	if(free_bus_waiting() == 1) {
		return 1;
	}
	//generam START (CR1) si asteptam start bit din SR1
	I2C1->CR1 |= (1U<<8);
	if(status_flag_waiting(1U<<0) == 1) {
		return 1;
	}
	//scriem in DR adresa slave ului shiftata cu 1 la stanga (bitul 0 = 0 => write)
	I2C1->DR = (sensor_addr<<1);
	//asteptam ADDR, apoi il stergem
	//while((I2C1->SR1 & (1U<<1)) == 0) { }
	if(status_flag_waiting(1U<<1) == 1) {
		return 1;
	}
	addr_clear();
	//asteptam TXE si scriem in DR adresa registrului
	//while((I2C1->SR1 & (1U<<7)) == 0) { }
	if(status_flag_waiting(1U<<7) == 1) {
		return 1;
	}
	I2C1->DR = (sensor_write_reg << 0);
	//pt fiecare octet de date: asteptam TXE, scriem octetul in DR
	for(int i = 0; i < n; i++) {
		//while((I2C1->SR1 & (1U<<7)) == 0) {}
		if(status_flag_waiting(1U<<7) == 1) {
			return 1;
		}
		I2C1->DR = (data[i] << 0);
	}
	//asteptam BTF (ultimul octet a iesit complet de pe bus)
	if(status_flag_waiting(1U<<2) == 1) {
		return 1;
	}
	//generam STOP
	I2C1->CR1 |= (1U<<9);
	return 0;
}


//functia de read

//--- partea 1: ii spunem senzorului de la ce registru citim (ca la write) ---
//asteptam bus liber
//generam START si asteptam SB
//DR = adresa shiftata cu 1 la stanga (write), asteptam ADDR, il stergem
//asteptam TXE, DR = adresa registrului
//asteptam BTF

//--- partea 2: repeated START si adresa pt citire ---
//generam START din nou (fara STOP inainte) si asteptam SB
//DR = adresa shiftata cu 1 la stanga, cu bitul 0 = 1 (read)
//asteptam ADDR, dar NU il stergem inca -> ordinea depinde de n

//--- partea 3: citirea, in functie de n ---
//cazul n == 1:
//    ACK = 0 (singurul octet primeste NACK)
//    stergem ADDR
//    generam STOP
//    asteptam RXNE, citim DR in data[0]

//cazul n == 2:
//    ACK = 0 si POS = 1 (NACK ul se aplica octetului urmator, adica al 2 lea)
//    stergem ADDR
//    asteptam BTF (octetul 1 e in DR, octetul 2 in shift register)
//    generam STOP
//    citim DR de doua ori (data[0], data[1])
//    POS = 0 (il readucem la normal)

//cazul n > 2:
//    ACK = 1
//    stergem ADDR
//    cat timp mai sunt mai mult de 3 octeti de citit: asteptam RXNE, citim DR
//    cand raman 3 octeti: asteptam BTF
//    ACK = 0
//    citim DR (octetul n-2)
//    asteptam BTF
//    generam STOP
//    citim DR (octetul n-1)
//    asteptam RXNE, citim DR (ultimul octet)

//return 0
