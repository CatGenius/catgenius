/******************************************************************************/
/* File    :	geniediag.c						      */
/* Function:	CatGenius Diagnostics Application			      */
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
#  pragma config CLKOUTEN = OFF		/* Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin) */
#  pragma config IESO = OFF		/* Internal/External Switchover (Internal/External Switchover mode is disabled) */
#  pragma config FCMEN = OFF		/* Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled) */

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

#include "../common/timer.h"
#include "../common/serial.h"
#include "../common/i2c.h"
#include "../common/catsensor.h"
#include "../common/water.h"

#ifdef HAS_COMMANDLINE
#include "../common/cmdline.h"
#include "../common/cmdline_tag.h"
#include "../common/cmdline_box.h"
#endif /* HAS_COMMANDLINE */

#ifdef HAS_BLUETOOTH
#include "../common/bluetooth.h"
#endif /* HAS_BLUETOOTH */

#include "userinterface.h"


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

#ifdef HAS_COMMANDLINE
static int start (int argc, char* argv[]);
static int setup (int argc, char* argv[]);
static int lock (int argc, char* argv[]);

/* command line commands */
const struct command	commands[] = {
	{"echo", cmd_echo},
	{"help", cmd_help},
#ifdef HAS_COMMANDLINE_BOX
	{"bowl", cmd_bowl},
	{"arm", cmd_arm},
	{"dosage", cmd_dosage},
	{"tap", cmd_tap},
	{"drain", cmd_drain},
	{"dryer", cmd_dryer},
	{"cat", cmd_cat},
	{"water", cmd_water},
	{"heat", cmd_heat},
	{"start", start},
	{"setup", setup},
	{"lock", lock},
#endif /* HAS_COMMANDLINE_BOX */
#ifdef HAS_COMMANDLINE_TAG
	{"tag", cmd_tag},
#endif /* HAS_COMMANDLINE_TAG */
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

	/* Initialize the serial port */
	serial_init(BITRATE, SERIAL_FLOW_NONE);

	printf("\n*** GenieDiag ***\n");
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

	/* Initialize software timers */
	timer_init();

#ifdef HAS_I2C
	/* Initialize the I2C bus */
	i2c_init();
#endif

	/* Initialize the cat sensor */
	catsensor_init();

	/* Initialize the water sensor and valve */
	water_init();

	/* Initialize the user interface */
	userinterface_init();

#ifdef HAS_COMMANDLINE
	/* Initialize the command line interface */
	cmdline_init();
#endif /* HAS_COMMANDLINE */

	/* Initialize interrupts */
	interrupt_init();

	/* Execute the run loop */
	for(;;){
		catsensor_work();
		water_work();
		catgenie_work();
		userinterface_work();
#ifdef HAS_COMMANDLINE
		cmdline_work();
#endif /* HAS_COMMANDLINE */
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
	/* (E)USART interrupts */
	if (RCIF)
		serial_rx_isr();
	if (TXIF)
		serial_tx_isr();
}


#ifdef HAS_COMMANDLINE
static int start (int argc, char* argv[])
{
	if (argc > 1)
		return ERR_SYNTAX;

	start_short();
	return ERR_OK;
}


static int setup (int argc, char* argv[])
{
	if (argc > 1)
		return ERR_SYNTAX;

	setup_short();
	return ERR_OK;
}


static int lock (int argc, char* argv[])
{
	if (argc > 1)
		return ERR_SYNTAX;

	both_long();
	return ERR_OK;
}
#endif /* HAS_COMMANDLINE */
