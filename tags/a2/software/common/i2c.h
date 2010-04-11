/******************************************************************************/
/* File    :	i2c.h							      */
/* Function:	Include file of 'i2c.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef I2C_H					/* Include file already compiled? */
#define I2C_H

#define I2C_RD		1
#define I2C_WR		0

/* Generic */
void		i2c_init		(void) ;
void		i2c_work		(void) ;
void		i2c_term		(void) ;

unsigned char	i2c_busy		(void) ;
void		i2c_start		(void) ;
void		i2c_restart		(void) ;
void		i2c_address		(unsigned char	byte,
					 unsigned char	read) ;
void		i2c_read		(unsigned char	ackbyte) ;
unsigned char	i2c_getbyte		(void) ;
void		i2c_write		(unsigned char	byte) ;
void		i2c_stop		(void) ;
unsigned char	i2c_acked		(void) ;

#endif /* I2C_H */
