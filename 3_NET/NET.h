#ifndef NET_H
#define NET_H

#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include "../LLC/LLC.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>
#include "../rfm12.h"


// CONSTANT DEFINITIONS
#define TRAN_SIZE 121 // Size of Transport Segment
#define NET_SIZE 128  // Size of Network Packet
#define INF 255       // Infinity
#define FLOODING 0    // Flooding Routing = 0
#define DISTVEC 1     // Distance Vector Routing = 1

// VARIABLES
#define ID 0        // This Ill Matto ID
#define ID_RANGE 2    // Number of Ill Mattos in Network
#define ROUTING 1     // Routing Mode: 0 = Flooding, 1 = Distance Vector 


// Packet Structure
typedef struct Packets{
    uint8_t control[2];         // control[1] = Hop Count, control[0] = check type and routing control
    uint8_t SRCadd;             // Source Address
    uint8_t DESTadd;            // Destination Address
    uint8_t length;             // Length of Packet
    uint8_t TRANseg[TRAN_SIZE]; // 121-bit Transport Segment
    uint16_t checksum;          // Checksum
}Packet;


// Network Layer Transmit and Receive
void transmit_NET(uint8_t TRANseg[], uint8_t DESTaddr);
void receive_NET(uint8_t DLLpacket[]);

// Routing
void flooding(Packet* p);
void distVec(Packet* p);

void echo(Packet* p, uint8_t ecID);
//void initialDists();

// Even Multiple-Bit Parity Check
int parCheck(Packet* p);                   
uint8_t countSetBits(uint8_t n); // Count number of bits high in binary conversion of integer
int binToDec(int binary[]);                // Convert binary to decimal


// Format packet and send to DLL or TRAN layers
void passPacket(Packet* p, uint8_t hopID);

void setup();

extern volatile uint16_t flCount;
extern volatile uint8_t flLimit;
extern uint8_t DLL_ACK;
extern volatile uint16_t ecCount0;
extern volatile uint16_t ecCount1;
extern volatile uint16_t ecCount2;
extern volatile uint8_t ecLimit0;
extern volatile uint8_t ecLimit1;
extern volatile uint8_t ecLimit2;

extern volatile uint8_t distanceTable[ID_RANGE][ID_RANGE];  // Routing Table Store Distance to each Element

#endif