//! MSP430F1612/1611 clock and I/O definitions

#include "platform.h"

#include <avr/io.h>
#include <util/delay.h>

//! Receive a byte.
unsigned char serial0_rx(){
  while( !(UCSR0A & (1 << RXC0)) );
  return UDR0;
}

//! Receive a byte.
unsigned char serial1_rx(){
  return 0;
}

//! Transmit a byte.
void serial0_tx(unsigned char x){
  while (!(UCSR0A & (1<<UDRE0)) );
  UDR0 = x;
}

//! Transmit a byte on the second UART.
void serial1_tx(unsigned char x){
}

//! Set the baud rate.
void setbaud0(unsigned char rate){
  /* disable everything briefly */
  UCSR0B = 0;

  int32_t r;
  switch(rate){
  case 1://9600 baud
    r = 9600;
    break;
  case 2://19200 baud
    r = 19200;
    break;
  case 3://38400 baud
    r = 38400;
    break;
  case 4://57600 baud
    r = 57600;
    break;

  default:
  case 5://115200 baud
    r = 115200;
    break;
  }

  /* enabling rx/tx must be done before frame/baud setup */
  UCSR0B = ((1 << TXEN0) | (1 << RXEN0));

  UCSR0A = (1 << U2X0);   /* double the baud rate */
  UCSR0C = (3 << UCSZ00); /* 8N1 */

  UBRR0L = (int8_t) (F_CPU/(r*8L)-1);
  UBRR0H = (F_CPU/(r*8L)-1) >> 8;

  return;
  
}

//! Set the baud rate of the second uart.
void setbaud1(unsigned char rate){
  //http://mspgcc.sourceforge.net/baudrate.html
  switch(rate){
  case 1://9600 baud
    
    break;
  case 2://19200 baud
    
    break;
  case 3://38400 baud
    
    break;
  case 4://57600 baud
    
    break;
  default:
  case 5://115200 baud
    
    break;
  }
}


void avr_init_uart0(){
  PORTD = _BV(PD2);
  setbaud0(0);
  _delay_ms(500); //takes a bit to stabilize
}


void avr_init_uart1(){
}



