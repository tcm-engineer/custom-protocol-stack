#include "LLC.h"

uint8_t last_frame = 0; //Last frame recevied by DLL
volatile uint8_t resend[8]; //Store which frames have been re-sent
volatile uint8_t ACK_frames; //Store which frames have been acknowledged
extern volatile uint8_t flLimit;
extern volatile uint8_t ecLimit0;
extern volatile uint8_t ecLimit1;
extern volatile uint8_t ecLimit2;
extern uint8_t DLL_ACK;

ISR(TIMER1_COMPA_vect) { //Once Go-Back-N timeout runs out, checks whch frames have been acknowledged and then set resend frames not been acknowledged

	 for(int i = ACK_frames; i<8;i++){
			resend[i] = 1;
		}
		
	return;
}

void transmit_DLL(uint8_t *net_array, uint8_t DEST_address) {
	
//-----------------------FRAMING--------------------------//	
	
	put_str("In Transmit: \n\r");
	
	
	Frame f;
	uint8_t dll_buffers[7][FRAME_SIZE];
	uint8_t arrSize;
	uint8_t ACK;
	
	f.header = 0x7E;
	f.control[0] = 255; //Send: data = 11111111 ACK = 00000000 
    f.SRC_address = ID; //GET VALUE FROM TOM
    f.DEST_address = DEST_address; //GET VALUE FROM TOM
    f.length = DATA_SIZE; //00010011 Fixed
    f.footer = 0x7E; //Stop frame flag  01111110
	
	 for(uint8_t i = 0; i<7;i++){
		 
        f.control[1] = i+1; //Frame number 1,2,3....
        
        for(uint8_t j = 0; j<DATA_SIZE; j++){
			
            f.data[j] = net_array[(DATA_SIZE*i)+j];
        }
        check_sum(&f); //Calculate checksum and set in Frame
//--------------Put into buffer array-----------------//		
		dll_buffers[i][0] = f.header;
		dll_buffers[i][1] = f.control[0];
		dll_buffers[i][2] = f.control[1];
		dll_buffers[i][3] = f.SRC_address;
		dll_buffers[i][4] = f.DEST_address;
		dll_buffers[i][5] = f.length;
		for (int k = 0; k < DATA_SIZE; k++){
			
			dll_buffers[i][6+k] = f.data[k];
			
		}
		dll_buffers[i][27] = f.checksum[0];
	
		dll_buffers[i][28] = f.checksum[1];
		dll_buffers[i][29] = f.footer;
		
		char text[40];
		for(int k = 0; k < FRAME_SIZE; k++){	
		sprintf(text, "%.2X", dll_buffers[i][k]);
		//put_str(text);
		}
		put_str("\r\n");
		
		arrSize = sizeof(dll_buffers[i]);
		while(transmit_PHY(dll_buffers[i], arrSize));
		
	}
	
	TIMSK1 |= _BV(OCIE1A); // start timer if timer runs out, interrupt code runs
	while(ACK_frames != 7){ //Listen for ACK's
		if(flLimit == 1 || ecLimit0 == 1 || ecLimit1 == 1 || ecLimit2 == 1){
			flLimit = 0;
			ecLimit0 = 0;
			ecLimit1 = 0;
			ecLimit2 = 0;
			last_frame = 0;
			//put_str("BREAK\n\n\n\r");
			return;
		}
		ACK = receive_PHY();
		char text[40];
		sprintf(text, "ACK = %d\n\r", ACK);
		//put_str(text);
		if (ACK != 0){
			ACK_frames = ACK;
			for(int k = 0; k<ACK_frames;k++){
				resend[k] = 0;
			}
		}		
		
		//sprintf(text, "Transmit_dll ACK'd frame: %d", ACK_frames);
		//put_str(text);
		//put_str("\n\r");
		for(int j=0; j<7;j++){
			if (resend[j] == 1) {
				arrSize = sizeof(dll_buffers[j]);
				while(transmit_PHY(dll_buffers[j], arrSize));
			}
	
		}
	}
	
	put_str("DLL - All Frames Acknowledged!\n\r");
	ACK_frames = 0;
	DLL_ACK = 1; //Let NET know DLL received all ACK's
	TIMSK1 &= ~_BV(OCIE1A);//if all acks arrived, diasble timer set ocie1a to 0
}

