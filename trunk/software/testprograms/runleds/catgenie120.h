/******************************************************************************/
/* File    :	catgenie120.h						      */
/* Function:	Include file of 'catgenie120.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CATGENIE120_H			/* Include file already compiled? */
#define CATGENIE120_H


/* Bowl modi */
#define BOWL_STOP		0
#define BOWL_CW			1
#define BOWL_CCW		2

/* Arm modi */
#define ARM_STOP		0
#define ARM_DOWN		1
#define ARM_UP			2


/* Generic */
void		catgenie_init		(void) ;
void		catgenie_work		(void) ;
void		catgenie_term		(void) ;

/* Event notification */
void		catsensor_event		(unsigned char detected) ;
void		watersensor_isr		(void) ;
void		heatsensor_isr		(void) ;

/* Indicators */
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

/* Actuators */
void		set_Bowl		(unsigned char mode);
void		set_Arm			(unsigned char mode);
void		set_Water		(unsigned char on);
void		set_Dosage		(unsigned char on);
void		set_Pump		(unsigned char on);
void		set_Dryer		(unsigned char on);

#endif /* CATGENIE120_H */
