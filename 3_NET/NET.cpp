#include "NET.h"

volatile uint16_t flCount;
volatile uint8_t flLimit;

volatile uint16_t ecCount0;
volatile uint16_t ecCount1;
volatile uint16_t ecCount2;
volatile uint8_t ecLimit0;
volatile uint8_t ecLimit1;
volatile uint8_t ecLimit2;
uint8_t DLL_ACK;
volatile uint8_t distanceTable[ID_RANGE][ID_RANGE];  // Routing Table Store Distance to each Element


ISR(TIMER2_COMPA_vect){
	
	if(ROUTING == FLOODING){
		flCount++;
		if(flCount == 500){
			flLimit = 1;
			flCount = 0;
			put_str("Next flood ID\n\r");								
		}
	}	
	
	if(ROUTING == DISTVEC){
		ecCount0++;
		ecCount1++;
		ecCount2++;		
		
		if(ecCount0 == 1000)
			if(ID != 0)
				ecLimit0 = 1;			
		
		if(ecCount1 == 1000)
			if(ID != 1)
				ecLimit1 = 1;				
		
		if(ecCount2 == 1000)
			if(ID != 2)
				ecLimit2 = 1;			
	}		
}


void transmit_NET(uint8_t TRANsegment[], uint8_t DESTaddr){
    Packet p;
    static int echoCount = 0;
 
	p.control[0] = 0;                  // Control indicates parity check && send message from TRAN to DLL  
    p.control[1] = 0;                  // Set Second byte of control un-used
    p.SRCadd = ID;                  // Source Address = Address of this Ill Matto
    p.length = NET_SIZE;               // Fixed Length of Packet size
	p.DESTadd = DESTaddr;               // Destination Address passed from Transport Layer
	for(int i = 0; i < TRAN_SIZE; i++)  // Transport Segment passed from Transport Layer
		p.TRANseg[i] = TRANsegment[i];  
     

	if(ROUTING == DISTVEC){
		// if echo count reach 10 iterations, send echo call to all neighbours
		if(echoCount == 0){
			p.control[0] = 2;                    // control indicates parity check && sending echo
			
			put_str("Send ECHO broadcast\n\n\r");

			int i = 1;
				if(ID != i){
					echo(&p,i);	
				}
			}			
			echoCount = 10;                         // Reset echo count
		}

		else{                                      // Transmit message from TRAN to DLL  		
			echoCount--; 
		}
		p.control[0] = 0;                       // control indicates parity check && sending echo
		p.checksum = parCheck(&p);
		//distVec(&p);							// Call Distance Vector Routing Algorithm
  
	
	
	// Calculate Hop Destination Address via pre-determined Routing Algorithm
	if(ROUTING == FLOODING){
		p.control[1] = ID_RANGE - 1; // Set hop limit
		flooding(&p);                // Call Flooding Routing Algorithm
	}            

}



void receive_NET(uint8_t DLLpacket[]){

    Packet p;
    uint8_t NETpacket[NET_SIZE];

    // Decode array to packet
    p.control[0] = DLLpacket[0];
    p.control[1] = DLLpacket[1];
    p.SRCadd = DLLpacket[2];
    p.DESTadd = DLLpacket[3];
    p.length = DLLpacket[4];
    for(int i = 5; i < 5 + TRAN_SIZE; i++)
        p.TRANseg[i-5] = DLLpacket[i];
    p.checksum = DLLpacket[126] + (DLLpacket[127] << 8);     
      

	if(parCheck(&p) == p.checksum){
		
		put_str("\n\rParity Check: PASS\n\r");
		
		if(p.control[0] == 0){                 // Received normal packet     

			if(ROUTING == FLOODING)            // Determine routing type to forward packet
				flooding(&p);
			if(ROUTING == DISTVEC) 
				distVec(&p);                   
			
		}

		// Received echo call, send back echo acknowledgment and distance table
		if(p.control[0] == 2){              
            put_str("\n\rReceived ECHO\n\r");		
			p.control[0] = 4;                 // Set control to parity check, echo acknowledgment and distance table
			p.DESTadd = p.SRCadd;             // Set destination to source of received packet
			p.SRCadd = ID;                 // Source is now ID of this ill matto
			p.length = NET_SIZE;              // Fixed size packet

			// Convert distance table matrix to array and fill TRAN segment
			int k = 0;
			for(int j = 0; j < ID_RANGE; j++){
				for(int i = 0; i < ID_RANGE; i++){
					p.TRANseg[k] = distanceTable[i][j];
					k++;
				}
			}
			
			p.checksum = parCheck(&p);        // Calculate parity check
			
			// Convert packet struct to array and send DLL
			passPacket(&p, p.DESTadd);
		}

		// Received echo acknowledgment and their distance table in TRAN segment
		if(p.control[0] == 4){    
			if(p.DESTadd == ID){
				put_str("\n\rReceived ECHO ACK\n\r");
				echo(&p, p.SRCadd);       // Stop echo timer, calculate distance
				int k = 0;
				for(int j = 0; j < ID_RANGE; j++){
					for(int i = 0; i < ID_RANGE; i++){
						
						// Take average between recieved distance table and current table
						if((p.TRANseg[k] != INF)&&(distanceTable[i][j] != INF))						
							distanceTable[i][j] = (p.TRANseg[k] + distanceTable[i][j])/2;
						else{
							if(p.TRANseg[k] == INF)
								distanceTable[i][j] = distanceTable[i][j];	
							else
								distanceTable[i][j] = p.TRANseg[k];										
						}							
						k++;
					}
				}
			}                    
		}
	}

	else{
		put_str("Parity Check: FAIL\n\r");
		return;
	}

}


