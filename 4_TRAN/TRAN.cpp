#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "TRAN.h"
#include "../rfm12lib/uart.h"


//#define CRYSTAL_FREQUENCY 12000000 //12 MHz
uint16_t expand_timer_counter;
struct Status status;
//TIMER
void start_timer(uint8_t compare_value){
  cli();
  TCNT0 = 0x00; //Reset status.timer_counter register
  OCR0A = compare_value; // compare_value/(12000000/1024)
  //Start timer by setting Set 1024 prescaler
  TCCR0B |= (1<<CS02) | (1<<CS00);
  TCCR0B &= ~(1 << CS01);
  sei();
  expand_timer_counter = 300;
}

void stop_timer(void){
  cli();
  TCCR0B &= ~(1<<CS02);
  TCCR0B &= ~(1<<CS01);
  TCCR0B &= ~(1<<CS00);
  sei();
  status.timer_counter = 0;
  expand_timer_counter = 0;
}


void init_timer(void){
  TCCR0B &= ~(1<<CS02);
  TCCR0B &= ~(1<<CS01);
  TCCR0B &= ~(1<<CS00);

  //Set CTC mode, Timer resets automatcally
  TCCR0A |= (1<<WGM01);
  TCCR0A &= ~(1<<WGM00);
  TCCR0B &= ~(1<<WGM02);

  // Normal port operation, OC0A disconnected.
  TCCR0A &= (0<<COM0A1);
  TCCR0A &= (0<<COM0A0);

  //Enable Compare Match
  TIMSK0 |= (1<<OCIE0A);

}

void init_transport_layer(void){
  status.timer_counter = 0;
  status.NACK_counter = 0;
  status.state = IDLE;
  status.connect_id = -1;
  status.number_of_data_packages = 0;
  init_timer();

}

void reset_layer(void){
  status.timer_counter = 0;
  status.NACK_counter = 0;
  status.state = IDLE;
  status.connect_id = -1;
  status.number_of_data_packages = 0;
}



uint16_t Fletcher16( uint8_t *data, int count )
{
   uint16_t sum1 = 0;
   uint16_t sum2 = 0;
   int index;

   for ( index = 0; index < count; ++index )
   {
      sum1 = (sum1 + data[index]) % 255;
      sum2 = (sum2 + sum1) % 255;
   }

   return (sum2 << 8) | sum1;
}



void add_checksum(uint8_t segment[]){
  uint16_t checksum = Fletcher16(segment, APPDATA_SIZE+ HEADER_SIZE-2); //TA_SIZE+ HEADER_SIZE-2 all elements except checksum
  segment[CHECKSUM] = (uint8_t) (checksum>>8); //sum_2
  segment[CHECKSUM + 1] = (uint8_t) checksum; //sum_1
}

void connect_to_node(uint8_t connect_segment[]){
  //uint8_t connect_segment[APPDATA_SIZE + HEADER_SIZE];
  connect_segment[CONTROL] = (CONNECTION_REQ<<4);
  connect_segment[CONTROL + 1] = 0;
  connect_segment[SRC_PORT] = 0;
  connect_segment[DEST_PORT] = 0;
  connect_segment[LENGTH] = APPDATA_SIZE + HEADER_SIZE;
  for (int i = APP_DATA; i< APPDATA_SIZE + APP_DATA; i++){
    connect_segment[i] = 0;
  }
  add_checksum(connect_segment);

}

void end_con(void){
  status.timer_counter = 0;
  status.NACK_counter = 0;
  status.state = IDLE;
  status.connect_id = -1;
  if(!status.number_of_data_packages) return;
  else if(status.number_of_data_packages == 1){
    status.number_of_data_packages--;
    return;
  }
  else {
    for(int i = APPDATA_SIZE+1; i < (status.number_of_data_packages*(APPDATA_SIZE+1)); i++){
      status.transmitt_buffer[i-(APPDATA_SIZE+1)] = status.transmitt_buffer[i];
    }
    status.number_of_data_packages--;

    //SEND DATA (As long as buffer isn't empty)
    uint8_t segment[HEADER_SIZE+APPDATA_SIZE];
    if(status.transmitt_buffer[0]<3){
      //put_str("Buffer not empty");
      status.state = AEP;
      status.connect_id = status.transmitt_buffer[0];
      connect_to_node(segment);
      transmit_NET(segment, status.transmitt_buffer[0]);
      start_timer(255); //10 is approx 1 ms!
      return;
    }
    else{ //Error...
      reset_layer();
    }
  }
}

