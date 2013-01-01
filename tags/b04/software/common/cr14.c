/******************************************************************************/
/* File    :	cr14.c							      */
/* Function:	ST-Micro CR14 RFID reader functional implementation	      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	7 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "cr14.h"
#include "i2c.h"

#include "serial.h"

/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define I2C_ADDR	0x50
#define PARAM_REG	0x00
#define FRAME_REG	0x01
#define FRAME_REG_SIZE	35
#define SLOTMARK_REG	0x03
#define FRAME_MAX	8

#define IDLE		0
#define RD_PARAMREG	1
#define WR_PARAMREG	8
#define RD_FRAME	13
#define WR_FRAME	24


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

unsigned char cr14_writeparamreg(unsigned char regval)
/******************************************************************************/
/* Function:	cr14_writeparamreg					      */
/*		- Write to the CR14 parameter register			      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	unsigned char	status	= 0;

	i2c_start();
	if (!i2c_address(I2C_ADDR, I2C_WR)) {
		status = CR14_BUSY;
		goto stop;
	}
	if (!i2c_write(PARAM_REG)) {
		status = -1;
		goto stop;
	}
	if (!i2c_write(regval))
		status = -1;
stop:
	i2c_stop();
	return status;
}
/* End: cr14_writeparamreg */


unsigned char cr14_readparamreg(unsigned char *regval)
/******************************************************************************/
/* Function:	cr14_readparamreg					      */
/*		- Read from the CR14 parameter register			      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	unsigned char	status	= 0;

	i2c_start();

	if (!i2c_address(I2C_ADDR, I2C_WR)) {
		status = CR14_BUSY;
		goto stop;
	}

	if (!i2c_write(PARAM_REG)) {
		status = CR14_NACK;
		goto stop;
	}

	i2c_restart();

	if (!i2c_address(I2C_ADDR, I2C_RD)) {
		status = CR14_NACK;
		goto stop;
	}

	i2c_read(regval, 0);

stop:
	i2c_stop();
	return status;
}
/* End: cr14_writeparamreg */


unsigned char cr14_writeframe(unsigned char *frame_ptr, unsigned char frame_len)
/******************************************************************************/
/* Function:	cr14_writeframe						      */
/*		- Write a frame of data to the CR14			      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	unsigned char	status	= 0;
	unsigned char	index	= 0;

	i2c_start();

	if (!i2c_address(I2C_ADDR, I2C_WR)) {
		status = CR14_BUSY;
		goto stop;
	}

	if (!i2c_write(FRAME_REG)) {
		status = CR14_NACK;
		goto stop;
	}

	if (!i2c_write(frame_len)) {
		status = CR14_NACK;
		goto stop;
	}

	while (index < frame_len) {
		if (!i2c_write(frame_ptr[index])) {
			status = CR14_NACK;
			goto stop;
		}
		index++;
	}

stop:
	i2c_stop();
	return status;
}
/* End: cr14_writeframe */


unsigned char cr14_readframe(unsigned char *frame_ptr, unsigned char *frame_len)
/******************************************************************************/
/* Function:	cr14_readframe						      */
/*		- Read a frame of data from the CR14			      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	unsigned char	status	= 0;
	unsigned char	length	= 0;
	unsigned char	index	= 0;

	i2c_start();

	if (!i2c_address(I2C_ADDR, I2C_WR)) {
		status = CR14_BUSY;
		goto stop;
	}

	if (!i2c_write(FRAME_REG)) {
		status = CR14_NACK;
		goto stop;
	}

	i2c_restart();

	if (!i2c_address(I2C_ADDR, I2C_RD)) {
		status = CR14_NACK;
		goto stop;
	}

	i2c_read(&length, 1);

	if (!length)
		goto nack;

	if (length == 0xFF) {
		status = CR14_CRCERR;
		goto nack;
	}

	if (length > *frame_len)
		length = *frame_len;

	while (index < length) {
		i2c_read(&frame_ptr[index], (index+1) < length);
		index++;
	}

	*frame_len = index;
	goto stop;

nack:
	*frame_len = 0;

	/* Dummy read, just to nack */
	i2c_read(0, 0);

stop:
	i2c_stop();
	return status;
}
/* End: cr14_readframe */

/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

