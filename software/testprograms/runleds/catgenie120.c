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

#define BIT(n)			(1U << (n))	/* Bit mask for bit 'n' */

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

/* Buttons */
#define STARTBUTTON_PORT	PORTB
#define STARTBUTTON_MASK	BIT(0)
#define SETUPBUTTON_PORT	PORTB
#define SETUPBUTTON_MASK	BIT(5)
#define CATSENSOR_PORT		PORTB
#define CATSENSOR_MASK		BIT(4)

/* Indicators */
#define BEEPER_PORT		PORTC
#define BEEPER_MASK		BIT(1)
#define LED_ERROR_PORT		PORTC
#define LED_ERROR_MASK		BIT(0)
#define LED_LOCKED_PORT		PORTE
#define LED_LOCKED_MASK		BIT(2)
#define LED_CARTRIDGE_PORT	PORTE
#define LED_CARTRIDGE_MASK	BIT(0)
#define LED_CAT_PORT		PORTE
#define LED_CAT_MASK		BIT(1)

/* Actuators */

/* Debouncers */
#define BUTTON_DEBOUNCE		(SECOND/20)	/* 50ms */
#define CATSENSOR_DEBOUNCE	(SECOND)	/* 1000ms */
#define DEBOUNCER_BUTTON_START	0
#define DEBOUNCER_BUTTON_SETUP	1
#define DEBOUNCER_SENSOR_CAT	2
#define DEBOUNCER_MAX		3

/* Pacers */
#define PACER_BITTIME		(SECOND/5)	/* 200ms */
#define PACER_BEEPER		0
#define PACER_LED_ERROR		1
#define PACER_LED_LOCKED	2
#define PACER_LED_CARTRIDGE	3
#define PACER_LED_CAT		4
#define PACER_MAX		5


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

struct debouncer {
	struct timer	timer;
	unsigned long	timeout;
	unsigned char	state;
	volatile char	*port;
	unsigned char	port_mask;
}
static struct debouncer	debouncers[DEBOUNCER_MAX] = {
	{{0xFFFF, 0xFFFFFFFF}, BUTTON_DEBOUNCE,    0, &STARTBUTTON_PORT, STARTBUTTON_MASK},
	{{0xFFFF, 0xFFFFFFFF}, BUTTON_DEBOUNCE,    0, &SETUPBUTTON_PORT, SETUPBUTTON_MASK},
	{{0xFFFF, 0xFFFFFFFF}, CATSENSOR_DEBOUNCE, 0, &CATSENSOR_PORT,   CATSENSOR_MASK}
};

struct pacer {
	struct timer	timer;
/*	unsigned long	timeout;*/
	unsigned char	mask;
	unsigned char	pattern;
	unsigned char	repeat;
	volatile char	*port;
	unsigned char	port_mask;
}
static struct pacer	pacers[PACER_MAX] = {
	{{0x0000, 0x00000000}, 0x01, 0x00, 0, &BEEPER_PORT,        BEEPER_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x55, 1, &LED_ERROR_PORT,     LED_ERROR_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x00, 0, &LED_LOCKED_PORT,    LED_LOCKED_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x00, 0, &LED_CARTRIDGE_PORT, LED_CARTRIDGE_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x00, 0, &LED_CAT_PORT,       LED_CAT_MASK}
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
	unsigned char	index ;

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

	/* Copy the initial states into the debouncer states */
	for (index = 0; index < DEBOUNCER_MAX; index++) {
		unsigned char	tempmask = debouncers[index].port_mask; /* for compiler limitations */

		debouncers[index].state = *debouncers[index].port & tempmask;
	}
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

	/* Execute the debouncers */
	for (index = 0; index < DEBOUNCER_MAX; index++)
		if (timeoutexpired(&debouncers[index].timer)) {
			unsigned char	tempstate = *debouncers[index].port; /* for compiler limitations */
			tempstate &= debouncers[index].port_mask;
			/* Check if the button state changed */
			if (tempstate != debouncers[index].state) {
				set_Beeper(1,0);

				debouncers[index].state = tempstate;
			}
			timeoutnever(&debouncers[index].timer);
		}

	/* Execute the pacers */
	for (index = 0; index < PACER_MAX; index++)
		if (timeoutexpired(&pacers[index].timer)) {
			unsigned char	tempmask = pacers[index].mask; /* for compiler limitations */

			/* Set a time for the next execution time */
			settimeout(&pacers[index].timer, PACER_BITTIME);
			/* Copy the current pattern bit to the beeper */
			if (pacers[index].pattern & tempmask)
				*pacers[index].port |= pacers[index].port_mask;
			else
				*pacers[index].port &= ~pacers[index].port_mask;
			/* Update the current bit */
			if (!(pacers[index].mask <<= 1)) {
				pacers[index].mask = 1;
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
	settimeout(&debouncers[DEBOUNCER_BUTTON_START].timer,
		   debouncers[DEBOUNCER_BUTTON_START].timeout);
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
	settimeout(&debouncers[DEBOUNCER_BUTTON_SETUP].timer,
		   debouncers[DEBOUNCER_BUTTON_SETUP].timeout);
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
	settimeout(&debouncers[DEBOUNCER_SENSOR_CAT].timer,
		   debouncers[DEBOUNCER_SENSOR_CAT].timeout);
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

void set_LED_Error (unsigned char pattern, unsigned char repeat)
{
	/* Reset the mask to the first bit */
	pacers[PACER_LED_ERROR].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_ERROR].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_ERROR].repeat = repeat;
}

void set_LED_Locked (unsigned char pattern, unsigned char repeat)
{
	/* Reset the mask to the first bit */
	pacers[PACER_LED_LOCKED].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_LOCKED].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_LOCKED].repeat = repeat;
}

void set_LED_Cartridge (unsigned char pattern, unsigned char repeat)
{
	/* Reset the mask to the first bit */
	pacers[PACER_LED_CARTRIDGE].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_CARTRIDGE].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_CARTRIDGE].repeat = repeat;
}

void set_LED_Cat (unsigned char pattern, unsigned char repeat)
{
	/* Reset the mask to the first bit */
	pacers[PACER_LED_CAT].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_CAT].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_CAT].repeat = repeat;
}

void set_Beeper (unsigned char pattern, unsigned char repeat)
{
	/* Reset the mask to the first bit */
	pacers[PACER_BEEPER].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_BEEPER].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_BEEPER].repeat = repeat;
}

/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

