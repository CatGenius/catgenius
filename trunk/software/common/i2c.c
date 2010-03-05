/******************************************************************************/
/* File    :	i2c.c							      */
/* Function:	I2C bus functional implementation			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "i2c.h"
#include "hardware.h"
#include "timer.h"

#include "serial.h"

/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define BIT(n)			(1U << (n))	/* Bit mask for bit 'n' */
#define BUS_FREQ	100000	/* 400kHz bus frequency */
#define I2C_IDLE	0
#define I2C_START	1
#define I2C_RESTART	2
#define I2C_WRITE	3
#define I2C_STOP	4

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer		timer    = EXIRED;
static unsigned char	rd_state = 0;
static unsigned char	state = 0;
static unsigned char	wr_byte = 0;
static unsigned char	rd_byte = 0;
static bit		acked = 0;

/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static unsigned char	i2c_busy(void);
static void		i2c_start(void);
static void		i2c_restart(void);
static void		i2c_address(unsigned char byte, unsigned char read);
static void		i2c_write(unsigned char byte);
static void		i2c_stop(void);


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
	/*
	 * Setup port C
	 */
	TRISC |= I2C_SCL_MASK |		/* I2C SCL */
		 I2C_SDA_MASK ;		/* I2C SDA */

	/* Set I2C controller in master mode */
	SSPCON  = 0x38;
	SSPCON2 = 0x00;

	/* Set I2C bus frequency */
	SSPADD = ((_XTAL_FREQ/4) / BUS_FREQ) /*-1*/;

	CKE = 1;     // use I2C levels      worked also with '0'
	SMP = 1;     // disable slew rate control  worked also with '0'
	
	PSPIF=0;      // clear SSPIF interrupt flag
	BCLIF=0;      // clear bus collision flag
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
	if (timeoutexpired(&timer)) {
		settimeout(&timer, SECOND);
		i2c_init();
		while (( SSPCON2 & 0x1F ) | RW ) {}; // wait for idle and not writing
		SEN=1;
		
		while (( SSPCON2 & 0x1F ) | RW ) {}; // wait for idle and not writing
		SSPBUF = 0x50 << 1;
		
		while (( SSPCON2 & 0x1F ) | RW ) {}; // wait for idle and not writing
		if (!ACKSTAT)
			putch('A');
		else
			putch('N');
		PEN=1;
	}

#if 0
	switch (state) {
	case I2C_IDLE:
		break;

	case I2C_START:
		/* Wait for idle */
		if (!((SSPCON2 & 0x1F) | RW)) {
			/* Start condition */
			SEN = 1;
			state = I2C_IDLE;
		}
		break;

	case I2C_RESTART:
		/* Wait for idle */
		if (!((SSPCON2 & 0x1F) | RW)) {
			/* Repeated start condition */
			RSEN = 1;
			state = I2C_IDLE;
		}
		break;

	case I2C_WRITE:
		/* Wait for idle */
		if (!((SSPCON2 & 0x1F) | RW)) {
			/* Write data */
			SSPBUF = wr_byte;
			acked = !ACKSTAT;
			state = I2C_IDLE;
		}
		break;

	case I2C_STOP:
		/* Wait for idle */
		if (!((SSPCON2 & 0x1F) | RW)) {
			/* Stop condition */
			PEN = 1;
			state = I2C_IDLE;
		}
		break;
	}
	
	switch(rd_state) {
	case 0:
		if (timeoutexpired(&timer) &&
		    !i2c_busy()) {
			settimeout(&timer, SECOND/4);
			i2c_start();
			rd_state++;
		}
		break;
	case 1:
		if (!i2c_busy()) {
			i2c_address(0x50, 0);
			rd_state++;
		}
		break;
	case 2:
		if (!i2c_busy()) {
			i2c_write(0x00);
			rd_state++;
		}
		break;
	case 3:
		if (!i2c_busy()) {
			if (acked)
				putch('A');
			else
				putch('N');
			state = I2C_IDLE;
			i2c_stop();
			rd_state = 0;
		}
		break;
	}
#endif
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



/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/
#if 0
static unsigned char i2c_busy(void)
{
	return (state != I2C_IDLE);
}

static void i2c_start(void)
{
	state = I2C_START;
}

static void i2c_restart(void)
{
	state = I2C_RESTART;
}

static void i2c_address(unsigned char byte, unsigned char read)
{
	wr_byte = byte << 1;
	if (read)
		wr_byte |= 0x01;
	state = I2C_WRITE;
}

static void i2c_write(unsigned char byte)
{
	wr_byte = byte;
	state = I2C_WRITE;
}

static void i2c_stop(void)
{
	state = I2C_STOP;
}
#endif