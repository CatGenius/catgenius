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
#include "catgenie120.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

__CONFIG(XT & WDTDIS & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);
//__CONFIG(XT & WDTEN & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);

#define NO_INTERRUPTS


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static unsigned char	PORTB_old;
static unsigned char	test		= 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

#ifdef NO_INTERRUPTS
static void isr (void);
#endif /* NO_INTERRUPTS */


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void init (void)
{
	unsigned char temp;

	/* Init the hardware */
	init_catgenie();
	PORTB_old = PORTB;

	/* initialize timer 1 */
	timer_init();

	/* Enable peripheral interrupts */
	PEIE = 1;

	/* Enable interrupts */
	GIE = 1;
}

void main (void)
{
	/* Initialize the hardware */
	init();

CCP1M0 = 1;
CCP1M1 = 1;
CCP1M2 = 1;
CCP1M3 = 1;
PR2 = 0b01010100 ;
T2CON = 0b00000101 ;
CCPR1L = 0b00101010 ;
CCP1CON = 0b00011100 ;

TOUTPS0 = 1;
TOUTPS1 = 1;
TOUTPS2 = 1;
TOUTPS3 = 1;
TMR2IF = 0;
TMR2IE = 1;


	/* Execute the run loop */
	for(;;){
#ifdef NO_INTERRUPTS
		isr();
#endif /* NO_INTERRUPTS */

		do_catgenie();
	}
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

#ifdef NO_INTERRUPTS
static void isr (void)
#else
static void interrupt isr (void)
#endif /* NO_INTERRUPTS */
{
	if (TMR1IF) {
		/* Reset interrupt */
		TMR1IF = 0;
		/* Handle interrupt */
		timer_isr();
	}
	if (TMR2IF) {
		/* Reset interrupt */
		TMR2IF = 0;
		/* Handle interrupt */
		TRISC ^= 0x04;
		if (TRISC & 0x04) {
			T2CKPS1 = 1;
			T2CKPS0 = 1;
		} else {
			T2CKPS1 = 0;
			T2CKPS0 = 1;
		}
	}
	if (INTF) {
		/* Reset interrupt */
		INTF = 0;
		/* Handle interrupt */
//		startbutton_isr();
	}
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
