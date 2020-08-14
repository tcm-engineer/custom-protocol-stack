#ifndef PHY_H
#define PHY_H

#include <util/delay.h>
#include <stdlib.h>
#include "../LLC/LLC.H"
#include "../MAC/csma.H"

uint8_t transmit_PHY(uint8_t *dll_array, uint8_t arrSize);
uint8_t receive_PHY();

#endif