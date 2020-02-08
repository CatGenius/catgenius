/******************************************************************************/
/* File    :	serial.h						      */
/* Function:	Include file of 'serial.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef SERIAL_H			/* Include file already compiled? */
#define SERIAL_H

void		serial_init	(unsigned long	bitrate);
void		serial_term	(void);
void		serial_rx_isr	(void);
void		serial_tx_isr	(void);
void		putch		(unsigned char	c);
unsigned char	readch		(char		*ch);

#endif /* SERIAL_H */
