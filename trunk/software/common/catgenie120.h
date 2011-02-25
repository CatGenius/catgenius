/******************************************************************************/
/* File    :	catgenie120.h						      */
/* Function:	Include file of 'catgenie120.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CATGENIE120_H			/* Include file already compiled? */
#define CATGENIE120_H


/* Init return flags */
#define START_BUTTON		0x01
#define SETUP_BUTTON		0x02
#define RESERVED_1		0x04
#define RESERVED_2		0x08
#define RESERVED_3		0x10
#define RESERVED_4		0x20
#define RESERVED_5		0x40
#define POWER_FAILURE		0x80


/* Bowl modi */
#define BOWL_STOP		0
#define BOWL_CW			1
#define BOWL_CCW		2

/* Arm modi */
#define ARM_STOP		0
#define ARM_DOWN		1
#define ARM_UP			2


/* Generic */
unsigned char	catgenie_init		(void) ;
void		catgenie_work		(void) ;
void		catgenie_term		(void) ;

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
unsigned char	get_Bowl		(void);
void		set_Arm			(unsigned char mode);
unsigned char	get_Arm			(void);
void		set_Water		(unsigned char on);
unsigned char	get_Water		(void);
void		set_Dosage		(unsigned char on);
unsigned char	get_Dosage		(void);
void		set_Pump		(unsigned char on);
unsigned char	get_Pump		(void);
void		set_Dryer		(unsigned char on);
unsigned char	get_Dryer		(void);

#endif /* CATGENIE120_H */
