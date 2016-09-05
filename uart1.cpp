/************************************************************************/
/*                                                                      */
/*                      Interrupt driven buffered UART                  */
/*                                                                      */
/*              Author: Peter Dannegger                                 */
/*                                                                      */
/************************************************************************/
#include "uart1.h"
#include <avr/interrupt.h>


static uint8_t rx_buff[RX1_SIZE];
static uint8_t rx_in;
static uint8_t rx_out;
static uint8_t tx_buff[TX1_SIZE];
static uint8_t tx_in;
static uint8_t tx_out;


#define ROLLOVER( x, max )	x = ++x >= max ? 0 : x
					// count up and wrap around

void init_uart1(void) {

  UBRR1H = UBRRH_VALUE;	//set baud rate
  UBRR1L = UBRRL_VALUE;

  #if USE_2X
  UCSR1A |= (1 << U2X1);
  #else
  UCSR1A &= ~(1 << U2X1);
  #endif

  UCSR1C = 1<<UCSZ11^1<<UCSZ10		// 8 Bit
          ^1<<UPM11			// Even Parity
  #ifdef URSEL1
    ^1<<URSEL1			// if UCSR1C shared with UBRR1H
  #endif
    ;

  UCSR1B = 1<<RXEN1^1<<TXEN1^		// enable RX, TX
	   1<<RXCIE1;			// enable RX interrupt
  rx_in = rx_out;			// set buffer empty
  tx_in = tx_out;
}


uint8_t ukbhit1(void) {
  return rx_out ^ (*(volatile uint8_t*)&(rx_in));		// rx_in modified by interrupt !
}


uint8_t ugetchar1(void) {
  uint8_t data;

  while(!ukbhit1());			// until at least one byte in
  data = rx_buff[rx_out];		// get byte
  ROLLOVER(rx_out, RX1_SIZE);
  URX1_IEN = 1;				// enable RX interrupt
  return data;
}


ISR(USART1_RX_vect) {
  uint8_t i = rx_in;

  ROLLOVER(i, RX1_SIZE);
  if(i == rx_out){			// buffer overflow
    URX1_IEN = 0;			// disable RX interrupt
    return;
  }
  rx_buff[rx_in] = UDR1;
  rx_in = i;
}


ISR(USART1_UDRE_vect) {
  if(tx_in == tx_out) {		// nothing to sent
    UTX1_IEN = 0;			// disable TX interrupt
    return;
  }
  UDR1 = tx_buff[tx_out];
  ROLLOVER(tx_out, TX1_SIZE);
}


uint8_t utx1_ready(void) {
  uint8_t i = tx_in;

  ROLLOVER(i, TX1_SIZE);
  return (*(volatile uint8_t*)&(tx_out)) ^ i;		// 0 = busy
}

uint8_t utx1_ready3(void) {
  uint8_t i = tx_in;
 
  ROLLOVER(i, TX1_SIZE);
  if(!((*(volatile uint8_t*)&(tx_out)) ^ i)) {
    return 0;
  }
  ROLLOVER(i, TX1_SIZE);
  if(!((*(volatile uint8_t*)&(tx_out)) ^ i)) {
    return 0;
  }
  ROLLOVER(i, TX1_SIZE);
  return (*(volatile uint8_t*)&(tx_out)) ^ i;
 
}

void uputchar1(uint8_t c) {
  uint8_t i = tx_in;

  ROLLOVER(i, TX1_SIZE);
  tx_buff[tx_in] = c;
  while(i == (*(volatile uint8_t*)&(tx_out)));		// until at least one byte free
					// tx_out modified by interrupt !
  tx_in = i;
  UTX1_IEN = 1;                         // enable TX interrupt
}


void uputs1_(uint8_t *s) {
  while(*s)
    uputchar1(*s++);
}
