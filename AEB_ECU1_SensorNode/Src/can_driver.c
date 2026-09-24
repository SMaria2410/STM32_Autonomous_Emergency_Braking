#include "can_driver.h"

void can_gpio_init(void) {
	//enable clock access la GPIOB
	RCC->AHB1ENR |= (1U<<1);
	//setam pinii PB8 si PB9 in AF mode
	//pt PB8: bit 16 = 0, bit 17 = 1
	GPIOB->MODER &=~ (1U<<16);
	GPIOB->MODER |= (1U<<17);
	//pt PB9: bit 18 = 0, bit 19 = 1
	GPIOB->MODER &=~ (1U<<18);
	GPIOB->MODER |= (1U<<19);
	//setam PB8 si PB9 AF = CAN1 RX si TX
	//pt PB8: 1001 ca sa fie AF9
	GPIOB->AFR[1] |= (1U<<0);
	GPIOB->AFR[1] &=~ (1U<<1);
	GPIOB->AFR[1] &=~ (1U<<2);
	GPIOB->AFR[1] |= (1U<<3);
	//pt PB9: 1001 ca sa fie AF9
	GPIOB->AFR[1] |= (1U<<4);
	GPIOB->AFR[1] &=~ (1U<<5);
	GPIOB->AFR[1] &=~ (1U<<6);
	GPIOB->AFR[1] |= (1U<<7);
	/*sau se poate pune direct valoarea 9 incepand cu bitii 0, respectiv 4
	GPIOB->AFR[1] |= (0x09<<0);
	GPIOB->AFR[1] |= (0x09<<4); */
	//enable CAN RX0 interrupt (pt receive de mesaje)
	NVIC_EnableIRQ(CAN1_RX0_IRQn);
}

void can_params_init(uint8_t mode) {
	//enable clock access la CAN1
	RCC->APB1ENR |= (1U<<25);
	//intram in initialization mode
	CAN1->MCR |= (1U<<0);
	//asteptam pana cand CAN1 este in initialization mode
	while(!(CAN1->MSR & (1U<<0))) ; //polling cu busy waiting
	//dam exit din sleep mode
	CAN1->MCR &=~ (1U<<1);
	//asteptam pana cand CAN1 da exit din sleep mode
	while((CAN1->MSR & (1U<<1)) != 0U) ; //polling cu busy waiting
	//configuram parametri de timp inclusiv baudrate ul,
	//configurand time segment 1 si 2 si prescaler ul
	CAN1->BTR = (1<<CAN_BTR_TS1_Pos) | (0<<CAN_BTR_TS2_Pos) | (9<<CAN_BTR_BRP_Pos);
	//selectam mode ul
	if(mode == 1) {
		//normal mode (loopback disabled)
		CAN1->BTR &=~ (1U<<30);
	} else {
		//loopback mode
		CAN1->BTR |= (1U<<30);
	}
}

void can_start(void) {
	//dam exit din initialization mode
	CAN1->MCR &=~ (1U<<0);
	//asteptam pana cand CAN1 da exit din initialization mode
	while((CAN1->MSR & (1U<<0)) != 0U) ; //polling cu busy waiting
	//dam enable interrupt pt FIFO0 message pending
	CAN1->IER |= (1U<<1);
}

uint8_t can_add_tx_message(can_tx_header_typedef *pHeader, uint32_t *pTxMailbox, uint8_t aData[]) {
	//verificam daca avem macar un mailbox de transmit empty
	if(((CAN1->TSR & (1U<<26)) != 0U) || ((CAN1->TSR & (1U<<27)) != 0U) || ((CAN1->TSR & (1U<<28)) != 0U)) {
		//alegem mailbox ul liber din flag ul CODE al TSR
		uint32_t transmitmailbox = (CAN1->TSR & ((1U<<24) | (1U<<25))) >> 24;
		//verificam sa fie un mailbox valid(0, 1 sau 2)
		if(transmitmailbox > 2U) {
			return 1;
		}
		//retinem adresa mailbox ului
		*pTxMailbox = transmitmailbox;
		//daca IDE = 0, setam std_id
		if(pHeader->ide == 0U) {
			CAN1->sTxMailBox[transmitmailbox].TIR = (pHeader->std_id << 21) | (pHeader->rtr << 1);
		} else {
			//daca IDE = 1, setam ext_id
			CAN1->sTxMailBox[transmitmailbox].TIR = (pHeader->ext_id << 3) | (1U<<2) | (pHeader->rtr << 1);
		}
		//setam dlc ul
		CAN1->sTxMailBox[transmitmailbox].TDTR = (pHeader->dlc);
		//activam/dezactivam transmit global time ul
		if(pHeader->transmit_global_time == 1U) {
			CAN1->sTxMailBox[transmitmailbox].TDTR |= (1U<<8);
		}
		//setam datele
		//pt low
		CAN1->sTxMailBox[transmitmailbox].TDLR = ((uint32_t)aData[0]) | ((uint32_t)aData[1] << 8) | ((uint32_t)aData[2] << 16) |  ((uint32_t)aData[3] <<24);
		//pt high
		CAN1->sTxMailBox[transmitmailbox].TDHR = ((uint32_t)aData[4]) | ((uint32_t)aData[5] << 8) | ((uint32_t)aData[6] <<16) | ((uint32_t)aData[7] << 24);
		//setam bitul TXRQ (bitul 0 din TIR) pentru a incepe transmisia
		CAN1->sTxMailBox[transmitmailbox].TIR |= (1U<<0);
		return 0;
	}
	return 1;
}



