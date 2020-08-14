#include "APP.h"
#include "TRAN.h"
#include "../application/application.h"
#include "../application/config.h"
#define PAD_VALUE 0
#define TEST 1

void pad_array(uint8_t array[], uint8_t start_index){
  for (int i = start_index; i<APPDATA_SIZE; i++){
    array[i] = PAD_VALUE;
  }
}

void app_layer_receive(uint8_t app_data[]){
   uint8_t i = 0;
   uint8_t button;
   uint8_t button_count;
  while (app_data[i] != PAD_VALUE){ //PAD_VALUE){
    button = app_data[i];
    button_count = app_data[i+1];
    i+=2;
    event_received(button, button_count);
  }
}

void app_layer_send(void){
  uint8_t app_data[APPDATA_SIZE];
  #if TEST
  //NODE_ID == 3
  if(switch_count_val() == 0){
    increment_switch_counter();
    app_data[0] = switch_2;
    app_data[1] = switch_count_val();
    pad_array(app_data, 2);
    trans_layer_send(app_data, NODE_ID_2);

  }
  else if (switch_count_val() == 1){
    increment_switch_counter();
      app_data[0] = switch_2;
      app_data[1] = switch_count_val();
      pad_array(app_data, 2);
      trans_layer_send(app_data, NODE_ID_2);

    }

  else if (switch_count_val() == 2){
      increment_switch_counter();
      app_data[0] = switch_2;
      app_data[1] = switch_count_val();
      pad_array(app_data, 2);
      trans_layer_send(app_data, NODE_ID_2);
      reset_switch_counter();
    }
  return;
  #endif

  #if NODE_ID == NODE_ID_3
  if(switch_count_val() == 0){
    increment_switch_counter();
    app_data[0] = switch_2;
    app_data[1] = switch_count_val();
    pad_array(app_data, 2);
    trans_layer_send(app_data, NODE_ID_1);

  }

  else if (switch_count_val() == 1){
    increment_switch_counter();
    app_data[0] = switch_2;
    app_data[1] = switch_count_val();
    pad_array(app_data, 2);
    trans_layer_send(app_data, NODE_ID_2);
  }

  else if (switch_count_val() == 2){
    increment_switch_counter();
    app_data[0] = switch_2;
    app_data[1] = switch_count_val();
    pad_array(app_data, 2);
    trans_layer_send(app_data, NODE_ID_2);
    trans_layer_send(app_data, NODE_ID_1);
    reset_switch_counter();
  }

  #elif NODE_ID == NODE_ID_2
  if(switch_count_val() == 0){
    increment_switch_counter();
    app_data[0] = SWITCH_1;
    app_data[1] = switch_count_val();
    pad_array(app_data, 2);
    trans_layer_send(app_data, NODE_ID_1);
  }
  else if (switch_count_val() == 1){
    increment_switch_counter();
    app_data[0] = SWITCH_1;
    app_data[1] = switch_count_val();
    pad_array(app_data, 2);
    trans_layer_send(app_data, NODE_ID_3);
  }
  else if (switch_count_val() == 2){
    increment_switch_counter();
    app_data[0] = SWITCH_1;
    app_data[1] = switch_count_val();
    pad_array(app_data, 2);
    trans_layer_send(app_data, NODE_ID_3);
    trans_layer_send(app_data, NODE_ID_1);
    reset_switch_counter();
  }
  #endif
}
