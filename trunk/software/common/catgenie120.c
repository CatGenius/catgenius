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
#include "../common/eventlog.h"

extern void heatsensor_event  (unsigned char detected);
extern void startbutton_event (unsigned char up);
extern void setupbutton_event (unsigned char up);


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

/* Timing configuration */
#define BUTTON_DEBOUNCE			(SECOND/20)	/*   50ms */
#define PACER_BITTIME			(SECOND/8)	/*  125ms */

/* Debouncers */
#define DEBOUNCER_BUTTON_START		0
#define DEBOUNCER_BUTTON_SETUP		1
#define DEBOUNCER_MAX			2

/* Pacers */
#define PACER_BEEPER			0
#define PACER_LED_ERROR			1
#define PACER_LED_LOCKED		2
#define PACER_LED_CARTRIDGE		3
#define PACER_LED_CAT			4
#define PACER_MAX			5


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static unsigned char	PORTB_old;
static bit		heat_old = 0;

struct debouncer {
	struct timer	timer;
	unsigned char	state		: 1;
	unsigned	port_bit	: 3;
	volatile char	*port;
	void		(*handler)(unsigned char);
};
static struct debouncer	debouncers[DEBOUNCER_MAX] = {
	{NEVER, 0, STARTBUTTON_BIT, &STARTBUTTON(PORT), startbutton_event},
	{NEVER, 0, SETUPBUTTON_BIT, &SETUPBUTTON(PORT), setupbutton_event}
};

struct pacer {
	struct timer	timer;
	unsigned char	pattern;
	unsigned	pattern_bit	: 3;
	unsigned	repeat		: 1;
	unsigned	port_bit	: 3;
	volatile char	*port;
};
static struct pacer	pacers[PACER_MAX] = {
	{EXPIRED, 0x00, 0x1, 0, BEEPER_BIT,        &BEEPER(LAT)},
	{EXPIRED, 0x00, 0x1, 0, LED_ERROR_BIT,     &LED_ERROR(LAT)},
	{EXPIRED, 0x00, 0x1, 0, LED_LOCKED_BIT,    &LED_LOCKED(LAT)},
	{EXPIRED, 0x00, 0x1, 0, LED_CARTRIDGE_BIT, &LED_CARTRIDGE(LAT)},
	{EXPIRED, 0x00, 0x1, 0, LED_CAT_BIT,       &LED_CAT(LAT)}
};


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void set_pacer (unsigned char pacer, unsigned char pattern, unsigned char repeat);


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

#if (defined _16F1939)
	/* Enable internal pull-ups globally */
	nWPUEN = 0;
#endif /* _16F1939 */

	/*
	 * Setup port A
	 */
	TRISA = NOT_USED_1_MASK      |	/* Not used (R39, Absent) */
		NOT_USED_2_MASK      |	/* Not used (R1, Absent) */
		WATERSENSORANALOG_MASK;	/* Analog water sensor input */
	PORTA = 0x00;
#if (defined _16F877A)
	/* Disable ADC */
	ADCON1 = 0x07;
#elif (defined _16F1939)
#ifdef WATERSENSOR_ANALOG
	/* Select digital function for all inputs, except for water sensor input */
	ANSELA = WATERSENSORANALOG_MASK;/* Analog water sensor input */
#else
	/* Select digital function for all inputs */
	ANSELA = 0;
#endif /* WATERSENSOR_ANALOG */
#endif /* _16F877A/_16F1939 */

	/*
	 * Setup port B
	 */
	TRISB = BIT(STARTBUTTON_BIT) |	/* Button Start/Pause */
		WATERVALVE_MASK  |	/* Water Sensor */
		HEATSENSOR_MASK  |	/* Over heat detector (U4) */
		CATSENSOR_MASK   |	/* Cat Sensor */
		BIT(SETUPBUTTON_BIT) |	/* Button Auto setup */
		NOT_USED_3_MASK  |	/* PGM Clock */
		NOT_USED_4_MASK  ;	/* PGM Data */
	PORTB = 0x00;
#if (defined _16F877A)
	/* Enable pull-up resitors on all pins */
	nRBPU = 0;
#elif (defined _16F1939)
	/* Select digital function for all inputs */
	ANSELB = 0;
	/* Enable pull-up resitors on all pins individually, except for WATERVALVE */
	WPUB = ~WATERVALVE_MASK;
