#ifndef CSMA_H
#define CSMA_H

#include <util/delay.h>
#include <stdlib.h>
#include "../rfm12lib/rfm12_config.h"
#include "../rfm12lib/rfm12.h"
#include "../rfm12lib/uart.h"
#include "../rfm12lib/rfm12_hw.h"
#include "../rfm12lib/rfm12_core.h"
#include "../rfm12lib/rfm12_spi.c"

void csma_p();

extern uint16_t csma_slot_length;
extern uint8_t csma_probability; 

#endif