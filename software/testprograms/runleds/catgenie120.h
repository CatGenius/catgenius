/******************************************************************************/
/* File    :	catgenie120.h						      */
/* Function:	Include file of 'catgenie120.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CATGENIE120_H			/* Include file already compiled? */
#define CATGENIE120_H

void		init_catgenie		(void) ;
void		do_catgenie		(void) ;
void		term_catgenie		(void) ;

void		startbutton_isr		(void) ;
void		setupbutton_isr		(void) ;
void		catsensor_isr		(void) ;

void		set_LED			(unsigned char led,
					 unsigned char on);
void		set_LED_Cat		(unsigned char pattern,
					 unsigned char repeat);
void		set_LED_Error		(unsigned char pattern,
					 unsigned char repeat);
void		set_LED_Cartridge	(unsigned char pattern,
					 unsigned char repeat);
void		set_LED_Locked		(unsigned char pattern,
					 unsigned char repeat);
void		set_Beeper		(unsigned char pattern,
					 unsigned char repeat) ;

#endif /* CATGENIE120_C */
