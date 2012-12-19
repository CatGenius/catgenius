/******************************************************************************/
/* File    :	iotester.c						      */
/* Function:	CatGenius IO testing Application			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>

#include "../common/hardware.h"		/* Flexible hardware configuration */

#include "../common/serial.h"
#include "../common/cmdline.h"
#include "../common/cmdline_gpio.h"

/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#if (defined _16F877A)
#  ifdef __DEBUG
	__CONFIG(FOSC_XT & WDTE_OFF & PWRTE_ON & BOREN_OFF & LVP_OFF & CPD_OFF & WRT_OFF & DEBUG_ON & CP_OFF);
#  else
	__CONFIG(FOSC_XT & WDTE_ON  & PWRTE_ON & BOREN_ON  & LVP_OFF & CPD_ON  & WRT_OFF & DEBUG_OFF & CP_ON);
#  endif
#elif (defined _16F1939)
#  ifdef __DEBUG
	__CONFIG(FOSC_XT & WDTE_OFF & PWRTE_OFF & MCLRE_ON & CP_OFF & CPD_OFF & BOREN_OFF & CLKOUTEN_OFF & IESO_OFF & FCMEN_OFF);
	__CONFIG(WRT_OFF & /* VCAPEN_OFF &*/ PLLEN_OFF & STVREN_ON & BORV_HI & LVP_OFF);
#  else
	__CONFIG(FOSC_XT & WDTE_ON  & PWRTE_ON & MCLRE_ON & CP_ON  & CPD_ON  & BOREN_ON  & CLKOUTEN_OFF & IESO_OFF & FCMEN_OFF);
	__CONFIG(WRT_OFF & /* VCAPEN_OFF &*/ PLLEN_OFF & STVREN_ON & BORV_HI & LVP_OFF);
#  endif
#endif


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

#ifdef __RESETBITS_ADDR
extern bit		__powerdown;
extern bit		__timeout;
#endif /* __RESETBITS_ADDR */

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

	/* Init the hardware */
	flags = catgenie_init();

	/* Initialize the serial port */
	serial_init();

	printf("\n*** I/O Tester ***\n");
	if (!nPOR) {
		DBG("Power-on reset\n");
		flags |= POWER_FAILURE;
	} else if (!nBOR) {
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
	nPOR = 1;
	nBOR = 1;

	if (flags & START_BUTTON)
		DBG("Start button held\n");
	if (flags & SETUP_BUTTON)
		DBG("Setup button held\n");

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
