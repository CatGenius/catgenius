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

#define	DEBOUNCE_TIME		(SECOND/37)	/* Multiples of 27ms */


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static bit			pinged       = 0;
static bit			echoed       = 0;
static bit			detected     = 0;
static bit			detected_old = 0;
static bit			debounced    = 0;
static struct timer		debouncer    = NEVER;


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
	if (pinged) {
		/* End ping in progress */
//		TRISC |= CATSENSOR_LED_MASK;
		TRISC = I2C_SCL_MASK |		/* I2C SCL */
			I2C_SDA_MASK |		/* I2C SDA */
			UART_TXD_MASK |		/* UART TxD */
			UART_RXD_MASK |		/* UART RxD */
			CATSENSOR_LED_PORT;
		pinged = 0;
		/* The echo is the detection */
		detected = echoed;
		/* Increase prescaler for long idle */
		T2CKPS1 = 1;
	} else {
		/* Reset the echo for the new ping */
		echoed = 0;
		/* Start new ping */
//		TRISC &= ~CATSENSOR_LED_MASK;
		TRISC = I2C_SCL_MASK |		/* I2C SCL */
			I2C_SDA_MASK |		/* I2C SDA */
			UART_TXD_MASK |		/* UART TxD */
			UART_RXD_MASK ;		/* UART RxD */
		pinged = 1;
		/* Decrease prescaler for short ping */
		T2CKPS1 = 0;
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
	if ( (pinged) &&
	     (CATSENSOR_PORT & CATSENSOR_MASK) )
		/* Turn off emitter to reset the receiver */
//		TRISC |= CATSENSOR_LED_MASK;
		TRISC = I2C_SCL_MASK |		/* I2C SCL */
			I2C_SDA_MASK |		/* I2C SDA */
			UART_TXD_MASK |		/* UART TxD */
			UART_RXD_MASK ;		/* UART RxD */
		/* Store the echo */
		echoed = 1;
}
/* End: catsensor_isr_input */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/
