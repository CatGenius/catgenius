/******************************************************************************/
/* File    :	timer.h							      */
/* Function:	Timer module						      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "catgenie120.h"
#include "hardware.h"
#include "timer.h"

extern void watersensor_event (unsigned char undetected);
extern void heatsensor_event  (unsigned char detected);
extern void startbutton_event (unsigned char up);
extern void setupbutton_event (unsigned char up);


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

/* Timing configuration */
#define WATERSENSORPOLLING	(SECOND/10)	/*  100ms*/
#define BUTTON_DEBOUNCE		(SECOND/20)	/*   50ms */
#define WATERSENSOR_DEBOUNCE	(SECOND)	/* 1000ms */
#define HEATSENSOR_DEBOUNCE	(SECOND/20)	/*   50ms */
#define PACER_BITTIME		(SECOND/8)	/*  125ms */

/* Debouncers */
#define DEBOUNCER_BUTTON_START	0
#define DEBOUNCER_BUTTON_SETUP	1
#define DEBOUNCER_SENSOR_WATER	2
#define DEBOUNCER_SENSOR_HEAT	3
#define DEBOUNCER_MAX		4

/* Pacers */
#define PACER_BEEPER		0
#define PACER_LED_ERROR		1
#define PACER_LED_LOCKED	2
#define PACER_LED_CARTRIDGE	3
#define PACER_LED_CAT		4
#define PACER_MAX		5


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static unsigned char	PORTB_old;

static struct timer	water_sensortimer      = EXPIRED;
static unsigned char	water_sensorbuffer     = WATERSENSOR_MASK;
static unsigned char	water_sensorbuffer_old = WATERSENSOR_MASK;
static bit		water_filling          = 0;

struct debouncer {
	struct timer	timer;
	unsigned long	timeout;
	unsigned char	state;
	volatile char	*port;
	unsigned char	port_mask;
	void		(*handler)(unsigned char);
};
static struct debouncer	debouncers[DEBOUNCER_MAX] = {
	{NEVER, BUTTON_DEBOUNCE,      0, &STARTBUTTON_PORT,   STARTBUTTON_MASK, startbutton_event},
	{NEVER, BUTTON_DEBOUNCE,      0, &SETUPBUTTON_PORT,   SETUPBUTTON_MASK, setupbutton_event},
	{NEVER, WATERSENSOR_DEBOUNCE, 0, &water_sensorbuffer, WATERSENSOR_MASK, watersensor_event},
	{NEVER, HEATSENSOR_DEBOUNCE,  0, &HEATSENSOR_PORT,    HEATSENSOR_MASK,  heatsensor_event}
};

struct pacer {
	struct timer	timer;
/*	unsigned long	timeout;*/
	unsigned char	mask;
	unsigned char	pattern;
	unsigned char	repeat;
	volatile char	*port;
	unsigned char	port_mask;
};
static struct pacer	pacers[PACER_MAX] = {
	{EXPIRED, 0x01, 0x00, 0, &BEEPER_PORT,        BEEPER_MASK},
	{EXPIRED, 0x01, 0x00, 0, &LED_ERROR_PORT,     LED_ERROR_MASK},
	{EXPIRED, 0x01, 0x00, 0, &LED_LOCKED_PORT,    LED_LOCKED_MASK},
	{EXPIRED, 0x01, 0x00, 0, &LED_CARTRIDGE_PORT, LED_CARTRIDGE_MASK},
	{EXPIRED, 0x01, 0x00, 0, &LED_CAT_PORT,       LED_CAT_MASK}
};


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

unsigned char catgenie_init (void)
/******************************************************************************/
/* Function:	init_catgenie						      */
/*		- Initialize the CatGenie 120 hardware			      */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	unsigned char	temp ;

	/*
	 * Setup port A
	 */
	TRISA = NOT_USED_1_MASK      |	/* Not used (R39, Absent) */
		NOT_USED_2_MASK      |	/* Not used (R1, Absent) */
		WATERSENSORANALOG_MASK;	/* Analog water sensor input */
	PORTA = 0x00;
	/* Disable ADC */
	ADCON1 = 0x07;

	/*
	 * Setup port B
	 */
	TRISB = STARTBUTTON_MASK |	/* Button Start/Pause */
		WATERSENSOR_MASK |	/* Water Sensor */
		HEATSENSOR_MASK  |	/* Over heat detector (U4) */
		CATSENSOR_MASK   |	/* Cat Sensor */
		SETUPBUTTON_MASK |	/* Button Auto setup */
		NOT_USED_3_MASK  |	/* PGM Clock */
		NOT_USED_4_MASK  ;	/* PGM Data */
	PORTB = 0x00;
	/* Turn on internal weak pull-up resitors on inputs */
	RBPU = 0;
	/* Clear the interrupt status */
	RBIF = 0;
	/* Enable interrupts */
	RBIE = 1;

	PORTB_old = STARTBUTTON_MASK |
		    SETUPBUTTON_MASK |
		    HEATSENSOR_MASK ;

	/*
	 * Setup port C
	 */
	TRISC = I2C_SCL_MASK |		/* I2C SCL */
		I2C_SDA_MASK |		/* I2C SDA */
		UART_RXD_MASK ;		/* UART RxD */

	PORTC = 0x00;

	/*
	 * Setup port D
	 */
	TRISD = 0;
	PORTD = WATERSENSORPULLUP_MASK;	/* Activate water sensor pull-up resistor */

	/*
	 * Setup port E
	 */
	TRISE = 0x00;			/* All outputs */
	PORTE = 0x00;

	/* Delay to settle ports */
	__delay_ms(100);
	__delay_ms(100);

	/* Copy the initial states into the debouncer states */
	for (temp = 0; temp < DEBOUNCER_MAX; temp++) {
		unsigned char	tempmask = debouncers[temp].port_mask; /* for compiler limitations */

		debouncers[temp].state = *debouncers[temp].port & tempmask;
	}

	/* Fill out the return flags */
	temp = 0;
	if (!(STARTBUTTON_PORT & STARTBUTTON_MASK))
		temp |= START_BUTTON_HELD;
	if (!(SETUPBUTTON_PORT & SETUPBUTTON_MASK))
		temp |= SETUP_BUTTON_HELD;

	return temp;
}
/* End: init_catgenie */


