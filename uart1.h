/************************************************************************/
/*                                                                      */
/*                      Interrupt driven buffered UART                  */
/*                                                                      */
/*              Author: Peter Dannegger                                 */
/*                                                                      */
/************************************************************************/
#ifndef UART1_H
#define UART1_H

#define BAUD 19200UL      // Baud rate
#include <util/setbaud.h>
#include <avr/io.h>
#include "MYDEFS.H"
					// size must be in range 2 .. 256
#define RX1_SIZE	32		// usable: RX1_SIZE + 2 (4 .. 258)
#define TX1_SIZE	32		// usable: TX1_SIZE + 1 (3 .. 257)

#define	uputs1(x)	uputs1_((uint8_t*)(x))	// avoid char warning


uint8_t ukbhit1(void);			// 0 = rx buffer empty
uint8_t ugetchar1(void);			// get received byte
uint8_t utx1_ready(void);			// 0 = tx still busy
uint8_t utx1_ready3(void);			// 0 = tx still busy; 1 = 3 Bytes free
void uputchar1(uint8_t c);			// send byte
void uputs1_(uint8_t *s);			// send string from SRAM
void init_uart1(void);


#define	UTX1_IEN	SBIT( UCSR1B, UDRIE1 )
#define	URX1_IEN	SBIT( UCSR1B, RXCIE1 )


/******************************	Resolve AVR naming chaos ****************/

//-----------------------------	Interrupt vectors -----------------------/
#ifndef	USART1_RX_vect
#if defined 	USART_RX_vect
#define		USART1_RX_vect	USART_RX_vect
#elif defined	USART_RXC_vect
#define		USART1_RX_vect	USART_RXC_vect
#elif defined	USART1_RXC_vect
#define		USART1_RX_vect	USART1_RXC_vect
#endif
#endif

#if !defined USART1_UDRE_vect && defined USART_UDRE_vect
#define	USART1_UDRE_vect USART_UDRE_vect
#endif

//-----------------------------	Register names --------------------------/
#ifndef	UCSR1A
#define	UCSR1A	UCSRA
#endif
#ifndef	UCSR1B
#define	UCSR1B	UCSRB
#endif
#ifndef	UCSR1C
#define	UCSR1C	UCSRC
#endif
#ifndef	UDR1
#define	UDR1	UDR
#endif
#ifndef	UBRR1L
#define	UBRR1L	UBRRL
#endif
#ifndef	UBRR1H
#define	UBRR1H	UBRRH
#endif

//-----------------------------	Bit names -------------------------------/
#ifndef	UCSZ11
#define	UCSZ11	UCSZ1
#endif
#ifndef	UCSZ11
#define	UCSZ11	UCSZ1
#endif
#if !defined URSEL1 && defined URSEL
#define	URSEL1	URSEL
#endif
#ifndef	RXEN1
#define	RXEN1	RXEN
#endif
#ifndef	TXEN1
#define	TXEN1	TXEN
#endif
#ifndef	UDRIE1
#define	UDRIE1	UDRIE
#endif
#ifndef	RXCIE1
#define	RXCIE1	RXCIE
#endif

#endif //UART1_H
