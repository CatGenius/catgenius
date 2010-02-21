/******************************************************************************/
/* File    :	washprogram.h						      */
/* Function:	Header file of 'washprogram.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef WASHPROGRAM_H				/* Include file already compiled? */
#define WASHPROGRAM_H


/* Generic */
void		washprogram_init	(void) ;
void		washprogram_work	(void) ;
void		washprogram_term	(void) ;

/* Control */
void		washprogram_mode	(unsigned char	justscoop) ;
void		washprogram_start	(void) ;
unsigned char	washprogram_running	(void) ;
void		washprogram_pause	(unsigned char	pause) ;


#endif /* WASHPROGRAM_H */
