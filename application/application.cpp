#include "application.h"
#include "../5_APP/APP.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "../rfm12lib/uart.h"

#define LED_ON PORTC |= (1<<PORTC0)
#define LED_OFF PORTC &= ~(1<<PORTC0)
#if (NODE_ID == NODE_ID_1)
#define LED_ON_1 PORTC |= (1<<PORTC1)
#define LED_OFF_1 PORTC &= ~(1<<PORTC1)
#endif

uint8_t button_interrupt_flag;
#if NODE_ID != NODE_ID_1
uint8_t switch_counter;
#endif


void set_interrupt_flag(void){
  button_interrupt_flag = 1;
}
void reset_interrupt_flag(void){
  button_interrupt_flag = 0;
}

int interrupt_flag_status(void){
  return button_interrupt_flag;
}

int switch_count_val(void){
  return switch_counter;
}

#if NODE_ID != NODE_ID_1
void increment_switch_counter(void){
  switch_counter ++;
}
void reset_switch_counter(void){
  switch_counter = 0;
}
#endif

uint8_t dummy; //For testing

void interrupt_init(void){
  EIMSK &= ~(1<<0); // INT0 Disable
  EICRA |= (1<<1); // Define interrupt INT0 as falling edge
  EICRA &= ~(1<<0); // Define interrupt INT0 as falling edge
  //EIFR &= ~(1<<0); // Clear interrupt flag 0s
  EIMSK |= (1<<0); // INT0 Enable
  return;
}

void init_app(void){
  reset_interrupt_flag();
  dummy = 0;
  DDRD &= ~(1<<2); //PIND2 input
  PORTD |= (1<<2); //Enable PIND2 Pullup

  //LED PIN-TOGGLE
  DDRC |= (1<<0); //C0
  LED_OFF;
  #if (NODE_ID == NODE_ID_1)
  //Extra LED;
  DDRC |= (1<<1); //C1
  LED_OFF_1;
  #endif

  interrupt_init();

  switch_counter = 0;
}

void event_received(uint8_t button, uint8_t button_count){
  #if (NODE_ID != NODE_ID_1)
  if (button_count == 1){
    LED_ON;
  }
  else if (button_count == 3){
    //reset
    LED_OFF;
    switch_counter = 0;
  }

  #else //node 1
  if (button_count == 2 && button = switch_2){
    LED_ON;
  }
  else if (button_count == 2 && button = switch_2){
    LED_ON_1;
  }
  else if (button_count == 3){
    //reset
    LED_OFF;
    LED_OFF_1;
  }
  #endif
}

ISR(INT0_vect){ //Pin PD2!
  set_interrupt_flag();
  if(dummy == 0){
    //put_str("Led_on");
    
    dummy++;
    //LED_ON;
  }
  else{
    //LED_OFF;
    dummy = 0;
    //put_str("Led_off");
  }
}
