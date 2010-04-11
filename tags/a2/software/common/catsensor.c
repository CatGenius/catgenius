/******************************************************************************/
/* File    :	catsensor.c						      */
/* Function:	Cat sensor functional implementation			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "catsensor.h"
#include "hardware.h"
#include "timer.h"
#include "catgenie120.h"

extern void catsensor_event (unsigned char detected);


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define BIT(n)			(1U << (n))	/* Bit mask for bit 'n' */

#define	PING_TIME		(SECOND / 10)	/* Ping every 100 ms */
#define	DEBOUNCE_TIME		(3 * PING_TIME)	/* 3 pings */


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static bit			pinging		= 0;
static bit			echoed		= 0;
static bit			detected	= 0;
static bit			detected_old	= 0;
static bit			debounced	= 0;
static struct timer		debouncer	= NEVER;
static struct timer		pingtime	= EXPIRED;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void catsensor_init (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
/*		- Initializes the module				      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	/*
	 * Initialize timer 2
	 */
	/* Select frequency */
	PR2 = 0x54 ;
	/* Postscaler to 1:16 
	 * Prescaler to 1:4
	 * Timer 2 On */
	T2CON = 0x7D ;
	/* Select duty cycle  */
	CCPR1L = 0x2A ;
	/* Set PWM mode
	 * Select duty cycle  */
	CCP1CON = 0x1F ;
	/* Clear timer 2 interrupt status */
	TMR2IF = 0;
	/* Enable timer 2 interrupt */
	TMR2IE = 1;
}
/* End: catsensor_init */


void catsensor_work (void)
/******************************************************************************/
/* Function:	Module worker routine					      */
/*		- Debounces the decoupled signal and notifies on changes      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if (!pinging &&
	    timeoutexpired(&pingtime)) {
	    	/* Set timer for next ping */
		settimeout(&pingtime, PING_TIME);
		/* Reset the echo */
		echoed = 0;
		/* Set the pinging flag to enable detection */
		pinging = 1;

		/* Select frequency */
		PR2 = 0x54 ;
		/* Postscaler to 1:16 
		 * Prescaler to 1:4
		 * Timer 2 On */
		T2CON = 0x7D ;
		/* Select duty cycle  */
		CCPR1L = 0x2A ;
		/* Set PWM mode
		 * Select duty cycle  */
		CCP1CON = 0x1F ;
		/* Clear timer 2 interrupt status */
		TMR2IF = 0;
		/* Enable timer 2 interrupt */
		TMR2IE = 1;
	}
	/* Debounce the decouples 'detect' signal */
	if (detected != detected_old) {
		settimeout(&debouncer, DEBOUNCE_TIME);
		detected_old = detected;
	}

	/* Notify is changed */
	if (timeoutexpired(&debouncer)) {
		catsensor_event(detected);
		timeoutnever(&debouncer);
	}
} /* catsensor_work */


void catsensor_term (void)
/******************************************************************************/
/* Function:	Module termination routine				      */
/*		- Terminates the module					      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	/* Disable timer 2 interrupt */
	TMR2IE = 0;
	/* Stop timer 2 */
	TMR2ON = 0;
}
/* End: catsensor_term */

void catsensor_isr_timer (void)
/******************************************************************************/
/* Function:	Timer interrupt service routine				      */
/*		- Interrupt will toggle pinging				.     */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if (pinging) {
		/* End ping in progress */
		pinging = 0;
/* Disable timer 2 interrupt */
PR2 = 0 ;
/* Postscaler to 1:16 
 * Prescaler to 1:4
 * Timer 2 On */
T2CON = 0 ;
/* Select duty cycle  */
CCPR1L = 0 ;
TMR2IE = 0;
TMR2IF = 0;
TMR2ON = 0;
CCP1CON = 0;
CATSENSOR_LED_PORT &= ~CATSENSOR_LED_MASK;
		/* The echo is the detection */
		detected = echoed;
	}
}
/* End: catsensor_isr_timer */


void catsensor_isr_input (void)
/******************************************************************************/
/* Function:	Input interrupt service routine				      */
/*		- Interrupt will handle echo				.     */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if ( (pinging) &&
	     !(CATSENSOR_PORT & CATSENSOR_MASK) ) {
		echoed = 1;
	}
}
/* End: catsensor_isr_input */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/
