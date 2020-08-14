#include <stdint.h>
#include <stdlib.h>

#define APPDATA_SIZE 114

void app_layer_send(void);
void app_layer_receive(uint8_t app_data[]);
void pad_array(uint8_t app_data[], uint8_t start_index);
void event_received(uint8_t button, uint8_t button_count);
