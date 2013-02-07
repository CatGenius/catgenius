/******************************************************************************/
/* File    :	cr14.h							      */
/* Function:	Include file of 'cr14.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#include "../common/app_prefs.h"

#if !(defined CR14_H) && (defined HAS_CR14)					/* Include file already compiled? */
#define CR14_H


#define CR14_OK		0
#define CR14_BUSY	1
#define CR14_NACK	2
#define CR14_CRCERR	3


/* Operations */
unsigned char	cr14_writeparamreg	(unsigned char	regval) ;
unsigned char	cr14_readparamreg	(unsigned char	*regval) ;
unsigned char	cr14_writeframe		(unsigned char	*frame_ptr,
					 unsigned char	frame_len) ;
unsigned char	cr14_readframe		(unsigned char	*frame_ptr,
					 unsigned char	*frame_len) ;

#endif /* CR14_H */