#endif /* _16F877A/_16F1939 */
	/* Clear the interrupt status */
#if (defined _16F877A)
	RBIF = 0;
#elif (defined _16F1939)
	/* Enable both rising- and falling-edge detection */
	IOCBP = CATSENSOR_MASK;
	IOCBN = CATSENSOR_MASK;
	IOCBF = 0;
	IOCIF = 0;
#endif /* _16F877A/_16F1939 */
	/* Enable interrupts */
#if (defined _16F877A)
	RBIE = 1;
#elif (defined _16F1939)
	IOCIE = 1;
#endif /* _16F877A/_16F1939 */

	PORTB_old = BIT(STARTBUTTON_BIT) |
		    BIT(SETUPBUTTON_BIT) |
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
	PORTD = 0;
#if (defined _16F1939)
	/* Select digital function for all inputs */
	ANSELD = 0;
#endif /* _16F1939 */

	/*
	 * Setup port E
	 */
	TRISE = 0x00;			/* All outputs */
	PORTE = 0x00;
#if (defined _16F1939)
	/* Select digital function for all inputs */
	ANSELE = 0;
	/* Disable all individual pull-ups */
	WPUE = 0x00;
#endif /* _16F1939 */

	/* Delay to settle ports */
	__delay_ms(100);
	__delay_ms(100);

	/* Copy the initial states into the debouncer states */
	for (temp = 0; temp < DEBOUNCER_MAX; temp++) {
		unsigned char	mask = 1 << debouncers[temp].port_bit; /* for compiler limitations */

		debouncers[temp].state = *debouncers[temp].port & mask;
	}

	/* Fill out the return flags */
	temp = 0;
	if (!(STARTBUTTON(PORT) & BIT(STARTBUTTON_BIT)))
		temp |= START_BUTTON;
	if (!(SETUPBUTTON(PORT) & BIT(SETUPBUTTON_BIT)))
		temp |= SETUP_BUTTON;

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

	/* Check for overheat */
	temp = HEATSENSOR(PORT) & HEATSENSOR_MASK;
	if (temp != heat_old) {
		heatsensor_event(temp);
		heat_old = temp;
	}

	/* Poll critical Port B inputs for changes */
	status    = PORTB;
	temp      = status ^ PORTB_old;
	PORTB_old = status;
	if (temp & BIT(STARTBUTTON_BIT))
		settimeout(&debouncers[DEBOUNCER_BUTTON_START].timer, BUTTON_DEBOUNCE);
	if (temp & BIT(SETUPBUTTON_BIT))
		settimeout(&debouncers[DEBOUNCER_BUTTON_SETUP].timer, BUTTON_DEBOUNCE);

	/* Execute the debouncers */
	for (temp = 0; temp < DEBOUNCER_MAX; temp++)
		if (timeoutexpired(&debouncers[temp].timer)) {
			unsigned char	tempstate = *debouncers[temp].port; /* for compiler limitations */
			tempstate &= 1 << debouncers[temp].port_bit;
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
			unsigned char	mask = 1 << pacers[temp].pattern_bit; /* for compiler limitations */

			/* Set a time for the next execution time */
			settimeout(&pacers[temp].timer, PACER_BITTIME);
			/* Copy the current pattern bit to the output */
			if (pacers[temp].pattern & mask)
				*pacers[temp].port |= (1 << pacers[temp].port_bit);
			else
				*pacers[temp].port &= ~(1 << pacers[temp].port_bit);
			/* Update the current bit */
			if (!++pacers[temp].pattern_bit) {
				/* Clear the pattern if repeat is not selected */
				if (!pacers[temp].repeat)
				{
					pacers[temp].pattern = 0;
					eventlog_track(EVENTLOG_PACER + temp, 0);
				}
			}
		}
}
/* End: catgenie_work */


void set_LED (unsigned char led, unsigned char on)
{
	volatile unsigned char	*latch;
	unsigned char		mask;
	switch(led) {
	case 1:
		latch = &LED_1(LAT);
		mask = LED_1_MASK;
		break;
	case 2:
		latch = &LED_2(LAT);
		mask = LED_2_MASK;
		break;
	case 3:
		latch = &LED_3(LAT);
		mask = LED_3_MASK;
		break;
	case 4:
		latch = &LED_4(LAT);
		mask = LED_4_MASK;
		break;
	default:
		return;
	}
	if (on)
		*latch |= mask;
	else
		*latch &= ~mask;

	eventlog_track(EVENTLOG_LED + (led-1), on);
}


