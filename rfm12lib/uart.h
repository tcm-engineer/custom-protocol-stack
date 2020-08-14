#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

#define F_CPU 12000000

//uart
void init_uart0 (void);
char get_ch (void);
void put_ch (char ch);
void put_str (char *str);

