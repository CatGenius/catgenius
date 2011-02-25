/******************************************************************************/
/* File    :	geniediag.c						      */
/* Function:	CatGenius Diagnostics Application			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>

#include "../common/hardware.h"		/* Flexible hardware configuration */

#include "../common/timer.h"
#include "../common/serial.h"
#include "../common/catsensor.h"
#include "userinterface.h"

/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#ifdef __DEBUG
__CONFIG(XT & WDTDIS & PWRTEN & BORDIS & LVPDIS & DUNPROT & WRTEN & DEBUGDIS & UNPROTECT);
#else
__CONFIG(XT & WDTEN  & PWRTEN & BOREN  & LVPDIS & DPROT   & WRTEN & DEBUGDIS & PROTECT);
#endif


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

#ifdef __RESETBITS_ADDR
extern bit		__powerdown;
extern bit		__timeout;
#endif /* __RESETBITS_ADDR */

static unsigned char	PORTB_old;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void dumpports(void)
{
	printf("TRISB = 0x%02X, PORTB = 0x%02X\n", TRISB, PORTB);
	printf("TRISC = 0x%02X, PORTC = 0x%02X\n", TRISC, PORTC);
	printf("TRISD = 0x%02X, PORTD = 0x%02X\n", TRISD, PORTD);
	printf("TRISE = 0x%02X, PORTE = 0x%02X\n", TRISE, PORTE);
}

static void interrupt_init (void);

/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void main (void)
{
	unsigned char	flags;

	/* Init the hardware */
	flags = catgenie_init();

	/* Initialize the serial port */
	serial_init();

	printf("\n*** GenieDiag ***\n");
	if (!POR) {
		DBG("Power-on reset\n");
		flags |= POWER_FAILURE;
	} else if (!BOR) {
		DBG("Brown-out reset\n");
		flags |= POWER_FAILURE;
	}
#ifdef __RESETBITS_ADDR
	else if (!__timeout)
		DBG("Watchdog reset\n");
	else if (!__powerdown)
		DBG("Pin reset (sleep)\n");
	else
		DBG("Pin reset\n");
#else
	else
		DBG("Unknown reset\n");
#endif /* __RESETBITS_ADDR */
	POR = 1;
	BOR = 1;
	if (flags & START_BUTTON)
		DBG("Start button held\n");
	if (flags & SETUP_BUTTON)
		DBG("Setup button held\n");

	/* Initialize software timers */
	timer_init();

	/* Initialize the cat sensor */
	catsensor_init();

	/* Initialize the user interface */
	userinterface_init();

	/* Initialize interrupts */
	interrupt_init();

	/* Execute the run loop */
	for(;;){
		catsensor_work();
		catgenie_work();
		userinterface_work();

		while (RCIF) {
			char rxd ;
			if (OERR)
			{
				TXEN = 0;
				TXEN = 1;
				CREN = 0;
				CREN = 1;
			}
			if (FERR)
			{
				rxd = RCREG;
				TXEN = 0;
				TXEN = 1;
			} else
				if (RCREG == '\n')
					dumpports();
		}
#ifndef __DEBUG
		CLRWDT();
#endif
	}
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void interrupt_init (void)
{
	PORTB_old = PORTB;

	/* Enable peripheral interrupts */
	PEIE = 1;

	/* Enable interrupts */
	GIE = 1;
}

static void interrupt isr (void)
{
	unsigned char temp;

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
		catsensor_isr_timer();
	}
	/* Port B interrupt */
	if (RBIF) {
		/* Reset interrupt */
		RBIF = 0;
		/* Detected changes */
		temp = PORTB ^ PORTB_old;
		/* Handle interrupt */
		if (temp & CATSENSOR_MASK)
			catsensor_isr_input();
		/* Update the old status */
		PORTB_old = PORTB ;
	}
}
