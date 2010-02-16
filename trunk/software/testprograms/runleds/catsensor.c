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
#include "timer.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define BIT(n)			(1U << (n))	/* Bit mask for bit 'n' */

#define LED_CAT_PORT		PORTE
#define LED_CAT_MASK		BIT(1)

#define CATDET_LED_PORT		PORTC
#define CATDET_LED_MASK		BIT(2)
#define CATSENSOR_PORT		PORTB
#define CATSENSOR_MASK		BIT(4)


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

#define PULSETOL	4
struct timer		catsensor = {0xFFFF, 0xFFFFFFFF};
unsigned char		catsensorpulses = 0;

static unsigned char	portb_old;


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

	/* Select Fosc/4 as source */
//	TMR1CS = 0;
	/* Set prescaler to 1:8 */
//	T1CKPS1 = 1;
//	T1CKPS0 = 1;
	/* Disable timer 1 external oscillator */
//	T1OSCEN = 0;
	/* Disable synchronized clock */
//	T1SYNC = 0;
	/* Switch on timer 1 */
//	TMR1ON = 1;
	/* Enable timer 1 interrupt */
//	TMR1IE = 1;

//	portb_old = PORTB;

}
/* End: catsensor_init */


void catsensor_work (void)
/******************************************************************************/
/* Function:	Module worker routine					      */
/*		- Terminates the module					      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	unsigned char	index ;
	unsigned char	status ;

	/* Poll critical Port B inputs for changes */
	status    = PORTB;
	index     = status ^ portb_old;
	portb_old = status;
	
	if (index & CATSENSOR_MASK) {
		if (CATSENSOR_PORT & CATSENSOR_MASK) {
			settimeout(&catsensor, (PULSETOL * SECOND)/33);
			if (catsensorpulses < 255)
				catsensorpulses++;
		}
	}

	if (catsensorpulses){
		if(timeoutexpired(&catsensor)) {
			/* Cat gone */
			catsensorpulses = 0;
LED_CAT_PORT &= ~LED_CAT_MASK;
		}
		if (catsensorpulses > PULSETOL) {
			/* Cat here */
LED_CAT_PORT |= LED_CAT_MASK;
		}
	}

} /* catsensor_work */


void catsensor_term (void)
/******************************************************************************/
/* Function:	Module initialisation routine			      */
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



/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

void catsensor_isr (void)
/******************************************************************************/
/* Function:	Interrupt service routine				      */
/*		- Each interrupt will increment the 32-bit ticks counter.     */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	TRISC ^= 0x04;
	if (TRISC & 0x04) {
		T2CKPS1 = 1;
		T2CKPS0 = 1;
	} else {
		T2CKPS1 = 0;
		T2CKPS0 = 1;
	}
}
/* End: catsensor_isr */
