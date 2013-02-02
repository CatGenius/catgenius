/******************************************************************************/
/* File    :	catgenius.c						      */
/* Function:	CatGenius Application					      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#if (defined __PICC__)
#  include <htc.h>
#  include "configbits.h"		/* PIC MCU configuration bits, include after htc.h */
#elif (defined __XC8)
#  include "configbits.h"		/* PIC MCU configuration bits, include before anything else */
#  include <xc.h>
#else
#  error Unsupported toolchain
#endif

#include <stdio.h>

#include "../common/hardware.h"		/* Flexible hardware configuration */

#include "../common/timer.h"
#include "../common/rtc.h"
#include "../common/serial.h"
#include "../common/i2c.h"
#include "../common/catsensor.h"
#include "../common/water.h"

#ifdef HAS_COMMANDLINE
#include "../common/cmdline.h"
#endif /* HAS_COMMANDLINE */

#ifdef HAS_BLUETOOTH
#include "../common/bluetooth.h"
#endif /* HAS_BLUETOOTH */

#include "userinterface.h"
#include "litterlanguage.h"


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

#if (defined __PICC__)
extern bit		__powerdown;
extern bit		__timeout;
#endif /* __PICC__ */

#ifdef HAS_COMMANDLINE
/* command line commands */
const struct command	commands[] = {
	{"?", help},
	{"help", help},
	{"echo", echo},
	{"", NULL}
};
#endif /* HAS_COMMANDLINE */

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

	/* Initialize the hardware */
	flags = catgenie_init();

#ifdef HAS_BLUETOOTH
	serial_init(BT_BITRATE);
	bluetooth_init();
	serial_term();
#endif /* HAS_BLUETOOTH */

	/* Initialize the serial port */
	serial_init(BITRATE);

	printf("\n*** CatGenius ***\n");
	if (!nPOR) {
		DBG("Power-on reset\n");
		flags |= POWER_FAILURE;
	} else if (!nBOR) {
		DBG("Brown-out reset\n");
		flags |= POWER_FAILURE;
	} else if (!__timeout)
		DBG("Watchdog reset\n");
	else if (!__powerdown)
		DBG("Pin reset (sleep)\n");
	else
		DBG("Pin reset\n");
	nPOR = 1;
	nBOR = 1;

	if (flags & START_BUTTON)
		DBG("Start button held\n");
	if (flags & SETUP_BUTTON)
		DBG("Setup button held\n");

	/* Initialize software timers */
	timer_init();

	/* Initialize real time clock */
	rtc_init(flags);

	/* Initialize the cat sensor */
	catsensor_init();

	/* Initialize the water sensor and valve */
	water_init();

	/* Initialize the user interface */
	userinterface_init(flags);

#ifdef HAS_COMMANDLINE
	/* Initialize the command line interface */
	cmdline_init();
#endif /* HAS_COMMANDLINE */

	/* Initialize the washing program */
	litterlanguage_init(flags);

	/* Initialize interrupts */
	interrupt_init();

	/* Execute the run loop */
	for(;;){
		rtc_work();
		catsensor_work();
		water_work();
		catgenie_work();
		userinterface_work();
#ifdef HAS_COMMANDLINE
		cmdline_work();
#endif /* HAS_COMMANDLINE */
		litterlanguage_work();
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
#if (defined _16F877A)
	if (RBIF) {
#elif (defined _16F1939)
	if (IOCIF) {
#endif
		/* Detected changes */
		temp = PORTB ^ PORTB_old;
		/* Reset interrupt */
#if (defined _16F877A)
		RBIF = 0;
#elif (defined _16F1939)
		IOCBF = 0;
		IOCIF = 0;
#endif
		/* Handle interrupt */
		if (temp & CATSENSOR_MASK)
			catsensor_isr_input();
		/* Update the old status */
		PORTB_old = PORTB ;
	}
}
