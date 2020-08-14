#include <stdint.h>
#include <stdlib.h>
#include "../APP/APP.h"

#define HEADER_SIZE 7
#define CONTROL 0 //Two bytes
#define SEQUENCE_NUMBER
#define SRC_PORT 2
#define DEST_PORT 3
#define LENGTH 4
#define APP_DATA 5 //114 Bytes
#define CHECKSUM 119

#define DATA_MESSAGE 0
#define ACK 1
#define NACK 2
#define CONNECTION_REQ 3
#define DISCONNECT_REQ 5

//STATES
#define IDLE 0
#define PEP 1 //Passive Establishment pendng
#define AEP 2 //Actve Establishment Pendng
#define CLIENT_CONNECTED 3 //Connection Established
#define SERVER_CONNECTED 4
#define PDP 5 //Passive Disconnect pending
#define ADP 6 //Active Disconnect Pending

#define VARIABLE_APP_DATA_LENGTH 0
#define USE_SEQUENCE_NUMBER 0

struct Status {
  uint8_t state;
  uint8_t timer_counter;
  uint8_t NACK_counter;
  uint8_t receive_buffer[APPDATA_SIZE]; //Only one can be received per connection.
  uint8_t transmitt_buffer[4*APPDATA_SIZE + 4]; //FIXED BUFFER SIZE FOR THIS APPLICATION! on form [ID, appdata, ID, appdata, ...] Maximum 4 ID's plus corresponding app-data.
  uint8_t number_of_data_packages;
  int connect_id;
};

void trans_layer_send(uint8_t app_data[], uint8_t ID);
void init_transport_layer(void);
void transport_layer_receive(uint8_t segment[], uint8_t src_ID);
//struct Segment seg;

//testing
void create_dummy_segment(uint8_t segment[]);
void add_checksum(uint8_t segment[]);
void response(int type);
