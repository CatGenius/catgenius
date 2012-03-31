/******************************************************************************/
/* File    :	i2c.c							      */
/* Function:	I2C bus functional implementation			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "hardware.h"			/* Flexible hardware configuration */

#include "i2c.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define BUS_FREQ	100000	/* 400kHz bus frequency */


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static unsigned char i2c_waitready(void);


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void i2c_init (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
/*		- Initializes the module				      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	/* Set I2C controller in master mode */
	SSPCON  = 0x38;
	SSPCON2 = 0x00;

	/* Set I2C bus frequency */
	SSPADD = ((_XTAL_FREQ/4) / BUS_FREQ) /*-1*/;

	CKE = 1;	/* Use I2C levels TODO: Why? Worked also with '0' */
	SMP = 1;	/* Disable slew rate control TODO: Why? Worked also with '0' */

	SSPIF=0;	/* Clear I2C interrupt flag */
	BCLIF=0;	/* Clear collision interrupt flag */
}
/* End: i2c_init */


void i2c_work (void)
/******************************************************************************/
/* Function:	Module worker routine					      */
/*		-       */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
} /* i2c_work */


void i2c_term (void)
/******************************************************************************/
/* Function:	Module termination routine				      */
/*		- Terminates the module					      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* End: i2c_term */

void i2c_start(void)
{
	i2c_waitready();
	/* Start condition */
	SEN = 1;
}

void i2c_restart(void)
{
	i2c_waitready();
	/* Repeated start condition */
	RSEN = 1;
}

void i2c_read(unsigned char *byte, unsigned char ack)
{
	i2c_waitready();
	/* Enable reading */
	RCEN = 1;

	i2c_waitready();
	/* Read data */
	if (byte)
		*byte = SSPBUF;

	i2c_waitready();
	/* Send (n)ack */
	ACKDT = ack?0:1;
	ACKEN = 1;
}

unsigned char i2c_write(unsigned char byte)
{
	i2c_waitready();
	/* Write data */
	SSPBUF = byte;
	i2c_waitready();
	return(!ACKSTAT);
}

void i2c_stop(void)
{
	i2c_waitready();
	/* Stop condition */
	PEN = 1;
}

/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static unsigned char i2c_waitready(void)
{
	/* Using a simple timeout for resource reasons */
	unsigned char timeout = 255;

	while(((SSPCON2 & 0x1F) | R_nW) && timeout)
		timeout--;
	return ((SSPCON2 & 0x1F) | R_nW);
}