// EVEN MULTI-BIT PARITY CHECK
// Split packet into 15 chunks of 8 bytes and 1 chunk of 6 bytes
// Each chunk calculated even parity
int parCheck(Packet* p){
    int evenPar[16];
    int n = p->checksum;
    int setBits = 0;
    int tranBit = 3;
    int limit = 11;

    // Count number of logic high bits in parity chunk (8 bytes)
    setBits = countSetBits(p->control[0]) + countSetBits(p->control[1]);
    setBits += countSetBits(p->SRCadd);
    setBits += countSetBits(p->DESTadd);
    setBits += countSetBits(p->length);
    setBits += countSetBits(p->TRANseg[0]) + countSetBits(p->TRANseg[1]) + countSetBits(p->TRANseg[2]);

    // Determine even parity
    if(setBits%2==0)
        evenPar[0] = 0;
    else
        evenPar[0] = 1;
    setBits = 0;

    for(int parBit = 1; parBit < 16; parBit++){
        
        for(tranBit; tranBit < limit; tranBit++)
            setBits =+ p->TRANseg[tranBit];
        
        if(setBits%2==0)
            evenPar[parBit] = 0;
        else
            evenPar[parBit] = 1;    
        setBits = 0; 
        tranBit =+ 8;
        limit =+ 8;
    }     
    return binToDec(evenPar);
}

uint8_t countSetBits(uint8_t n) 
{ 
    uint8_t result = 0; 
    while (n) { 
        result += n & 1; 
        n >>= 1; 
    } 
    return result; 
} 

int binToDec(int binary[]){
    int dec = 0;
    for(int i = 0; i < 16; i++)
        if(binary[i] == 1)
            dec += pow(2,i);

    return dec;
}



// FLOODING
void flooding(Packet* p){
	put_str("FLOODING\n\r");  
	
    if(p->DESTadd == ID){
        put_str("Destination: FOUND\n\r");
        passPacket(p,INF);
    }
    else{
		put_str("Destination: NOT FOUND\n\r");
		put_str("FORWARD packet:\n\n\r");
        if(p->SRCadd != ID)
            p->control[1]--;                    // Decrement Hop Count if Packet is not sent from this Ill Matto

		flCount = 0;
		flLimit = 0;
        for(uint8_t floodID = 0; floodID < ID_RANGE; floodID++){      // Sent Packet to Every Node
            if((floodID != ID) && (floodID != p->SRCadd)){
				if(flLimit == 1){
					flLimit = 0;					
					floodID++;
				}
				else{
					p->DESTadd = floodID;
					p->checksum = parCheck(p);
					passPacket(p,floodID);				
				}
            }
        }       
    }    
}


