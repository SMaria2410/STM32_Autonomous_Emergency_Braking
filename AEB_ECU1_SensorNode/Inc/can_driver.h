#ifndef CAN_DRIVER_H_
#define CAN_DRIVER_H_

#include "stm32f4xx.h"
#include "stm32f446xx.h"

void can_gpio_init(void);
void can_params_init(uint8_t mode);
void can_start(void);

typedef struct {
	//standard id
	uint32_t std_id;
	//extended id
	uint32_t ext_id;
	//tipul de id pt mesajul care va fi transmis(0 pt std_id, 1 pt ext_id)
	uint32_t ide;
	//0 pt transmit, 1 pt request
	uint32_t rtr;
	//lungimea frame ului care va fi transmis
	uint32_t dlc;
	//enable ul pt timestamp
	uint8_t transmit_global_time;
}can_tx_header_typedef;

typedef struct {
	//standard id
	uint32_t std_id;
	//extended id
	uint32_t ext_id;
	//tipul de id pt mesajul care va fi receptionat
	uint32_t ide;
	//0 pt transmit, 1 pt request
	uint32_t rtr;
	//lungimea frame ului care va fi receptionat
	uint32_t dlc;
	//timestamp ul lui SOF
	uint32_t timestamp;
	//indexul filtrului
	uint32_t filter_match_index;
}can_rx_header_typedef;

uint8_t can_add_tx_message(can_tx_header_typedef *pHeader, uint32_t *pTxMailbox, uint8_t aData[]);
uint8_t can_get_rx_message(can_rx_header_typedef *pHeader, uint32_t RxFifo, uint8_t aData[]);

#endif
