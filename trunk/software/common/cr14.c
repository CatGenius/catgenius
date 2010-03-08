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

static unsigned char	state	= 0;
static unsigned char	frame[FRAME_MAX];
static unsigned char	length	= 0;
static unsigned char	index	= 0;
static bit		error	= 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void cr14_init (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
/*		- Initializes the module				      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* End: cr14_init */


void cr14_work (void)
/******************************************************************************/
/* Function:	Module worker routine					      */
/*		-       */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if (i2c_busy())
		return;
	
	switch(state) {
	case IDLE:
		break;

	case RD_PARAMREG:
		i2c_start();
		state++;
		break;
	case RD_PARAMREG+1:
		i2c_address(I2C_ADDR, I2C_WR);
		state++;
		break;
	case RD_PARAMREG+2:
		i2c_write(PARAM_REG);
		state++;
		break;
	case RD_PARAMREG+3:
		i2c_restart();
		state++;
		break;
	case RD_PARAMREG+4:
		i2c_address(I2C_ADDR, I2C_RD);
		state++;
		break;
	case RD_PARAMREG+5:
		i2c_read(0);
		state++;
		break;
	case RD_PARAMREG+6:
		frame[0] = i2c_getbyte();
		i2c_stop();
		state = IDLE;
		break;

	case WR_PARAMREG:
		i2c_start();
		state++;
		break;
	case WR_PARAMREG+1:
		i2c_address(I2C_ADDR, I2C_WR);
		state++;
		break;
	case WR_PARAMREG+2:
		i2c_write(PARAM_REG);
		state++;
		break;
	case WR_PARAMREG+3:
		i2c_write(frame[0]);
		state++;
		break;
	case WR_PARAMREG+4:
		if (!i2c_acked())
			putch('N');
		i2c_stop();
		state = IDLE;
		break;

	case RD_FRAME:
		i2c_start();
		state++;
		break;
	case RD_FRAME+1:
		i2c_address(I2C_ADDR, I2C_WR);
		state++;
		break;
	case RD_FRAME+2:
		i2c_write(FRAME_REG);
		state++;
		break;
	case RD_FRAME+3:
		i2c_restart();
		state++;
		break;
	case RD_FRAME+4:
		i2c_address(I2C_ADDR, I2C_RD);
		state++;
		break;
	case RD_FRAME+5:
		i2c_read(1);
		state++;
		break;
	case RD_FRAME+6:
		length = i2c_getbyte();
		if (!length) {
			state = RD_FRAME+9;
		} else {
			if (length > FRAME_MAX)
				length = FRAME_MAX;
			index = 0;
			state++;
		}
		break;
	case RD_FRAME+7:
		if (index < length) {
			i2c_read((index+1) < length);
			state++;
		} else
			state = RD_FRAME+10;
		break ;
	case RD_FRAME+8:
		frame[index] = i2c_getbyte();
		index++;
		state--;
		break;
	case RD_FRAME+9:
		/* Dummy read, just to nack */
		i2c_read(0);
		state++;
		break;
	case RD_FRAME+10:
		i2c_stop();
		state = IDLE;
		break;

	case WR_FRAME:
		i2c_start();
		state++;
		break;
	case WR_FRAME+1:
		i2c_address(I2C_ADDR, I2C_WR);
		state++;
		break;
	case WR_FRAME+2:
		i2c_write(FRAME_REG);
		state++;
		break;
	case WR_FRAME+3:
		i2c_write(length);
		index = 0;
		state++;
		break;
	case WR_FRAME+4:
		if (index < length) {
			i2c_write(frame[index]);
			index++;
		} else
			state++;
		break;
	case WR_FRAME+5:
		if (!i2c_acked())
			putch('N');
		i2c_stop();
		state = IDLE;
		break;
	}
} /* cr14_work */


void cr14_term (void)
/******************************************************************************/
/* Function:	Module termination routine				      */
/*		- Terminates the module					      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* End: cr14_term */


unsigned char cr14_busy(void)
{
	return (state != IDLE);
}

void cr14_writeparamreg(unsigned char regval)
{
	frame[0] = regval;
	state = WR_PARAMREG;
}

void cr14_readparamreg(void)
{
	state = RD_PARAMREG;
}

unsigned char cr14_paramreg(void)
{
	return frame[0];
}

void cr14_writeframe(unsigned char *frame_ptr, unsigned char frame_len)
{
	unsigned char index;

	if (frame_len > FRAME_MAX)
		frame_len = FRAME_MAX;
	for (index = 0; index < frame_len; index++)
		frame[index] = frame_ptr[index];
	length = frame_len;
	state = WR_FRAME;
}

void cr14_readframe(void)
{
	state = RD_FRAME;
}

unsigned char cr14_getframe(unsigned char *frame_ptr)
{
	unsigned char index;

	for (index = 0; index < length; index++)
		frame_ptr[index] = frame[index];

	return length;
}

/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

