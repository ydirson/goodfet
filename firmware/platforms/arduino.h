/*! \file goodfet.h
  \author Travis Goodspeed
  \brief Port descriptions for the GoodFET platform.
*/

#define ARDUINO

#define F_CPU 16000000UL

void arduino_init();

#include <avr/io.h>
//#include <util/delay.h>

//LED on P1.0
#define PLEDOUT PORTB
#define PLEDDIR DDRB
#define PLEDPIN PB5

#define SPIOUT PORTB
#define SPIDIR DDRB
#define SPIIN  PINB

// see also include/spi.h
#define SETSS PORTB|=SS;
#define CLRSS PORTB&=~SS;
#define DIRSS DDRB|=SS;

#define SETCE PORTB|=CE
#define CLRCE PORTB&=~CE
#define DIRCE PORTB|=CE

// network byte order converters
#define htons(x) ((((uint16_t)(x) & 0xFF00) >> 8) | \
				 (((uint16_t)(x) & 0x00FF) << 8))
#define htonl(x) ((((uint32_t)(x) & 0xFF000000) >> 24) | \
				  (((uint32_t)(x) & 0x00FF0000) >> 8) | \
				  (((uint32_t)(x) & 0x0000FF00) << 8) | \
				  (((uint32_t)(x) & 0x000000FF) << 24))

#define ntohs htons
#define ntohl htonl