void catgenie_work (void)
/******************************************************************************/
/* Function:	do_catgenie						      */
/*		- Worker function for the CatGenie 120 hardware		      */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	unsigned char	temp = 0;
	unsigned char	status ;

	/* Poll the water sensor */
	if (timeoutexpired(&water_sensortimer)) {
		/* Set new polling timeout */
		settimeout(&water_sensortimer, WATERSENSORPOLLING);
		if (!water_filling)
			/* Unmute Water Sensor by switching on the LED */
			PORTB |= WATERSENSOR_LED_MASK;
		do {
			/* 0 == no reflection == water detected */
			if (water_sensorbuffer = WATERSENSOR_PORT & WATERSENSOR_MASK)
				break;
			temp++;
		} while (temp <= 125); /* TODO: Verify if 125 loops is ~2ms */
		if (!water_filling)
			/* Mute Water Sensor by switching off the LED */
			PORTB &= ~WATERSENSOR_LED_MASK;
		/* Detect state change */
		if (water_sensorbuffer != water_sensorbuffer_old) {
			/* Postpone the debouncer */
			settimeout(&debouncers[DEBOUNCER_SENSOR_WATER].timer,
				   debouncers[DEBOUNCER_SENSOR_WATER].timeout);
			water_sensorbuffer_old = water_sensorbuffer;
		}
	}

	/* Poll critical Port B inputs for changes */
	status    = PORTB;
	temp      = status ^ PORTB_old;
	PORTB_old = status;
	if (temp & STARTBUTTON_MASK)
		settimeout(&debouncers[DEBOUNCER_BUTTON_START].timer,
			   debouncers[DEBOUNCER_BUTTON_START].timeout);
	if (temp & SETUPBUTTON_MASK)
		settimeout(&debouncers[DEBOUNCER_BUTTON_SETUP].timer,
			   debouncers[DEBOUNCER_BUTTON_SETUP].timeout);
	if (temp & HEATSENSOR_MASK)
		/* TODO: We may want to respond immediatly */
		settimeout(&debouncers[DEBOUNCER_SENSOR_HEAT].timer,
			   debouncers[DEBOUNCER_SENSOR_HEAT].timeout);

	/* Execute the debouncers */
	for (temp = 0; temp < DEBOUNCER_MAX; temp++)
		if (timeoutexpired(&debouncers[temp].timer)) {
			unsigned char	tempstate = *debouncers[temp].port; /* for compiler limitations */
			tempstate &= debouncers[temp].port_mask;
			/* Check if the state changed */
			if (tempstate != debouncers[temp].state) {
				/* Call function pointer (cannot be NULL) */
				debouncers[temp].handler(tempstate);
				debouncers[temp].state = tempstate;
			}
			timeoutnever(&debouncers[temp].timer);
		}

	/* Execute the pacers */
	for (temp = 0; temp < PACER_MAX; temp++)
		if (timeoutexpired(&pacers[temp].timer)) {
			unsigned char	tempmask = pacers[temp].mask; /* for compiler limitations */

			/* Set a time for the next execution time */
			settimeout(&pacers[temp].timer, PACER_BITTIME);
			/* Copy the current pattern bit to the beeper */
			if (pacers[temp].pattern & tempmask)
				*pacers[temp].port |= pacers[temp].port_mask;
			else
				*pacers[temp].port &= ~pacers[temp].port_mask;
			/* Update the current bit */
			if (!(pacers[temp].mask <<= 1)) {
				pacers[temp].mask = 1;
				/* Clear the pattern if repeat is not selected */
				if (!pacers[temp].repeat)
					pacers[temp].pattern = 0;
			}
		}
}

