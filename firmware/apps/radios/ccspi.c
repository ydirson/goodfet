/*! \file ccspi.c
  \author Travis Goodspeed
  \brief Chipcon SPI Register Interface
  
  Unfortunately, there is very little similarity between the CC2420
  and the CC2500, to name just two of the myriad of Chipcon SPI
  radios.  Auto-detection will be a bit difficult, but more to the
  point, all high level functionality must be moved into the client.
*/

//Higher level left to client application.

#include "platform.h"
#include "command.h"
#include <stdlib.h> //added for itoa
#include <signal.h>
#include <io.h>
#include <iomacros.h>

#include "ccspi.h"
#include "spi.h"

//! Handles a Chipcon SPI command.
void ccspi_handle_fn( uint8_t const app,
					  uint8_t const verb,
					  uint32_t const len);

// define the ccspi app's app_t
app_t const ccspi_app = {

	/* app number */
	CCSPI,

	/* handle fn */
	ccspi_handle_fn,

	/* name */
	"CCSPI",

	/* desc */
	"\tThe CCSPI app adds support for the Chipcon SPI register\n"
	"\tinterface. Unfortunately, there is very little similarity\n"
	"\tbetween the CC2420 and the CC2500, to name just two of the\n"
	"\tmyriad of Chipcon SPI radios.  Auto-detection will be a bit\n"
	"\tdifficult, but more to the point, all high level functionality\n"
	"\tmust be moved into the client.\n"
};

//! Set up the pins for CCSPI mode.
void ccspisetup(){
  SPIDIR&=~MISO;
  SPIDIR|=MOSI+SCK;
  DIRSS;
  DIRCE;
  
  P4OUT|=BIT5; //activate CC2420 voltage regulator
  msdelay(100);
  
  //Reset the CC2420.
  P4OUT&=~BIT6;
  P4OUT|=BIT6;
  
  //Begin a new transaction.
  CLRSS;
  SETSS;
}

//! Read and write an CCSPI byte.
u8 ccspitrans8(u8 byte){
  register unsigned int bit;
  //This function came from the CCSPI Wikipedia article.
  //Minor alterations.
  
  for (bit = 0; bit < 8; bit++) {
    /* write MOSI on trailing edge of previous clock */
    if (byte & 0x80)
      SETMOSI;
    else
      CLRMOSI;
    byte <<= 1;
 
    SETCLK;
  
    /* read MISO on trailing edge */
    byte |= READMISO;
    CLRCLK;
  }
  
  return byte;
}


//! Writes a register
u8 ccspi_regwrite(u8 reg, const u8 *buf, int len){
  CLRSS;
  
  reg=ccspitrans8(reg);
  while(len--)
    ccspitrans8(*buf++);
  
  SETSS;
  return reg;//status
}
//! Reads a register
u8 ccspi_regread(u8 reg, u8 *buf, int len){
  CLRSS;
  
  reg=ccspitrans8(reg);
  while(len--)
    *buf++=ccspitrans8(0);
  
  SETSS;
  return reg;//status
}

