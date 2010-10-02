/******************************************************************************/
/* File    :	catgenius.c						      */
/* Function:	CatGenius Application					      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>

#include "../common/timer.h"
#include "../common/rtc.h"
#include "../common/hardware.h"
#include "../common/serial.h"
#include "../common/i2c.h"
#include "../common/cr14.h"
#include "../common/srix4k.h"
#include "../common/catsensor.h"
#include "../common/watersensor.h"
#include "../common/catgenie120.h"
#include "userinterface.h"
#include "litterlanguage.h"

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

	printf("\n*** CatGenius ***\n");
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

	if (flags & START_BUTTON_HELD)
		DBG("Start button held\n");
	if (flags & SETUP_BUTTON_HELD)
		DBG("Setup button held\n");

	/* Initialize software timers */
	timer_init();

	/* Initialize real time clock */
	rtc_init(flags);

	/* Initialize the I2C bus */
	i2c_init();

	/* Initialize the RFID tag */
//	srix4k_init();

	/* Initialize the cat sensor */
	catsensor_init();

	/* Initialize the cat sensor */
	watersensor_init();

	/* Initialize the user interface */
	userinterface_init(flags);

	/* Initialize the washing program */
	litterlanguage_init(flags);

	/* Initialize interrupts */
	interrupt_init();

	/* Execute the run loop */
	for(;;){
		rtc_work();
		catsensor_work();
		watersensor_work();
		catgenie_work();
		userinterface_work();
		litterlanguage_work();
		srix4k_work();

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
			} else {
				rxd = RCREG;
				switch (rxd) {
				case 'm':
					incminutes();
					printtime();
					DBG("\n");
					break;
				case 'h':
					inchours();
					printtime();
					DBG("\n");
					break;
				case 'w':
					incweekday();
					printtime();
					DBG("\n");
					break;
				case '\n':
					printtime();
					DBG("\n");
					break;
				}
			}
		}
#ifndef __DEBUG
		CLRWDT();
#endif
	}
}

void heatsensor_event (unsigned char detected)
/******************************************************************************/
/* Function:	heatsensor_event					      */
/*		- Handle state changes of over-heat sensor		      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* heatsensor_event */


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
