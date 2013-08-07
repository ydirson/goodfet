/*! \file arduino.c
  \author Travis Goodspeed
  \brief Arduino platform support.
*/

#include "platform.h"
#ifdef ARDUINO

#include <avr/interrupt.h>
#include <util/delay.h>

//! Arduino setup code.
void arduino_init(){
  //LED port as output.
  DDRB = 0xFF;
  
  //Disabled interrupts.
  cli();
  
  avr_init_uart0();
}

void led_init(){

  PLEDDIR |= (1 << PLEDPIN);
}

void  led_on() {
  PLEDOUT |= (1 << PLEDPIN);
}

void led_off() {
  PLEDOUT &= ~(1 << PLEDPIN);
}

#endif
