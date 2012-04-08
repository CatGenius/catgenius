/******************************************************************************/
/* File    :	catgenie120.c						      */
/* Function:	Catgenie 120 hardware support module			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "hardware.h"			/* Flexible hardware configuration */

#include "timer.h"

extern void heatsensor_event  (unsigned char detected);
extern void startbutton_event (unsigned char up);
extern void setupbutton_event (unsigned char up);


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

/* Timing configuration */
#define BUTTON_DEBOUNCE		(SECOND/20)	/*   50ms */
#define HEATSENSOR_DEBOUNCE	(SECOND/20)	/*   50ms */
#define PACER_BITTIME		(SECOND/8)	/*  125ms */

/* Debouncers */
#define DEBOUNCER_BUTTON_START	0
#define DEBOUNCER_BUTTON_SETUP	1
#define DEBOUNCER_SENSOR_HEAT	2
#define DEBOUNCER_MAX		3

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
/* Function:	catgenie_init						      */
/*		- Initialize the CatGenie 120 hardware			      */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	unsigned char	temp ;

#if (defined HW_CATGENIE120PLUS)
	/* Enable internal pull-ups globally */
	nWPUEN = 0;
#endif

	/*
	 * Setup port A
	 */
	TRISA = NOT_USED_1_MASK      |	/* Not used (R39, Absent) */
		NOT_USED_2_MASK      |	/* Not used (R1, Absent) */
		WATERSENSORANALOG_MASK;	/* Analog water sensor input */
	PORTA = 0x00;
	/* Disable ADC */
	ADCON1 = 0x07;
#if (defined HW_CATGENIE120PLUS)
	/* Select digital function */
	ANSELA = 0;
#endif

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
#if (defined HW_CATGENIE120)
	nRBPU = 0;
#elif (defined HW_CATGENIE120PLUS)
	/* Select digital function */
	ANSELB = 0;
	/* Enable all individual pull-ups */
	WPUB = 0xFF;
#endif
	/* Clear the interrupt status */
#if (defined HW_CATGENIE120)
	RBIF = 0;
#elif (defined HW_CATGENIE120PLUS)
	/* Enable both rising- and falling-edge detection */
	IOCBP = CATSENSOR_MASK;
	IOCBN = CATSENSOR_MASK;
	IOCBF = 0;
	IOCIF = 0;
#endif
	/* Enable interrupts */
#if (defined HW_CATGENIE120)
	RBIE = 1;
#elif (defined HW_CATGENIE120PLUS)
	IOCIE = 1;
#endif

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
#if (defined HW_CATGENIE120PLUS)
	/* Select digital function */
	ANSELD = 0;
#endif

	/*
	 * Setup port E
	 */
	TRISE = 0x00;			/* All outputs */
	PORTE = 0x00;
#if (defined HW_CATGENIE120PLUS)
	ANSELE = 0;
	/* Disable all indifidual pull-ups */
	WPUE = 0x00;
#endif

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
/* Function:	catgenie_work						      */
/*		- Worker function for the CatGenie 120 hardware		      */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	unsigned char	temp = 0;
	unsigned char	status;

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
			/* Copy the current pattern bit to the output */
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
/* End: catgenie_work */


void catgenie_term (void)
/******************************************************************************/
/* Function:	catgenie_term						      */
/*		- Terminate the CatGenie 120 hardware			      */
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

	/* Expire the bit timer */
//	timeoutnow(&pacers[PACER_LED_ERROR].timer); TODO: Possible stack overflow
	/* Reset the mask to the first bit */
	pacers[PACER_LED_ERROR].mask = 0x01;
	/* Copy the blink pattern */
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

	/* Expire the bit timer */
//	timeoutnow(&pacers[PACER_LED_LOCKED].timer); TODO: Possible stack overflow
	/* Reset the mask to the first bit */
	pacers[PACER_LED_LOCKED].mask = 0x01;
	/* Copy the blink pattern */
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

	/* Expire the bit timer */
//	timeoutnow(&pacers[PACER_LED_CARTRIDGE].timer); TODO: Possible stack overflow
	/* Reset the mask to the first bit */
	pacers[PACER_LED_CARTRIDGE].mask = 0x01;
	/* Copy the blink pattern */
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

	/* Expire the bit timer */
//	timeoutnow(&pacers[PACER_LED_CAT].timer); TODO: Possible stack overflow
	/* Reset the mask to the first bit */
	pacers[PACER_LED_CAT].mask = 0x01;
	/* Copy the blink pattern */
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

	/* Expire the bit timer */
//	timeoutnow(&pacers[PACER_BEEPER].timer); TODO: Possible stack overflow
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


unsigned char get_Bowl (void)
{
	switch (BOWL_PORT & (BOWL_CWCCW_MASK | BOWL_ONOFF_MASK)) {
	case BOWL_ONOFF_MASK:
		return (BOWL_CCW);
	case BOWL_ONOFF_MASK | BOWL_CWCCW_MASK:
		return (BOWL_CW);
	}
	return (BOWL_STOP);
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


unsigned char get_Arm (void)
{
	switch (ARM_PORT & (ARM_UPDOWN_MASK | ARM_ONOFF_MASK)) {
	case ARM_ONOFF_MASK:
		return (ARM_UP);
	case ARM_ONOFF_MASK | ARM_UPDOWN_MASK:
		return (ARM_DOWN);
	}
	return (ARM_STOP);
}


void set_Water (unsigned char on)
{
	if (on)
		/* Unmute Water Sensor by switching on the LED */
		WATERSENSOR_LED_PORT |= WATERSENSOR_LED_MASK;
	else
		/* Mute Water Sensor by switching off the LED */
		WATERSENSOR_LED_PORT &= ~WATERSENSOR_LED_MASK;
}


unsigned char get_Water (void)
{
	return (WATERSENSOR_LED_PORT & WATERSENSOR_LED_MASK);
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


unsigned char get_Pump (void)
{
	return (PUMP_PORT & PUMP_MASK);
}


void set_Dryer	(unsigned char on)
{
	if (on)
		DRYER_PORT |= DRYER_MASK;
	else
		DRYER_PORT &= ~DRYER_MASK;
}


unsigned char get_Dryer (void)
{
	return (DRYER_PORT & DRYER_MASK);
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

