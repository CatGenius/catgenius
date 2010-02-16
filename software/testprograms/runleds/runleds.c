/******************************************************************************/
/* File    :	runleds.c						      */
/* Function:	CatGenie test program					      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "timer.h"
#include "catsensor.h"
#include "catgenie120.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

__CONFIG(XT & WDTDIS & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);
//__CONFIG(XT & WDTEN & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static unsigned char	PORTB_old;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void interrupt_init (void);
//static void isr (void);


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void main (void)
{
	/* Init the hardware */
	catgenie_init();
	PORTB_old = PORTB;

	/* Initialize software timers */
	timer_init();

	/* Initialize the cat sensor */
	catsensor_init();

	/* Initialize interrupts */
	interrupt_init();

	/* Execute the run loop */
	for(;;){
		catsensor_work();
		catgenie_work();
	}
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void interrupt_init (void)
{
	/* Enable peripheral interrupts */
	PEIE = 1;

	/* Enable interrupts */
	GIE = 1;
}

static void interrupt isr (void)
{
	/* Timer 1 interrupt */
	if (TMR1IF) {
		/* Reset interrupt */
		TMR1IF = 0;
		/* Handle interrupt */
		timer_isr();
	}
	/* Timer 2 interrupt */
	if (TMR2IF) {
		/* Reset interrupt */
		TMR2IF = 0;
		/* Handle interrupt */
		catsensor_isr();
	}
	/* Port B interrupt */
	if (RBIF) {
		unsigned char	PORTB_diff = PORTB ^ PORTB_old;
		/* Reset interrupt */
		RBIF = 0;
		/* Handle interrupt */
//		if (PORTB_diff & 0x10)
//			catsensor_isr();
//		if (PORTB_diff & 0x20)
//			setupbutton_isr();
		PORTB_old = PORTB ;
	}
}
