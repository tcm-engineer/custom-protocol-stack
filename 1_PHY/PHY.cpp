#include "PHY.h"

uint8_t transmit_PHY(uint8_t *dll_array, uint8_t arrSize){
	//Check to see if frame successfully passed to TX buffer
	
		uint8_t status = rfm12_tx(arrSize, 0, dll_array); //Pass frame to buffer
		char text[4];
		for(int i = 0; i < arrSize; i++){
			sprintf(text, "%.2X", dll_array[i]);
			//put_str(text);
		}
		put_str("\n\r");
		if (status == RFM12_TX_ENQUEUED){
			
			//put_str("frame moved to transfer buffer\n\r");
		}
		else if (status == RFM12_TX_OCCUPIED){
			put_str("transmit buffer full\n\r");
			return 1; //Return status of frame in buffer
		}
		else if (status == RFM12_TX_ERROR){
			put_str("packet longer than transmit buffer\n\r");
			return 1;
		}
		else {
			put_str("ERROR!\n\r");
		}
		csma_p(); //Flow control and transmit data
		return 0; //Return 0 when frame succesfully transmitted
}

uint8_t receive_PHY() {
	//put_str("In Receive:\n\r");
	// code from main.cpp~
	// checks to see if a packet has been received and if so sends to UART
	if (rfm12_rx_status() == STATUS_COMPLETE) { //Packet received
		//put_str("packet received\n\r");
		uint8_t *bufptr;
		uint8_t frame[30];
		bufptr = rfm12_rx_buffer(); //Take RX buffer contents
		char text[4];
		for (uint8_t i = 0; i < rfm12_rx_len(); i++){
			frame[i] = bufptr[i]; //Store RX buffer contents
			sprintf(text, "%.2X", bufptr[i]);
			//put_str(text);
		}
		put_str("\n\r");
		rfm12_rx_clear(); //Clear RX Buffer	
		return receive_DLL(frame); //Send received frame to DLL
		
	}
}