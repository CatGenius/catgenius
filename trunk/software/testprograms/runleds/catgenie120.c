/******************************************************************************/
/* File    :	timer.h							      */
/* Function:	Timer module						      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "catgenie120.h"
#include "timer.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define BIT(n)		(1U << (n))	/* Bit mask for bit 'n' */

#define TRISx0_OUT	(0 << 0)
#define TRISx0_IN	(1 << 0)
#define TRISx1_OUT	(0 << 1)
#define TRISx1_IN	(1 << 1)
#define TRISx2_OUT	(0 << 2)
#define TRISx2_IN	(1 << 2)
#define TRISx3_OUT	(0 << 3)
#define TRISx3_IN	(1 << 3)
#define TRISx4_OUT	(0 << 4)
#define TRISx4_IN	(1 << 4)
#define TRISx5_OUT	(0 << 5)
#define TRISx5_IN	(1 << 5)
#define TRISx6_OUT	(0 << 6)
#define TRISx6_IN	(1 << 6)
#define TRISx7_OUT	(0 << 7)
#define TRISx7_IN	(1 << 7)

#define DEBOUNCE	(SECOND/20)	/* 50ms */

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	startbutton_debounce = {0xFFFF, 0xFFFFFFFF};
static struct timer	setupbutton_debounce = {0xFFFF, 0xFFFFFFFF};
static struct timer	catsensor_debounce   = {0xFFFF, 0xFFFFFFFF};


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void init_catgenie (void)
/******************************************************************************/
/* Function:	init_catgenie						      */
/*		- Initialize the CatGenie 120 hardware			      */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	/*
	 * Setup port A
	 */
	TRISA = TRISx0_IN  |	/* Not used (R39, Absent) */
		TRISx1_IN  |	/* Cat Sensor? (U6 Pin2) */
		TRISx2_OUT |	/* LED 2 */
		TRISx3_OUT |	/* LED 3 */
		TRISx4_IN  |	/* Not used (R1, Absent) */
		TRISx5_OUT ;	/* LED 4 */
	PORTA = 0x00;
	ADCON1 = 0x07;

	/*
	 * Setup port B
	 */
	TRISB = TRISx0_IN  |	/* Button Start/Pause */
		TRISx1_IN  |	/* Over heat detector (U4) */
		TRISx2_OUT |	/* LED Water sensor */
		TRISx3_OUT |	/* Relay Water valve (RL1) */
		TRISx4_IN  |	/* Cat Sensor */
		TRISx5_IN  |	/* Button Auto setup */
		TRISx6_IN  |	/* PGM Clock */
		TRISx7_IN  ;	/* PGM Data */
	PORTB = 0x00;
	/* Turn on all internal weak pull-up resitors */
	RBPU = 0;
	/* Clear the interrupt status */
	INTF = 0;
	RBIF = 0;
	/* Enable interrupts */
	INTE = 1;
	RBIE = 1;

	/*
	 * Setup port C
	 */
	TRISC = TRISx0_OUT |	/* LED Error */
		TRISx1_OUT |	/* Beeper */
		TRISx2_OUT |	/* Cat Sensor LED */
		TRISx3_IN  |	/* I2C SCL */
		TRISx4_IN  |	/* I2C SDA */
		TRISx5_OUT |	/* LED 1 */
		TRISx6_IN  |	/* UART TxD */
		TRISx7_IN  ;	/* UART RxD */
	PORTC = 0x00;

	/*
	 * Setup port D
	 */
	TRISD = TRISx0_IN  |	/* Water Sensor */
		TRISx1_OUT |	/* Pump on/off (RL3) */
		TRISx2_OUT |	/* Blow dryer on/off (RL2) */
		TRISx3_OUT |	/* Dosage pump on/off (RL4) */
		TRISx4_OUT |	/* Bowl cw/ccw (RL7) */
		TRISx5_OUT |	/* Bowl on/off (RL5) */
		TRISx6_OUT |	/* Scooper up/down (RL8) */
		TRISx7_OUT ;	/* Scooper on/off (RL6) */
	PORTD = 0x00;

	/*
	 * Setup port E
	 */
	TRISE = TRISx0_OUT |	/* LED Cartridge */
		TRISx1_OUT |	/* LED Cat */
		TRISx2_OUT ;	/* LED Child Lock */
	PORTE = 0x00;
}
/* End: init_catgenie */


void do_catgenie (void)
/******************************************************************************/
/* Function:	do_catgenie						      */
/*		- Worker function for the CatGenie 120 hardware		      */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	/* Handle the debounced Start/Pause button */
	if (&startbutton_debounce) {
		timeoutnever(&startbutton_debounce);
		if (PORTB & 0x01) {
		}
	}
	/* Handle the debounced Auto setup button */
	if (&setupbutton_debounce) {
		timeoutnever(&setupbutton_debounce);
		if (PORTB & 0x20)
			set_LED_Locked(1) ;
		else
			set_LED_Locked(0) ;
	}
	/* Handle the debounced cat sensor */
	if (&catsensor_debounce) {
		timeoutnever(&catsensor_debounce);
		if (PORTB & 0x10) {
		}
	}
}

void term_catgenie (void)
/******************************************************************************/
/* Function:	term_catgenie						      */
/*		- Initialize the CatGenie 120 hardware			      */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* End: term_catgenie */


void startbutton_isr (void)
/******************************************************************************/
/* Function:	startbutton_isr						      */
/*		- Handle state changes of the Start/Pause button	      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	settimeout(&startbutton_debounce, DEBOUNCE);
}
/* startbutton_isr */


void setupbutton_isr (void)
/******************************************************************************/
/* Function:	setupbutton_isr						      */
/*		- Handle state changes of the Auto setup button		      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	settimeout(&setupbutton_debounce, DEBOUNCE);
}
/* setupbutton_isr */


void catsensor_isr (void)
/******************************************************************************/
/* Function:	catsensor_isr						      */
/*		- Handle state changes of cat sensor			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* catsensor_isr */


void set_LED (unsigned char led, unsigned char on)
{
	switch(led) {
	case 1:
		if (on)
			PORTC |= BIT(5);
		else
			PORTC &= ~BIT(5);
		break;
	case 2:
		if (on)
			PORTA |= BIT(2);
		else
			PORTA &= ~BIT(2);
		break;
	case 3:
		if (on)
			PORTA |= BIT(3);
		else
			PORTA &= ~BIT(3);
		break;
	case 4:
		if (on)
			PORTA |= BIT(5);
		else
			PORTA &= ~BIT(5);
		break;
	}
}

void set_LED_Error (unsigned char on)
{
	if (on)
		PORTC |= BIT(0);
	else
		PORTC &= ~BIT(0);
}

void set_LED_Cartridge (unsigned char on)
{
	if (on)
		PORTE |= BIT(0);
	else
		PORTE &= ~BIT(0);
}

void set_LED_Cat (unsigned char on)
{
	if (on)
		PORTE |= BIT(1);
	else
		PORTE &= ~BIT(1);
}

void set_LED_Locked (unsigned char on)
{
	if (on)
		PORTE |= BIT(2);
	else
		PORTE &= ~BIT(2);
}

void set_Beeper (unsigned char on)
{
	if (on)
		PORTC |= BIT(1);
	else
		PORTC &= ~BIT(1);
}

/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

