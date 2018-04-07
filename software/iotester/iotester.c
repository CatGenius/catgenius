/******************************************************************************/
/* File    :	iotester.c						      */
/* Function:	CatGenius IO testing Application			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/

/******************************************************************************/
/* Configuration bits (should precede includes)				      */
/******************************************************************************/

#if (defined _16F877A)
#  pragma config FOSC = XT		/* Oscillator Selection bits (XT oscillator) */
#  pragma config PWRTE = ON		/* Power-up Timer Enable bit (PWRT enabled) */
#  pragma config LVP = OFF		/* Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming) */
#  pragma config WRT = OFF		/* Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control) */
#  ifdef __DEBUG
#    pragma config WDTE = OFF		/* Watchdog Timer Enable bit (WDT disabled) */
#    pragma config BOREN = OFF		/* Brown-out Reset Enable bit (BOR disabled) */
#    pragma config CPD = OFF		/* Data EEPROM Memory Code Protection bit (Data EEPROM code protection off) */
#    pragma config CP = OFF		/* Flash Program Memory Code Protection bit (Code protection off) */
#  else
#    pragma config WDTE = ON		/* Watchdog Timer Enable bit (WDT enabled) */
#    pragma config BOREN = ON		/* Brown-out Reset Enable bit (BOR enabled) */
#    pragma config CPD = ON		/* Data EEPROM Memory Code Protection bit (Data EEPROM code-protected) */
#    pragma config CP = ON		/* Flash Program Memory Code Protection bit (All program memory code-protected) */
#  endif
#elif (defined _16F1939)
#  pragma config FOSC = XT		/* Oscillator Selection (XT Oscillator, Crystal/resonator connected between OSC1 and OSC2 pins) */
#  pragma config MCLRE = ON		/* MCLR Pin Function Select (MCLR/VPP pin function is MCLR) */
#  pragma config CLKOUTEN = OFF		/* Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin) */
#  pragma config IESO = OFF		/* Internal/External Switchover (Internal/External Switchover mode is disabled) */
#  pragma config FCMEN = OFF		/* Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled) */
#  ifdef __DEBUG
#    pragma config WDTE = OFF		/* Watchdog Timer Enable (WDT disabled) */
#    pragma config PWRTE = OFF		/* Power-up Timer Enable (PWRT disabled) */
#    pragma config CP = OFF		/* Flash Program Memory Code Protection (Program memory code protection is disabled) */
#    pragma config CPD = OFF		/* Data Memory Code Protection (Data memory code protection is disabled) */
#    pragma config BOREN = OFF		/* Brown-out Reset Enable (Brown-out Reset disabled) */
#  else
#    pragma config WDTE = ON		/* Watchdog Timer Enable (WDT enabled) */
#    pragma config PWRTE = ON		/* Power-up Timer Enable (PWRT enabled) */
#    pragma config CP = ON		/* Flash Program Memory Code Protection (Program memory code protection is enabled) */
#    pragma config CPD = ON		/* Data Memory Code Protection (Data memory code protection is enabled) */
#    pragma config BOREN = ON		/* Brown-out Reset Enable (Brown-out Reset enabled) */
#  endif

#  pragma config WRT = OFF		/* Flash Memory Self-Write Protection (Write protection off) */
#  pragma config VCAPEN = OFF		/* Voltage Regulator Capacitor Enable (All VCAP pin functionality is disabled) */
#  pragma config PLLEN = OFF		/* PLL Enable (4x PLL disabled) */
#  pragma config STVREN = ON		/* Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset) */
#  pragma config BORV = HI		/* Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), high trip point selected.) */
#  pragma config LVP = OFF		/* Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming) */
#endif


#include <xc.h>
#include <stdio.h>

#include "../common/hardware.h"		/* Flexible hardware configuration */

#include "../common/serial.h"
#include "../common/cmdline.h"
#include "../common/cmdline_gpio.h"

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

/* command line commands */
const struct command	commands[] = {
	{"?", cmd_help},
	{"help", cmd_help},
	{"echo", cmd_echo},
	{"gpio", cmd_gpio},
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
	serial_init(BITRATE, SERIAL_FLOW_NONE);

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
