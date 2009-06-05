//GoodFET I2C Master Application
//Handles basic I/O

//Higher level left to client application.

//Based upon a neighborly example at
//http://codinglab.blogspot.com/2008/10/i2c-on-avr-using-bit-banging.html

#include "platform.h"
#include "command.h"

#include <signal.h>
#include <io.h>
#include <iomacros.h>


//Pins and I/O
#define SS   BIT3
#define SDA  BIT2
#define SCL  BIT3

#define I2CDELAY(x) delay(x)

//Bits are cleared by output of low.
//Bits are set but input and pull-up resistor.
#define SETSDA P5DIR&=~SDA
#define CLRSDA P5DIR|=SDA
#define SETSCL P5DIR&=~SCL
#define CLRSCL P5DIR|=SCL

#define READSDA (P5IN&SDA?1:0)
#define SETBOTH P5DIR&=~(SDA|SCL)

#define I2C_DATA_HI() SETSDA
#define I2C_DATA_LO() CLRSDA

#define I2C_CLOCK_HI() SETSCL
#define I2C_CLOCK_LO() CLRSCL

//! Write an I2C bit.
void I2C_WriteBit( unsigned char c )
{
  if(c>0)
    I2C_DATA_HI();
  else
    I2C_DATA_LO();

  I2C_CLOCK_HI();
  I2CDELAY(1);

  I2C_CLOCK_LO();
  I2CDELAY(1);

  if(c>0)
    I2C_DATA_LO();

  I2CDELAY(1);
}

//! Read an I2C bit.
unsigned char I2C_ReadBit()
{
  I2C_DATA_HI();

  I2C_CLOCK_HI();
  I2CDELAY(1);

  unsigned char c = READSDA;

  I2C_CLOCK_LO();
  I2CDELAY(1);

  return c;
}

//! Inits bitbanging port, must be called before using the functions below
void I2C_Init()
{
  //Clear SDA and SCL.
  //Direction, not value, is used to set the value.
  //(Pull-up or 0.)
  P5OUT&=~(SDA|SCL);
  
  I2C_CLOCK_HI();
  I2C_DATA_HI();

  I2CDELAY(1);
}

//! Send a START Condition
void I2C_Start()
{
  // set both to high at the same time
  SETBOTH;
  I2CDELAY(1);
  
  I2C_DATA_LO();
  I2CDELAY(1);
  
  I2C_CLOCK_LO();
  I2CDELAY(1);
}

//! Send a STOP Condition
void I2C_Stop()
{
  I2C_CLOCK_HI();
  I2CDELAY(1);

  I2C_DATA_HI();
  I2CDELAY(1);
}

//! write a byte to the I2C slave device
unsigned char I2C_Write( unsigned char c )
{
  char i;
  for (i=0;i<8;i++){
    I2C_WriteBit( c & 128 );
    c<<=1;
  }
  
  return I2C_ReadBit();
}


//! read a byte from the I2C slave device
unsigned char I2C_Read( unsigned char ack )
{
  unsigned char res = 0;
  char i;
  
  for (i=0;i<8;i++){
    res <<= 1;
    res |= I2C_ReadBit();  
  }
  
  if( ack > 0)
    I2C_WriteBit(0);
  else
    I2C_WriteBit(1);
  
  I2CDELAY(1);
  
  return res;
}


//! Handles a monitor command.
void i2chandle(unsigned char app,
	       unsigned char verb,
	       unsigned char len){
  unsigned char i;
  switch(verb){
    
  case PEEK:
    break;
  case POKE:
    break;
    
  case READ:
    if(len>0)          //optional parameter of length
      len=cmddata[0];
    if(len==0)         //default value of 1
      len=1;
    for(i=0;i<len;i++)
      cmddata[i]=I2C_Read(1);  //Always acknowledge
    txdata(app,verb,len);
    break;
  case WRITE:
    for(i=0;i<len;i++)
      I2C_Write(cmddata[i]);
    txdata(app,verb,0);
    break;
  case START:
    I2C_Start();
    break;
  case STOP:
    I2C_Stop();
    break;
  case SETUP:
    I2C_Init();
    txdata(app,verb,0);
    break;
  }
}
