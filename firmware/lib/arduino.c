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

        uint8_t x;

        /* explicitly clear interrupts */
        cli();

        /* init the USART */
        avr_init_uart0();

	/* set the LED as an output */
	PLEDDIR |= (1 << PLEDPIN);
	PLEDOUT |= (1 << PLEDPIN);

        /* explicitly enable interrupts */
        sei();

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

void
led_toggle(void)
{
	led_on();
	_delay_ms(30);
	led_off();
}

#endif
