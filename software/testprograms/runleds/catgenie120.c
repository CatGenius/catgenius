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

#define TRISx0_OUT		(0 << 0)
#define TRISx0_IN		(1 << 0)
#define TRISx1_OUT		(0 << 1)
#define TRISx1_IN		(1 << 1)
#define TRISx2_OUT		(0 << 2)
#define TRISx2_IN		(1 << 2)
#define TRISx3_OUT		(0 << 3)
#define TRISx3_IN		(1 << 3)
#define TRISx4_OUT		(0 << 4)
#define TRISx4_IN		(1 << 4)
#define TRISx5_OUT		(0 << 5)
#define TRISx5_IN		(1 << 5)
#define TRISx6_OUT		(0 << 6)
#define TRISx6_IN		(1 << 6)
#define TRISx7_OUT		(0 << 7)
#define TRISx7_IN		(1 << 7)

#define STARTBUTTON_PORT	PORTB
#define STARTBUTTON_BIT		(1 << 0)
#define SETUPBUTTON_PORT	PORTB
#define SETUPBUTTON_BIT		(1 << 5)
#define CATSENSOR_PORT		PORTB
#define CATSENSOR_BIT		(1 << 4)

#define BEEPER_PORT		PORTC
#define BEEPER_MASK		BIT(1)
#define LED_ERROR_PORT		PORTC
#define LED_ERROR_MASK		BIT(0)
#define LED_LOCK_PORT		PORTE
#define LED_LOCK_MASK		BIT(2)
#define LED_CARTRIDGE_PORT	PORTE
#define LED_CARTRIDGE_MASK	BIT(0)
#define LED_CAT_PORT		PORTE
#define LED_CAT_MASK		BIT(1)

#define BUTTON_DEBOUNCE		(SECOND/20)	/* 50ms */
#define CATSENSOR_DEBOUNCE	(SECOND)	/* 1000ms */

#define PACER_BITTIME		(SECOND/5)	/* 200ms */
#define PACER_BEEPER		0
#define PACER_LED_ERROR		1
#define PACER_LED_LOCK		2
#define PACER_LED_CARTRIDGE	3
#define PACER_LED_CAT		4
#define PACER_MAX		5

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	startbutton_debounce = {0xFFFF, 0xFFFFFFFF};
static struct timer	setupbutton_debounce = {0xFFFF, 0xFFFFFFFF};
static struct timer	catsensor_debounce   = {0xFFFF, 0xFFFFFFFF};
static unsigned char	startbutton_state = 0;
static unsigned char	setupbutton_state = 0;
static unsigned char	catsensor_state = 0;

struct pacer {
	struct timer	bit_timer;
	unsigned char	bit_mask;
	unsigned char	pattern;
	unsigned char	repeat;
	volatile char	*port;
	unsigned char	port_mask;
}
static struct pacer	pacers[PACER_MAX] = {
	{{0x0000, 0x00000000}, 0x01, 0x00, 0, &BEEPER_PORT,        BEEPER_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x0F, 1, &LED_ERROR_PORT,     LED_ERROR_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x05, 1, &LED_LOCK_PORT,      LED_LOCK_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x50, 1, &LED_CARTRIDGE_PORT, LED_CARTRIDGE_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x0A, 1, &LED_CAT_PORT,       LED_CAT_MASK}
};


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
	/* Turn on internal weak pull-up resitors on inputs */
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

	startbutton_state = STARTBUTTON_PORT & STARTBUTTON_BIT;
	setupbutton_state = SETUPBUTTON_PORT & SETUPBUTTON_BIT;
	catsensor_state = CATSENSOR_PORT & CATSENSOR_BIT;
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
	unsigned char	index ;

	/* Handle the debounced Start/Pause button */
	if (timeoutexpired(&startbutton_debounce)) {
		timeoutnever(&startbutton_debounce);
		/* Check if the button state changed */
		if ((STARTBUTTON_PORT & STARTBUTTON_BIT) != startbutton_state) {

			startbutton_state = STARTBUTTON_PORT & STARTBUTTON_BIT;
		}
	}
	/* Handle the debounced Auto setup button */
	if (timeoutexpired(&setupbutton_debounce)) {
		timeoutnever(&setupbutton_debounce);
		/* Check if the button state changed */
		if ((SETUPBUTTON_PORT & SETUPBUTTON_BIT) != setupbutton_state) {
			set_Beeper(1,0);
			setupbutton_state = SETUPBUTTON_PORT & SETUPBUTTON_BIT;
		}
	}
	/* Handle the debounced cat sensor */
	if (timeoutexpired(&catsensor_debounce)) {
		timeoutnever(&catsensor_debounce);
		if ((CATSENSOR_PORT & CATSENSOR_BIT) != catsensor_state) {

			catsensor_state = CATSENSOR_PORT & CATSENSOR_BIT;
		}
	}
	/* Execute the pacers */
	for (index = 0; index < PACER_MAX; index++)
		if (timeoutexpired(&pacers[index].bit_timer)) {
			unsigned char tempmask = pacers[index].bit_mask;
			/* Set a time for the next execution time */
			settimeout(&pacers[index].bit_timer, PACER_BITTIME);
			/* Copy the current pattern bit to the beeper */
			if (pacers[index].pattern & tempmask)
				*pacers[index].port |= pacers[index].port_mask;
			else
				*pacers[index].port &= ~pacers[index].port_mask;
			/* Update the current bit */
			if (!(pacers[index].bit_mask <<= 1)) {
				pacers[index].bit_mask = 1;
				/* Clear the pattern if repeat is not selected */
				if (!pacers[index].repeat)
					pacers[index].pattern = 0;
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
	settimeout(&startbutton_debounce, BUTTON_DEBOUNCE);
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
	settimeout(&setupbutton_debounce, BUTTON_DEBOUNCE);
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
	settimeout(&setupbutton_debounce, CATSENSOR_DEBOUNCE);
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

void set_Beeper (unsigned char pattern, unsigned char repeat)
{
	/* Reset the mask to the first bit */
	pacers[PACER_BEEPER].bit_mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_BEEPER].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_BEEPER].repeat = repeat;
}

/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