void create_data_segment(uint8_t buffer[], uint8_t segment[]){
  segment[CONTROL] = 0x00; //Message containing data, no crc
  #if !USE_SEQUENCE_NUMBER
  segment[CONTROL] = 0x00; //No sequence number, assuming app_data < 114
  #else
  //ADD code for sequence_number here!
  #endif
  segment[SRC_PORT] = 0x00;
  segment[DEST_PORT] = 0x00;
  segment[LENGTH] = APPDATA_SIZE + HEADER_SIZE;
  for (int i = APP_DATA; i< APPDATA_SIZE + APP_DATA; i++){
    segment[i] = buffer[i-APP_DATA+1]; //APP_DATA byte is the 5th, +1 to skip the ID...
  }
  add_checksum(segment);
}

void create_dummy_segment(uint8_t segment[]){
  for (int i = 0; i< APPDATA_SIZE + HEADER_SIZE;i++){
    segment[i] = 0x00;
  }
}
int verify_checksum(uint8_t segment[]){
  uint16_t checksum = Fletcher16(segment, APPDATA_SIZE+ HEADER_SIZE-2);
  return segment[CHECKSUM] == (uint8_t) (checksum>>8) && segment[CHECKSUM + 1] == (uint8_t) checksum;
}

void transport_layer_receive(uint8_t segment[], uint8_t src_ID){

  if (verify_checksum(segment)){

    uint8_t answer_segment[APPDATA_SIZE + HEADER_SIZE];
    switch(status.state){
      case IDLE: //Not connected; Only need to handle connection requests;
        if(((segment[0]&0x70)>>4) == CONNECTION_REQ){ //Connection request
          status.connect_id = src_ID;
          status.state = PEP;
          create_dummy_segment(answer_segment);
          answer_segment[CONTROL] = (ACK<<4);
          add_checksum(answer_segment);
          transmit_NET(answer_segment, src_ID);
          start_timer(255);
        }
        break;
      case PEP:
        if(((segment[0]&0x70)>>4) == DATA_MESSAGE){
          stop_timer();
          status.state = SERVER_CONNECTED;
          create_dummy_segment(answer_segment);
          answer_segment[CONTROL] = (ACK<<4);
          add_checksum(answer_segment);
          for(int i = 0;i <APPDATA_SIZE;i ++){
            status.receive_buffer[i] = segment[i+APP_DATA];
          }
          transmit_NET(answer_segment, src_ID);
          start_timer(255);
        }
        break;
      case AEP:

        if(((segment[0]&0x70)>>4) == ACK){
          stop_timer();
          status.state = CLIENT_CONNECTED;
          status.NACK_counter = 0; //Reset when status.state is switched...
          create_data_segment(status.transmitt_buffer, answer_segment);
          answer_segment[CONTROL] = (DATA_MESSAGE<<4);
          add_checksum(answer_segment);
          transmit_NET(answer_segment, src_ID);
          start_timer(255);
        }
        else if(((segment[0]&0x70)>>4) == NACK){
          stop_timer();
          if(status.NACK_counter ++ > 10){
            end_con();
            return;
          }
          create_dummy_segment(answer_segment);
          answer_segment[CONTROL] = (CONNECTION_REQ<<4); //Try again; counter!
          add_checksum(answer_segment);
          transmit_NET(answer_segment, src_ID);
          start_timer(255);
        }
        break;
      case SERVER_CONNECTED:

        if(((segment[0]&0x70)>>4) == DATA_MESSAGE){
          stop_timer();
          create_dummy_segment(answer_segment);
          answer_segment[CONTROL] = (ACK<<4);
          add_checksum(answer_segment);
          for(int i = 0;i <APPDATA_SIZE;i ++){
            status.receive_buffer[i] = segment[i+APP_DATA];
          }
          transmit_NET(answer_segment, src_ID);
          start_timer(255);
        }
        else if(((segment[0]&0x70)>>4) == DISCONNECT_REQ){
          stop_timer();
          status.state = PDP;
          create_dummy_segment(answer_segment);
          answer_segment[CONTROL] = (DISCONNECT_REQ<<4);
          add_checksum(answer_segment);
          transmit_NET(answer_segment, src_ID);
          start_timer(255);
        }
        break;
      case CLIENT_CONNECTED:
        if(((segment[0]&0x70)>>4) == ACK){ //Only one data_segment to be sent per connection!
          stop_timer();
          status.state = ADP;
          status.NACK_counter = 0;
          create_dummy_segment(answer_segment);
          answer_segment[CONTROL] = (DISCONNECT_REQ<<4);
          add_checksum(answer_segment);
          transmit_NET(answer_segment, src_ID);
          start_timer(255);
        }
        else if(((segment[0]&0x70)>>4) == NACK){
          stop_timer();
          if(status.NACK_counter ++ > 10){
            end_con();
            return;
          }
          create_data_segment(status.transmitt_buffer, answer_segment);
          answer_segment[CONTROL] = (DATA_MESSAGE<<4);
          add_checksum(answer_segment);
          transmit_NET(answer_segment, src_ID);
          start_timer(255);
        }
        break;

      case PDP:
        if(((segment[0]&0x70)>>4) == ACK){
          stop_timer();
          app_layer_receive(status.receive_buffer);
          end_con();
          return;
        }
        break;

      case ADP: //SENDING DATA!
        if(((segment[0]&0x70)>>4) == DISCONNECT_REQ){
          stop_timer();
          create_dummy_segment(answer_segment);
          answer_segment[CONTROL] = (ACK<<4);
          add_checksum(answer_segment);
          transmit_NET(answer_segment, src_ID);
          end_con();
        }
        break;
      }
    }
  }

