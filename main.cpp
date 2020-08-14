#include <util/delay.h>
#include "rfm12lib/rfm12.h"
#include "1_PHY/PHY.h"
#include "2_1_MAC/csma.h"
#include "2_2_LLC/LLC.h"
#include "3_NET/NET.h"
#include "4_TRAN/TRAN.h"
#include "5_APP/APP.h"


void setup();

int main() {
	setup();
			

	uint8_t TRANseg[TRAN_SIZE];
	// Test segment from TRAN 
    int DESTadd = 2;                 // Destination Address
    for(int i = 0; i < TRAN_SIZE; i++)
        TRANseg[i] = 0;
	
	TIMSK2 |= _BV(OCIE2A); //Count system time
	transmit_NET(TRANseg, DESTadd);
	put_str("done\n\n\n\n\n\r");
	
	while(1){
		
		//_delay_ms(300000);
		receive_PHY();
		
	}
	TIMSK2 &= ~_BV(OCIE2A);
		//transmit_DLL(net_array, DEST_address);
		//_delay_ms(300000); // delay for readability
	
}

void setup() {
	
	ecCount0 = 0;
	ecCount1 = 0;
	ecCount2 = 0;
	ecLimit0 = 0;
	ecLimit1 = 0;
	ecLimit2 = 0;
	
	init_uart0();
	_delay_ms(100);
	rfm12_init();
	_delay_ms(100);
	sei();
	
	//initialise timer CHANGED TO TIMER 2
	TCCR1A |= 0x00; 
	TCCR1B |= _BV(CS10) | _BV(CS12) | _BV(WGM12); //Pre-scaler and CTC mode
	OCR1A = (uint16_t) (((F_CPU/PRESCALER)/1000)*30); //30ms timer delay
	
		// Timer 2 set-up
	TCCR2A |= _BV(WGM21);                         // Set CTC mode
	TCCR2B |= _BV(CS20) | _BV(CS21) | _BV(CS22);  // Pre-scaler to 1024
	OCR2A = 117;                                  // ~10ms
	
	 // Initialise distance table
    for(int j = 0; j < ID_RANGE; j++){
        for(int i = 0; i < ID_RANGE; i++){
            if((i == ID)&&(j == ID))
                distanceTable[i][j] = 0;
            else
                distanceTable[i][j] = INF;
        }
    }
}

