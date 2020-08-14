#ifndef LLC_H
#define LLC_H

#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "../PHY/PHY.h"
#include "../NET/NET.h"

#define NET_SIZE 128
#define DATA_SIZE 21
#define FRAME_SIZE 30
#define F_CPU 12000000
#define PRESCALER 1024


typedef struct Frame
{
    uint8_t header;
    uint8_t control[2]; 
    uint8_t SRC_address;
    uint8_t DEST_address;
    uint8_t length;
    uint8_t data[DATA_SIZE];
    uint8_t checksum[2];
    uint8_t footer;
    
}Frame;

extern uint8_t last_frame; //Last frame recevied by DLL
extern volatile uint8_t resend[8]; //Store which frames have been re-sent
extern volatile uint8_t ACK_frames; //Store which frames have been acknowledged


void transmit_DLL(uint8_t *net_array, uint8_t DEST_address);
uint8_t receive_DLL(uint8_t *recv_frame);
void check_sum(Frame* f);

#endif