uint8_t receive_DLL(uint8_t* recv_frame){
   
    //put_str("In Receive_dll:\n\r");
	
	char text[4];
	//for (uint8_t i = 0; i < FRAME_SIZE; i++){
		//sprintf(text, "%.2X", recv_frame[i]);
		//put_str(text);
	//}
	//put_str("\n\r");
   
   
	Frame f;
	
    static uint8_t net_array[NET_SIZE];
    uint8_t ACK_array[FRAME_SIZE];
    uint8_t arrSize;
	uint8_t frame_ACK;
    
    f.header = recv_frame[0];
	if(f.header == 0x7E){
        f.control[0] = recv_frame[1];
        f.control[1] = recv_frame[2];
        f.SRC_address = recv_frame[3];
        f.DEST_address = recv_frame[4];
        f.length = recv_frame[5];
        for(uint8_t i = 0; i<FRAME_SIZE;i++){
            f.data[i] = recv_frame[i+6];
        }  

        f.footer = recv_frame[29];
        
        if (f.DEST_address != ID){ //Check if frame for this IlMatto
            //put_str("DLL - Frame not for this IlMatto \n\r");
            return 0;
        } else {
            //put_str("DLL - Frame for this IlMatto\n\r");
            check_sum(&f); //Calculate checksum of received frame
            if (f.checksum[0] == recv_frame[27] && f.checksum[1] == recv_frame[28]){ //Check checksums match
                //put_str("DLL - Checksums are the same\n\r");
        
                if (f.control[0] == 0){ //Check if ACK frame
                    //put_str("DLL - Frame Received is ACK\n\r");
                    
					
					char text[4];
					sprintf(text, "DLL - ACK of Frame: %d", recv_frame[2]);
					//put_str(text);
					put_str("\n\r");
					return recv_frame[2];
                } else { //Frame is DATA
                    //put_str("DLL - Frame Received is DATA\n\r");
                    
					if (last_frame + 1 == f.control[1]){
						++last_frame;
						 //Send ACK
						Frame ACK;
                    
						ACK.header = 0x7E;
						ACK.control[0] = 0;
						ACK.control[1] = f.control[1];
						ACK.SRC_address = ID;
						ACK.DEST_address = f.SRC_address;
						ACK.length = DATA_SIZE;
						for(uint8_t i = 0; i<21;i++){
							ACK.data[i] = 0;
						}
						check_sum(&ACK);
						ACK.footer = 0x7E;
						
						ACK_array[0] = ACK.header;
						ACK_array[1] = ACK.control[0];
						ACK_array[2] = ACK.control[1];
						ACK_array[3] = ACK.SRC_address;
						ACK_array[4] = ACK.DEST_address;
						ACK_array[5] = ACK.length;
						for (int k = 0; k < 21; k++){
				
							ACK_array[6+k] = ACK.data[k];
				
						}
						ACK_array[27] = ACK.checksum[0];
			
						ACK_array[28] = ACK.checksum[1];
						ACK_array[29] = ACK.footer;
			
						arrSize = sizeof(ACK_array);
						while(transmit_PHY(ACK_array, arrSize));
						//put_str("ACK sent\n\r");
						
                    
						for(uint8_t i = 0; i<f.length; i++){
							if(f.control[1] == 1){
								net_array[i] = f.data[i];
							} else {
								net_array[(DATA_SIZE*(f.control[1]-1))+i] = f.data[i]; //Fill net_packet array with netork payload from frame
							}
	
								
							
						}
						
						
						if(f.control[1] == 7 && last_frame == 7){ //Check if last frame in packet and all 6 frames are correct (GO-BACK-N)
							last_frame = 0;
							put_str("DLL - Passed Packet to Network layer\n\r");
							
							for(int k = 0; k<NET_SIZE; k++){	
								sprintf(text, "%.2X", net_array[k]);
								put_str(text);
							}
							put_str("\r\n");
							
							receive_NET(net_array); //Pass network packet to network layer
						}
					}
                   return 0;
                }
                
            } else {
                //put_str(" DLL - Checksums not the same\n");
				return 0;
                //Don't send ACK
            }
            
        }
	}else{
		//put_str("DLL - Received Frame not got header 0x7E");
	}
}

void check_sum(Frame* f) { //Calculate sum of nibbles of each byte in checksum
    
    uint16_t sum = 0; //2 BYTE SUM
    uint8_t nibbles[52]; //52 nibbles in frame (includes: control[1], control[2], SRC_address, DEST_address, Length, 21bytes of DATA)

    nibbles[0] = f->control[0] & 0x0F; //Get Least Significant nibble of Byte
    nibbles[1] = ((f->control[0] & 0xF0) >> 4); //Get Most Significant nibble of Byte
    nibbles[2] = f->control[1] & 0x0F;
    nibbles[3] = ((f->control[1] & 0xF0) >> 4);
    nibbles[4] = f->SRC_address & 0x0F;
    nibbles[5] = ((f->SRC_address & 0xF0) >> 4);
    nibbles[6] = f->DEST_address & 0x0F;
    nibbles[7] = ((f->DEST_address & 0xF0) >> 4);
    nibbles[8] = f->length & 0x0F;
    nibbles[9] = ((f->length & 0xF0) >> 4);
    int j = 0;
    for (int i = 10; i<52;i++){
        nibbles[i] = f->data[j] & 0x0F;
        i++;
        nibbles[i] = ((f->data[j] & 0xF0) >> 4);
        j++;
    }
    
    for (int i = 0;i<52;i++){
        
        sum = sum + nibbles[i]; //Add all nibbles together and store in sum
    }
    
    f->checksum[0] = sum & 0xff; //Allocate Least Significant BYTE of sum to BYTE 0 of checksum 
    f->checksum[1] =(sum >> 8);	//Allocate MOST Significant BYTE of sum to BYTE 1 of checksum 
    
    return;
    
}