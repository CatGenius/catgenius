/******************************************************************************/
/* File    :	cr14.h							      */
/* Function:	Include file of 'cr14.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CR14_H					/* Include file already compiled? */
#define CR14_H

/* Generic */
void		cr14_init		(void) ;
void		cr14_work		(void) ;
void		cr14_term		(void) ;

/* Status */
unsigned char	cr14_busy		(void) ;

/* Operations */
void		cr14_writeparamreg	(unsigned char	regval) ;
void		cr14_readparamreg	(void) ;
unsigned char	cr14_paramreg		(void) ;
void		cr14_writeframe		(unsigned char	*frame_ptr,
					 unsigned char	frame_len) ;
void		cr14_readframe		(void) ;
unsigned char	cr14_getframe		(unsigned char	*frame_ptr) ;

#endif /* CR14_H */