void trans_layer_send(uint8_t app_data[], uint8_t ID){
  if (!status.number_of_data_packages){
    //status.transmitt_buffer is empty
    status.transmitt_buffer[0] = ID;
    for(int i = 0; i < APPDATA_SIZE; i++){
      status.transmitt_buffer[i+1] = app_data[i];
    }
    status.number_of_data_packages++;

    //SEND DATA;
    uint8_t segment[HEADER_SIZE+APPDATA_SIZE];

    if(status.state == IDLE){
      status.state = AEP;
      status.connect_id = ID;
      connect_to_node(segment);
      transmit_NET(segment, ID);
      start_timer(255);
    return;
  }
}
  //BUFFER DATA
  else if(status.number_of_data_packages == 1){
    status.transmitt_buffer[APPDATA_SIZE+1] = ID;
    for(int i = 0; i < APPDATA_SIZE; i++){
      status.transmitt_buffer[APPDATA_SIZE+i+2] = app_data[i];
    }
    status.number_of_data_packages ++;
  }
  else if(status.number_of_data_packages == 2){
    status.transmitt_buffer[2*APPDATA_SIZE+2] = ID;
    for(int i = 0; i < APPDATA_SIZE; i++){
      status.transmitt_buffer[2*APPDATA_SIZE+i+3] = app_data[i];
    }
    status.number_of_data_packages++;
  }
  else if (status.number_of_data_packages == 3){
    status.transmitt_buffer[3*APPDATA_SIZE+3] = ID;
    for(int i = 0; i < APPDATA_SIZE; i++){
      status.transmitt_buffer[3*APPDATA_SIZE+i+4] = app_data[i];
    }
    status.number_of_data_packages++;
  }
}


ISR(TIMER0_COMPA_vect){
  if(!(expand_timer_counter--)){
    if(status.timer_counter++ > 2){ //do not try agian...
      stop_timer();
      end_con();
    }
    uint8_t segment[HEADER_SIZE+APPDATA_SIZE];
    switch (status.state) {
      case AEP:
        //RESEND CONNECT_RQ;
        connect_to_node(segment);
        transmit_NET(segment, status.connect_id);
        break;
      case PEP:
        create_dummy_segment(segment);
        segment[CONTROL] = (ACK<<4);
        add_checksum(segment);
        transmit_NET(segment, status.connect_id);
        break;

      case SERVER_CONNECTED:
        create_dummy_segment(segment);
        segment[CONTROL] = (ACK<<4);
        add_checksum(segment);
        transmit_NET(segment, status.connect_id);
        break;

      case CLIENT_CONNECTED:
        create_data_segment(status.transmitt_buffer, segment);
        segment[CONTROL] = (DATA_MESSAGE<<4);
        add_checksum(segment);
        transmit_NET(segment, status.connect_id);
        break;

      case PDP:
        create_dummy_segment(segment);
        segment[CONTROL] = (DISCONNECT_REQ<<4);
        add_checksum(segment);
        transmit_NET(segment, status.connect_id);
        break;

      case ADP:
        create_dummy_segment(segment);
        segment[CONTROL] = (DISCONNECT_REQ<<4);
        add_checksum(segment);
        transmit_NET(segment, status.connect_id);
        break;
      default:
        return;
    }
    expand_timer_counter = 300;
  }
}