void catgenie_term (void)
/******************************************************************************/
/* Function:	term_catgenie						      */
/*		- Initialize the CatGenie 120 hardware			      */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* End: term_catgenie */

void set_LED (unsigned char led, unsigned char on)
{
	volatile unsigned char	*port;
	unsigned char		mask;
	switch(led) {
	case 1:
		port = &LED_1_PORT;
		mask = LED_1_MASK;
		break;
	case 2:
		port = &LED_2_PORT;
		mask = LED_2_MASK;
		break;
	case 3:
		port = &LED_3_PORT;
		mask = LED_3_MASK;
		break;
	case 4:
		port = &LED_4_PORT;
		mask = LED_4_MASK;
		break;
	default:
		return;
	}
	if (on)
		*port |= mask;
	else
		*port &= ~mask;
}

void set_LED_Error (unsigned char pattern, unsigned char repeat)
{
	if( repeat &&
	    (pacers[PACER_LED_ERROR].pattern == pattern) &&
	    (pacers[PACER_LED_ERROR].repeat == repeat) )
		return;

	/* Reset the mask to the first bit */
	pacers[PACER_LED_ERROR].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_ERROR].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_ERROR].repeat = repeat;
}

void set_LED_Locked (unsigned char pattern, unsigned char repeat)
{
	if( repeat &&
	    (pacers[PACER_LED_LOCKED].pattern == pattern) &&
	    (pacers[PACER_LED_LOCKED].repeat == repeat) )
		return;

	/* Reset the mask to the first bit */
	pacers[PACER_LED_LOCKED].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_LOCKED].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_LOCKED].repeat = repeat;
}

void set_LED_Cartridge (unsigned char pattern, unsigned char repeat)
{
	if( repeat &&
	    (pacers[PACER_LED_CARTRIDGE].pattern == pattern) &&
	    (pacers[PACER_LED_CARTRIDGE].repeat == repeat) )
		return;

	/* Reset the mask to the first bit */
	pacers[PACER_LED_CARTRIDGE].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_CARTRIDGE].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_CARTRIDGE].repeat = repeat;
}

void set_LED_Cat (unsigned char pattern, unsigned char repeat)
{
	if( repeat &&
	    (pacers[PACER_LED_CAT].pattern == pattern) &&
	    (pacers[PACER_LED_CAT].repeat == repeat) )
		return;
	/* Reset the mask to the first bit */
	pacers[PACER_LED_CAT].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_CAT].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_CAT].repeat = repeat;
}

void set_Beeper (unsigned char pattern, unsigned char repeat)
{
	if( repeat &&
	    (pacers[PACER_BEEPER].pattern == pattern) &&
	    (pacers[PACER_BEEPER].repeat == repeat) )
		return;

	/* Reset the mask to the first bit */
	pacers[PACER_BEEPER].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_BEEPER].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_BEEPER].repeat = repeat;
}

void set_Bowl (unsigned char mode)
{
	switch (mode) {
	default:
	case BOWL_STOP:
		BOWL_PORT &= ~(BOWL_CWCCW_MASK | BOWL_ONOFF_MASK);
		break;
	case BOWL_CW:
		BOWL_PORT |= BOWL_CWCCW_MASK;
		BOWL_PORT |= BOWL_ONOFF_MASK;
		break;
	case BOWL_CCW:
		BOWL_PORT &= ~BOWL_CWCCW_MASK;
		BOWL_PORT |=  BOWL_ONOFF_MASK;
		break;
	}
}

void set_Arm (unsigned char mode)
{
	switch (mode) {
	default:
	case ARM_STOP:
		ARM_PORT &= ~(ARM_UPDOWN_MASK | ARM_ONOFF_MASK);
		break;
	case ARM_UP:
		ARM_PORT &= ~ARM_UPDOWN_MASK;
		ARM_PORT |=  ARM_ONOFF_MASK;
		break;
	case ARM_DOWN:
		ARM_PORT |= ARM_UPDOWN_MASK;
		ARM_PORT |= ARM_ONOFF_MASK;
		break;
	}
}

void set_Water (unsigned char on)
{
	if (on) {
		water_filling = 1;
		/* Unmute Water Sensor by switching on the LED */
		PORTB |= WATERSENSOR_LED_MASK;
	} else {
		water_filling = 0;
		/* Mute Water Sensor by switching off the LED */
		PORTB &= ~WATERSENSOR_LED_MASK;
	}
}

void set_Dosage (unsigned char on)
{
	if (on)
		DOSAGE_PORT |= DOSAGE_MASK;
	else
		DOSAGE_PORT &= ~DOSAGE_MASK;
}

unsigned char get_Dosage (void)
{
	return (DOSAGE_PORT & DOSAGE_MASK);
}

void set_Pump (unsigned char on)
{
	if (on)
		PUMP_PORT |= PUMP_MASK;
	else
		PUMP_PORT &= ~PUMP_MASK;
}


void set_Dryer	(unsigned char on)
{
	if (on)
		DRYER_PORT |= DRYER_MASK;
	else
		DRYER_PORT &= ~DRYER_MASK;
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

