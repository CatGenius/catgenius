/******************************************************************************/
/* File    :	iotester.c						      */
/* Function:	CatGenius IO testing Application			      */
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

#include "../common/serial.h"
#include "../common/cmdline.h"
#include "../common/cmdline_gpio.h"


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

#if (defined __PICC__)
extern bit		__powerdown;
extern bit		__timeout;
#endif /* __PICC__ */

/* command line commands */
const struct command	commands[] = {
	{"?", help},
	{"help", help},
	{"echo", echo},
	{"gpio", gpio},
	{"", NULL}
};


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

	/* Initialize the serial port */
	serial_init(BITRATE, 0);

	printf("\n*** I/O Tester ***\n");
	if (!nPOR) {
		printf("Power-on reset\n");
		flags |= POWER_FAILURE;
	} else if (!nBOR) {
		printf("Brown-out reset\n");
		flags |= POWER_FAILURE;
	}
#ifdef __RESETBITS_ADDR
	else if (!__timeout)
		printf("Watchdog reset\n");
	else if (!__powerdown)
		printf("Pin reset (sleep)\n");
	else
		printf("Pin reset\n");
#else
	else
		printf("Unknown reset\n");
#endif /* __RESETBITS_ADDR */
	nPOR = 1;
	nBOR = 1;

	if (flags & START_BUTTON)
		printf("Start button held\n");
	if (flags & SETUP_BUTTON)
		printf("Setup button held\n");

	/* Initialize the command line interface */
	cmdline_init();

	/* Execute the run loop */
	for(;;){
		cmdline_work();
#ifndef __DEBUG
		CLRWDT();
#endif
	}
}


void startbutton_event (unsigned char up)
{
}


void setupbutton_event (unsigned char up)
{
}


void heatsensor_event (unsigned char up)
{
}