void set_LED_Error (unsigned char pattern, unsigned char repeat)
{
	set_pacer(PACER_LED_ERROR, pattern, repeat);
}


void set_LED_Locked (unsigned char pattern, unsigned char repeat)
{
	set_pacer(PACER_LED_LOCKED, pattern, repeat);
}


void set_LED_Cartridge (unsigned char pattern, unsigned char repeat)
{
	set_pacer(PACER_LED_CARTRIDGE, pattern, repeat);
}


void set_LED_Cat (unsigned char pattern, unsigned char repeat)
{
	set_pacer(PACER_LED_CAT, pattern, repeat);
}


void set_Beeper (unsigned char pattern, unsigned char repeat)
{
	set_pacer(PACER_BEEPER, pattern, repeat);
}


void set_Bowl (unsigned char mode)
{
	switch (mode) {
	default:
	case BOWL_STOP:
		BOWL(LAT) &= ~(BOWL_CWCCW_MASK | BOWL_ONOFF_MASK);
		break;
	case BOWL_CW:
		BOWL(LAT) |= BOWL_CWCCW_MASK;
		BOWL(LAT) |= BOWL_ONOFF_MASK;
		break;
	case BOWL_CCW:
		BOWL(LAT) &= ~BOWL_CWCCW_MASK;
		BOWL(LAT) |=  BOWL_ONOFF_MASK;
		break;
	}

	eventlog_track(EVENTLOG_BOWL, mode);
}


unsigned char get_Bowl (void)
{
	switch (BOWL(LAT) & (BOWL_CWCCW_MASK | BOWL_ONOFF_MASK)) {
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
		ARM(LAT) &= ~(ARM_UPDOWN_MASK | ARM_ONOFF_MASK);
		break;
	case ARM_UP:
		ARM(LAT) &= ~ARM_UPDOWN_MASK;
		ARM(LAT) |=  ARM_ONOFF_MASK;
		break;
	case ARM_DOWN:
		ARM(LAT) |= ARM_UPDOWN_MASK;
		ARM(LAT) |= ARM_ONOFF_MASK;
		break;
	}

	eventlog_track(EVENTLOG_ARM, mode);
}


unsigned char get_Arm (void)
{
	switch (ARM(LAT) & (ARM_UPDOWN_MASK | ARM_ONOFF_MASK)) {
	case ARM_ONOFF_MASK:
		return (ARM_UP);
	case ARM_ONOFF_MASK | ARM_UPDOWN_MASK:
		return (ARM_DOWN);
	}
	return (ARM_STOP);
}


void set_Dosage (unsigned char on)
{
	if (on)
		DOSAGE(LAT) |= DOSAGE_MASK;
	else
		DOSAGE(LAT) &= ~DOSAGE_MASK;

	eventlog_track(EVENTLOG_DOSAGE, on);
}


unsigned char get_Dosage (void)
{
	return (DOSAGE(LAT) & DOSAGE_MASK);
}


void set_Pump (unsigned char on)
{
	if (on)
		PUMP(LAT) |= PUMP_MASK;
	else
		PUMP(LAT) &= ~PUMP_MASK;

	eventlog_track(EVENTLOG_PUMP, on);
}


unsigned char get_Pump (void)
{
	return (PUMP(LAT) & PUMP_MASK);
}


void set_Dryer	(unsigned char on)
{
	if (on)
		DRYER(LAT) |= DRYER_MASK;
	else
		DRYER(LAT) &= ~DRYER_MASK;

	eventlog_track(EVENTLOG_DRYER, on);
}


unsigned char get_Dryer (void)
{
	return (DRYER(LAT) & DRYER_MASK);
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void set_pacer (unsigned char pacer, unsigned char pattern, unsigned char repeat)
{
	if( repeat &&
	    (pacers[pacer].pattern == pattern) &&
	    (pacers[pacer].repeat == repeat) )
		return;

	/* Expire the bit timer */
	timeoutnow(&pacers[pacer].timer);
	/* Reset the mask to the first bit */
	pacers[pacer].pattern_bit = 0x0;
	/* Copy the pacer pattern */
	pacers[pacer].pattern = pattern;
	/* Copy the repeat flag */
	pacers[pacer].repeat = repeat;

	eventlog_track(EVENTLOG_PACER + pacer, pattern);
}
