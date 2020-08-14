#include "uart.h"


void init_uart0 (void)
{
	/* Configure 9600 baud , 8-bit , no parity and one stop bit */
	const int baud_rate = 9600;
	UBRR0H = (F_CPU/(baud_rate*16L)-1) >> 8;
	UBRR0L = (F_CPU/(baud_rate*16L)-1);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

char get_ch (void)
{
	while (!( UCSR0A & _BV(RXC0)));
	return UDR0 ;
}
void put_ch ( char ch)
{
	while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = ch;
}

void put_str (char *str)
{
	int i;
	for (i=0; str[i]; i++) put_ch(str[i]);
}