//! Handles a Chipcon SPI command.
void ccspi_handle_fn( uint8_t const app,
		      uint8_t const verb,
		      uint32_t const len){
  unsigned long i;
  
  //debugstr("Chipcon SPI handler.");
  
  switch(verb){
  case PEEK:
    cmddata[0]|=0x40; //Set the read bit.
    //DO NOT BREAK HERE.
  case READ:
  case WRITE:
  case POKE:
    CLRSS; //Drop !SS to begin transaction.
    for(i=0;i<len;i++)
      cmddata[i]=ccspitrans8(cmddata[i]);
    SETSS;  //Raise !SS to end transaction.
    txdata(app,verb,len);
    break;
  case SETUP:
    ccspisetup();
    txdata(app,verb,0);
    break;
  case CCSPI_RX:
#ifdef FIFOP
    //Has there been an overflow?
    if((!FIFO)&&FIFOP){
      debugstr("Clearing overflow");
      CLRSS;
      ccspitrans8(0x08); //SFLUSHRX
      SETSS;
    }
    
    //Is there a packet?
    if(FIFOP&&FIFO){
      //Wait for completion.
      while(SFD);
      
      //Get the packet.
      CLRSS;
      ccspitrans8(CCSPI_RXFIFO | 0x40);
      //ccspitrans8(0x3F|0x40);
      cmddata[0]=0xff; //to be replaced with length
      for(i=0;i<cmddata[0]+2;i++)
        cmddata[i]=ccspitrans8(0xde);
      SETSS;
      
      //Flush buffer.
      CLRSS;
      ccspitrans8(0x08); //SFLUSHRX
      SETSS;
      //Only should transmit length of one more than the reported
      // length of the frame, which holds the length byte:
      txdata(app,verb,cmddata[0]+1);
    }else{
      //No packet.
      txdata(app,verb,0);
    }
#else
    debugstr("Can't RX a packet with SFD and FIFOP definitions.");
    txdata(app,NOK,0);
#endif
    break;
  case CCSPI_RX_FLUSH:
    //Flush the buffer.
    CLRSS;
    ccspitrans8(CCSPI_SFLUSHRX);
    SETSS;
    
    txdata(app,verb,0);
    break;

  case CCSPI_REFLEX:
#if defined(FIFOP) && defined(SFD) && defined(FIFO) && defined(PLED2DIR) && defined(PLED2PIN) && defined(PLED2OUT)
    txdata(app,verb,1);  //Just sending some response back to client
    while(1) {
        //Wait until a packet is received
        while(!SFD);
        //Turn on LED 2 (green) as signal
	    PLED2DIR |= PLED2PIN;
	    PLED2OUT &= ~PLED2PIN;

        //Put radio in TX mode
        CLRSS;
        ccspitrans8(0x04);
        SETSS;

        //Load the jamming packet.
        //TODO try to preload this to get faster effects
        CLRSS;
        ccspitrans8(CCSPI_TXFIFO);
        char pkt[15] = {0x0f, 0x01, 0x08, 0x82, 0xff, 0xff, 0xff, 0xff, 0xde, 0xad, 0xbe, 0xef, 0xba, 0xbe, 0xc0};
        //char pkt[12] = {0x0c, 0x01, 0x08, 0x82, 0xff, 0xff, 0xff, 0xff, 0xde, 0xad, 0xbe, 0xef};
        for(i=0;i<pkt[0];i++)
          ccspitrans8(pkt[i]);
        SETSS;

        //Transmit the packet.
        CLRSS;
        ccspitrans8(0x04); //STXON
        SETSS;
        msdelay(100);      //Instead of waiting for pulse on SFD
        //Flush TX buffer.
        CLRSS;
        ccspitrans8(0x09); //SFLUSHTX
        SETSS;

        //Turn off LED 2 (green) as signal
	    PLED2DIR |= PLED2PIN;
	    PLED2OUT |= PLED2PIN;
    }
    //TODO the firmware stops staying in this mode after a while, and stops jamming... need to find a fix.
    break;
#else
    debugstr("Can't reflexively jam without SFD, FIFO, FIFOP, and P2LEDx definitions - try using telosb platform.");
    txdata(app,NOK,0);
#endif

  case CCSPI_REFLEX_SEQNUM:
#if defined(FIFOP) && defined(SFD) && defined(FIFO) && defined(PLED2DIR) && defined(PLED2PIN) && defined(PLED2OUT)
    //char byte[4];
    while(1) {
        //Has there been an overflow in the RX buffer?
        //TODO do we really need to check this??
        if((!FIFO)&&FIFOP){
          //debugstr("Clearing overflow");
          CLRSS;
          ccspitrans8(0x08); //SFLUSHRX
          SETSS;
        }

        //Wait until a packet is received
        while(!SFD);
        //Turn on LED 2 (green) as signal
	    PLED2DIR |= PLED2PIN;
	    PLED2OUT &= ~PLED2PIN;

        //Get the orignally received packet, up to the seqnum field.
        CLRSS;
        ccspitrans8(CCSPI_RXFIFO | 0x40);
        for(i=0;i<4;i++)
            cmddata[i]=ccspitrans8(0xde);
        SETSS;
        //Flush RX buffer.
        CLRSS;
        ccspitrans8(0x08); //SFLUSHRX
        SETSS;
        //Send the sequence number of the jammed packet back to the client
        //itoa(cmddata[3], byte, 16);
        //debugstr(byte);
        txdata(app,verb,cmddata[3]);

        //Put radio in TX mode
        CLRSS;
        ccspitrans8(0x04);
        SETSS;

        //Load the packet.
        CLRSS;
        ccspitrans8(CCSPI_TXFIFO);
        char pkt[12] = {0x0c, 0x01, 0x08, 0x82, 0xff, 0xff, 0xff, 0xff, 0xde, 0xad, 0xbe, 0xef};
        for(i=0;i<pkt[0];i++)
          ccspitrans8(pkt[i]);
        SETSS;

        //Transmit the packet.
        CLRSS;
        ccspitrans8(0x04);  //STXON
        SETSS;
        msdelay(200);       //Instead of examining SFD line status
        //Flush TX buffer.
        CLRSS;
        ccspitrans8(0x09);  //SFLUSHTX
        SETSS;

        //Turn off LED 2 (green) as signal
	    PLED2DIR |= PLED2PIN;
	    PLED2OUT |= PLED2PIN;
    }
    //TODO the firmware stops staying in this mode after a while, and stops jamming... need to find a fix.
#else
    debugstr("Can't reflexively jam without SFD, FIFO, FIFOP, and P2LEDx definitions - try using telosb platform.");
    txdata(app,NOK,0);
#endif
    break;

  case CCSPI_TX_FLUSH:
    //Flush the buffer.
    CLRSS;
    ccspitrans8(CCSPI_SFLUSHTX);
    SETSS;
    
    txdata(app,verb,0);
    break;
  case CCSPI_TX:
#ifdef FIFOP
    
    //Wait for last packet to TX.
    //while(ccspi_status()&BIT3);
    
    //Load the packet.
    CLRSS;
    ccspitrans8(CCSPI_TXFIFO);
    for(i=0;i<cmddata[0];i++)
      ccspitrans8(cmddata[i]);
    SETSS;
    
    //Transmit the packet.
    CLRSS;
    ccspitrans8(0x04); //STXON
    SETSS;
    
    //Wait for the pulse on SFD, after which the packet has been sent.
    while(!SFD);
    while(SFD);
    
    //Flush TX buffer.
    CLRSS;
    ccspitrans8(0x09); //SFLUSHTX
    SETSS;
    
    txdata(app,verb,0);
#else
    debugstr("Can't TX a packet with SFD and FIFOP definitions.");
    txdata(app,NOK,0);
#endif
    break;
  default:
    debugstr("Not yet supported in CCSPI");
    txdata(app,verb,0);
    break;
  }

}