// DISTANCE VECTOR ROUTING
void distVec(Packet* p){
    put_str("DISTANCE VECTOR ROUTING\n\r");
	
	char str[30]; 
    uint8_t distance[5];      // Store first row of matrix, updates itself according to new nodes it visits so that particular element will have the least distance
    uint8_t visitedNode[5];   // Give info about nodes visited during algorithm running
    uint8_t prevDists[5];     // Info on previous distances
    uint8_t minDist;          // Store min distance
    uint8_t nextNode = 0;

    uint8_t nextHop;
    uint8_t destination;
    uint8_t numHops;

	if(p->DESTadd == ID){
        put_str("Destination: FOUND\n\r");
        passPacket(p,INF);
    }
	
	else{
		for(uint8_t i = 0; i < ID_RANGE; i++){
			visitedNode[i] = 0;
			prevDists[i] = 0;
			distance[i] = distanceTable[i][0];
		}
		distance[ID] = 0;
		visitedNode[ID] = 1;

		for(uint8_t i = 0; i < ID_RANGE; i++){
			minDist = INF;
			for(uint8_t j = 0; j < ID_RANGE; j++){
				if((minDist > distance[j]) && (visitedNode[j] != 1)){
					minDist = distance[j];
					nextNode = j;
				}
			}

			visitedNode[nextNode] = 1;

			// Start of ALGORITHM
			for(uint8_t j = 0; j < ID_RANGE; j++){
				if(visitedNode[j] != 1){
					if(minDist+distanceTable[nextNode][j] < distance[j]){
						distance[j] = minDist+distanceTable[nextNode][j];
						prevDists[j] = nextNode;
					}
				}
			}
		}
		destination = p->DESTadd;
		nextHop = destination;
		numHops = 0;
		while(nextHop != 0){
			nextHop = prevDists[nextHop];
			numHops++;
		}
		put_str("Number of Hops: ");
		sprintf(str, "%d", numHops);
		put_str(str);
		put_str("\n\r");
		if(numHops == 1)
			passPacket(p, destination); //printf("Next Hop ID: %i\n",destination);
			
		else
			passPacket(p, prevDists[destination]); //printf("Next Hop ID: %i\n",prevDists[destination]);
	}
}


// ECHO
void echo(Packet* p, uint8_t ecID){    
	char str[10];   

    // Send ECHO
    if((p->control[0] == 2) || (p->control[0] == 3)){
		if(ecID == 0){
			ecCount0 = 0;
		}
		if(ecID == 1){
			ecCount1 = 0;
		}
		if(ecID == 2){
			ecCount2 = 0;
		}
        p->DESTadd = ecID;
		p->checksum = parCheck(p);
		passPacket(p, ecID);
    }

    // Recieve ECHO Acknowledgement
    if((p->control[0] == 4) || (p->control[0] == 5)){			
		put_str("\nID: ");
		sprintf(str, "%d", ecID);
		put_str(str);
		put_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\r");			
		
		if(ecID == 0){
			if(ecLimit0 == 1){
				ecLimit0 = 0;
				put_str("Time taken: ");
				sprintf(str, "%d", INF);
				put_str(str);
				put_str("\n\r");
				distanceTable[ID][ecID] = INF;	
				distanceTable[ecID][ID] = INF;	
			}
			else{
				put_str("Time taken: ");
				sprintf(str, "%d", ecCount0);
				put_str(str);
				put_str("\n\r");
				distanceTable[ID][ecID] = ecCount0;	
				distanceTable[ecID][ID] = ecCount0;	
			}				
		}
		if(ecID == 1){
			if(ecLimit1 == 1){
				ecLimit1 = 0;
				put_str("Time taken: ");
				sprintf(str, "%d", INF);
				put_str(str);
				put_str("\n\r");
				distanceTable[ID][ecID] = INF;	
				distanceTable[ecID][ID] = INF;	
			}
			else{
				put_str("Time taken: ");
				sprintf(str, "%d", ecCount1);
				put_str(str);
				put_str("\n\r");
				distanceTable[ID][ecID] = ecCount1;	
				distanceTable[ecID][ID] = ecCount1;	
			}
		}
		if(ecID == 2){
			if(ecLimit2 == 1){
				ecLimit2 = 0;
				put_str("Time taken: ");
				sprintf(str, "%d", INF);
				put_str(str);
				put_str("\n\r");
				distanceTable[ID][ecID] = INF;	
				distanceTable[ecID][ID] = INF;	
			}
			else{
				put_str("Time taken: ");
				sprintf(str, "%d", ecCount2);
				put_str(str);
				put_str("\n\r");
				distanceTable[ID][ecID] = ecCount2;	
				distanceTable[ecID][ID] = ecCount2;	
			}

		}
               
    }
}


void passPacket(Packet* p, uint8_t hopID){
    uint8_t NETpacket[NET_SIZE];
   	char str[40]; 
	
    // Convert packet struct to array
    NETpacket[0] = p->control[0];
    NETpacket[1] = p->control[1];
    NETpacket[2] = p->SRCadd;
    NETpacket[3] = p->DESTadd;
    NETpacket[4] = p->length;
    for(int i = 5; i < TRAN_SIZE + 5; i++)
        NETpacket[i] = p->TRANseg[i];
    NETpacket[126] = p->checksum & 0xff;
    NETpacket[127] = p->checksum >> 8;

    // Send to TRAN or DLL
    if(hopID > ID_RANGE-1){
		transport_layer_receive(p->TRANseg, p->SRCadd);		
    }
    else{
		transmit_DLL(NETpacket, hopID);
    }
}