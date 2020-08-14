#include <stdint.h>
#include <stdlib.h>
#include "config.h"

#if NODE_ID != NODE_ID_1
void increment_switch_counter(void);
void reset_switch_counter(void);
int switch_count_val(void);
#endif


void init_app(void);
void event_received(uint8_t button, uint8_t button_count);
void set_interrupt_flag(void);
void reset_interrupt_flag(void);
int interrupt_flag_status(void);
