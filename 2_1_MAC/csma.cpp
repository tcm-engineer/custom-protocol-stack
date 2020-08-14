#include "CSMA.h"

uint16_t csma_slot_length = 2;
uint8_t csma_probability = 70;

void transmit_data() {
	RFM12_INT_OFF();
	rfm12_data(RFM12_CMD_PWRMGT | PWRMGT_DEFAULT );
	//calculate number of bytes to be sent by ISR
	//2 sync bytes + len byte + type byte + checksum + message length + 1 dummy byte
	ctrl.num_bytes = rf_tx_buffer.len + 6;
	//reset byte sent counter
	ctrl.bytecount = 0;
	//set mode for interrupt handler
	ctrl.rfm12_state = STATE_TX;	
	//fill 2byte 0xAA preamble into data register
	//the preamble helps the receivers AFC circuit to lock onto the exact frequency
	//(hint: the tx FIFO [if el is enabled] is two staged, so we can safely write 2 bytes before starting)
	rfm12_data(RFM12_CMD_TX | PREAMBLE);
	rfm12_data(RFM12_CMD_TX | PREAMBLE);					
	rfm12_data(RFM12_CMD_PWRMGT | PWRMGT_DEFAULT | RFM12_PWRMGT_ET);
	//enable the interrupt to continue the transmission
	RFM12_INT_ON();
}

void csma_p() {
	srand(68); //Seed for random nuber
	uint16_t tran_status = 0; // current state of the transceiver
	if (ctrl.txstate == STATUS_OCCUPIED) { // data ready to transmit
		uint8_t sent = 0; //If message sent or not
		while (!sent) { // while the packet has not been sent
			_delay_ms(csma_slot_length); // time slot length
			// check channel state
			RFM12_INT_OFF();
			tran_status = rfm12_read(RFM12_CMD_STATUS);
			RFM12_INT_ON();
			if (ctrl.rfm12_state == STATE_RX_IDLE) { // if radio is idle
				if (!(tran_status & RFM12_STATUS_RSSI)) { // if channel is idle
					if (rand() % 100 < csma_probability) { // probability of transmitting
						sent = 1;
						transmit_data();
					}
				}
			}
		}
	}
}