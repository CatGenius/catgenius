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
#include "timer.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define BIT(n)			(1U << (n))	/* Bit mask for bit 'n' */

#define TRISx0_OUT		(0 << 0)
#define TRISx0_IN		(1 << 0)
#define TRISx1_OUT		(0 << 1)
#define TRISx1_IN		(1 << 1)
#define TRISx2_OUT		(0 << 2)
#define TRISx2_IN		(1 << 2)
#define TRISx3_OUT		(0 << 3)
#define TRISx3_IN		(1 << 3)
#define TRISx4_OUT		(0 << 4)
#define TRISx4_IN		(1 << 4)
#define TRISx5_OUT		(0 << 5)
#define TRISx5_IN		(1 << 5)
#define TRISx6_OUT		(0 << 6)
#define TRISx6_IN		(1 << 6)
#define TRISx7_OUT		(0 << 7)
#define TRISx7_IN		(1 << 7)

/* Buttons */
#define STARTBUTTON_PORT	PORTB
#define STARTBUTTON_MASK	BIT(0)
#define SETUPBUTTON_PORT	PORTB
#define SETUPBUTTON_MASK	BIT(5)
#define BUTTON_DEBOUNCE		(SECOND/20)	/* 50ms */

/* Indicators */
#define LED_2_PORT		PORTA
#define LED_2_MASK		BIT(2)
#define LED_3_PORT		PORTA
#define LED_3_MASK		BIT(3)
#define LED_4_PORT		PORTA
#define LED_4_MASK		BIT(5)
#define LED_ERROR_PORT		PORTC
#define LED_ERROR_MASK		BIT(0)
#define BEEPER_PORT		PORTC
#define BEEPER_MASK		BIT(1)
#define LED_1_PORT		PORTC
#define LED_1_MASK		BIT(5)
#define LED_CARTRIDGE_PORT	PORTE
#define LED_CARTRIDGE_MASK	BIT(0)
#define LED_CAT_PORT		PORTE
#define LED_CAT_MASK		BIT(1)
#define LED_LOCKED_PORT		PORTE
#define LED_LOCKED_MASK		BIT(2)

/* Actuators */
#define WATER_PORT		PORTB
#define WATER_MASK		BIT(3)
#define PUMP_PORT		PORTD
#define PUMP_MASK		BIT(1)
#define DRYER_PORT		PORTD
#define DRYER_MASK		BIT(2)
#define DOSAGE_PORT		PORTD
#define DOSAGE_MASK		BIT(3)
#define BOWL_PORT		PORTD
#define BOWL_MASK_CWCCW		BIT(4)
#define BOWL_MASK_ONOFF		BIT(5)
#define ARM_PORT		PORTD
#define ARM_MASK_UPDOWN		BIT(6)
#define ARM_MASK_ONOFF		BIT(7)
#define DOSAGE_SECONDS_PER_ML	10	/* For 1 ml of cleaning liquid, 10 seconds of pumping */

/* Sensors */
#define CATSENSOR_LED_PORT	PORTC
#define CATSENSOR_LED_MASK	BIT(2)
#define CATSENSOR_PORT		PORTB
#define CATSENSOR_MASK		BIT(4)
#define WATERSENSOR_LED_PORT	PORTB
#define WATERSENSOR_LED_MASK	BIT(2)
#define WATERSENSOR_PORT	PORTA
#define WATERSENSOR_MASK	BIT(1)
#define WATERSENSOR_DEBOUNCE	(3*SECOND)	/* 3000ms */
#define HEATSENSOR_PORT		PORTB
#define HEATSENSOR_MASK		BIT(1)
#define HEATSENSOR_DEBOUNCE	(0)		/* 0ms */

/* Debouncers */
#define DEBOUNCER_BUTTON_START	0
#define DEBOUNCER_BUTTON_SETUP	1
#define DEBOUNCER_SENSOR_WATER	2
#define DEBOUNCER_SENSOR_HEAT	3
#define DEBOUNCER_MAX		4

/* Pacers */
#define PACER_BITTIME		(SECOND/5)	/* 200ms */
#define PACER_BEEPER		0
#define PACER_LED_ERROR		1
#define PACER_LED_LOCKED	2
#define PACER_LED_CARTRIDGE	3
#define PACER_LED_CAT		4
#define PACER_MAX		5

static void watersensor_event (unsigned char detected);
static void heatsensor_event  (unsigned char detected);
static void startbutton_event (unsigned char up);
static void setupbutton_event (unsigned char up);

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static unsigned char	PORTA_old;
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
	{{0xFFFF, 0xFFFFFFFF}, BUTTON_DEBOUNCE,      0, &STARTBUTTON_PORT, STARTBUTTON_MASK, startbutton_event},
	{{0xFFFF, 0xFFFFFFFF}, BUTTON_DEBOUNCE,      0, &SETUPBUTTON_PORT, SETUPBUTTON_MASK, setupbutton_event},
	{{0xFFFF, 0xFFFFFFFF}, WATERSENSOR_DEBOUNCE, 0, &WATERSENSOR_PORT, WATERSENSOR_MASK, watersensor_event},
	{{0xFFFF, 0xFFFFFFFF}, HEATSENSOR_DEBOUNCE,  0, &HEATSENSOR_PORT,  HEATSENSOR_MASK,  heatsensor_event}
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
	{{0x0000, 0x00000000}, 0x01, 0x00, 0, &BEEPER_PORT,        BEEPER_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x55, 1, &LED_ERROR_PORT,     LED_ERROR_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x00, 0, &LED_LOCKED_PORT,    LED_LOCKED_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x00, 0, &LED_CARTRIDGE_PORT, LED_CARTRIDGE_MASK},
	{{0x0000, 0x00000000}, 0x01, 0x00, 0, &LED_CAT_PORT,       LED_CAT_MASK}
};


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void catgenie_init (void)
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
	TRISA = TRISx0_IN  |	/* Not used (R39, Absent) */
		TRISx1_IN  |	/* Water Sensor */
		TRISx2_OUT |	/* LED 2 */
		TRISx3_OUT |	/* LED 3 */
		TRISx4_IN  |	/* Not used (R1, Absent) */
		TRISx5_OUT ;	/* LED 4 */
	PORTA = 0x00;
	ADCON1 = 0x07;

	/*
	 * Setup port B
	 */
	TRISB = TRISx0_IN  |	/* Button Start/Pause */
		TRISx1_IN  |	/* Over heat detector (U4) */
		TRISx2_OUT |	/* LED Water sensor */
		TRISx3_OUT |	/* Relay Water valve (RL1) */
		TRISx4_IN  |	/* Cat Sensor */
		TRISx5_IN  |	/* Button Auto setup */
		TRISx6_IN  |	/* PGM Clock */
		TRISx7_IN  ;	/* PGM Data */
	PORTB = 0x00;
	/* Turn on internal weak pull-up resitors on inputs */
	RBPU = 0;
	/* Clear the interrupt status */
	RBIF = 0;
	/* Enable interrupts */
	RBIE = 1;

	/* Turn on the water sensor LED */
	PORTB = WATERSENSOR_LED_MASK;

	/*
	 * Setup port C
	 */
	TRISC = TRISx0_OUT |	/* LED Error */
		TRISx1_OUT |	/* Beeper */
		TRISx2_OUT |	/* Cat Sensor LED */
		TRISx3_IN  |	/* I2C SCL */
		TRISx4_IN  |	/* I2C SDA */
		TRISx5_OUT |	/* LED 1 */
		TRISx6_IN  |	/* UART TxD */
		TRISx7_IN  ;	/* UART RxD */
	PORTC = 0x00;

	/*
	 * Setup port D
	 */
	TRISD = TRISx0_IN  |	/* ? */
		TRISx1_OUT |	/* Pump on/off (RL3) */
		TRISx2_OUT |	/* Blow dryer on/off (RL2) */
		TRISx3_OUT |	/* Dosage pump on/off (RL4) */
		TRISx4_OUT |	/* Bowl cw/ccw (RL7) */
		TRISx5_OUT |	/* Bowl on/off (RL5) */
		TRISx6_OUT |	/* Arm up/down (RL8) */
		TRISx7_OUT ;	/* Arm on/off (RL6) */
	PORTD = 0x00;

	/*
	 * Setup port E
	 */
	TRISE = TRISx0_OUT |	/* LED Cartridge */
		TRISx1_OUT |	/* LED Cat */
		TRISx2_OUT ;	/* LED Child Lock */
	PORTE = 0x00;

	PORTA_old = PORTA;
	PORTB_old = PORTB;

	/* Copy the initial states into the debouncer states */
	for (temp = 0; temp < DEBOUNCER_MAX; temp++) {
		unsigned char	tempmask = debouncers[temp].port_mask; /* for compiler limitations */

		debouncers[temp].state = *debouncers[temp].port & tempmask;
	}
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
	unsigned char	temp ;
	unsigned char	status ;

	/* Poll Port A inputs for changes */
	status    = PORTA;
	temp      = status ^ PORTA_old;
	PORTA_old = status;
	if (temp & WATERSENSOR_MASK)
		settimeout(&debouncers[DEBOUNCER_SENSOR_WATER].timer,
			   debouncers[DEBOUNCER_SENSOR_WATER].timeout);

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

void catsensor_event (unsigned char detected)
/******************************************************************************/
/* Function:	catsensor_event						      */
/*		- Handle state changes of cat sensor			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if (detected)
		set_LED_Cat(0x55, 1);
	else
		set_LED_Cat(0x0, 0);
}
/* catsensor_isr */


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
	/* Reset the mask to the first bit */
	pacers[PACER_LED_ERROR].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_ERROR].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_ERROR].repeat = repeat;
}

void set_LED_Locked (unsigned char pattern, unsigned char repeat)
{
	/* Reset the mask to the first bit */
	pacers[PACER_LED_LOCKED].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_LOCKED].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_LOCKED].repeat = repeat;
}

void set_LED_Cartridge (unsigned char pattern, unsigned char repeat)
{
	/* Reset the mask to the first bit */
	pacers[PACER_LED_CARTRIDGE].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_CARTRIDGE].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_CARTRIDGE].repeat = repeat;
}

void set_LED_Cat (unsigned char pattern, unsigned char repeat)
{
	/* Reset the mask to the first bit */
	pacers[PACER_LED_CAT].mask = 0x01;
	/* Copy the beep pattern */
	pacers[PACER_LED_CAT].pattern = pattern;
	/* Copy the repeat flag */
	pacers[PACER_LED_CAT].repeat = repeat;
}

void set_Beeper (unsigned char pattern, unsigned char repeat)
{
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
		BOWL_PORT &= ~(BOWL_MASK_CWCCW | BOWL_MASK_ONOFF);
		break;
	case BOWL_CW:
		BOWL_PORT &= ~BOWL_MASK_CWCCW;
		BOWL_PORT |=  BOWL_MASK_ONOFF;
		break;
	case BOWL_CCW:
		BOWL_PORT |= BOWL_MASK_CWCCW;
		BOWL_PORT |= BOWL_MASK_ONOFF;
		break;
	}
}

void set_Arm (unsigned char mode)
{
	switch (mode) {
	default:
	case ARM_STOP:
		ARM_PORT &= ~(ARM_MASK_UPDOWN | ARM_MASK_ONOFF);
		break;
	case ARM_UP:
		ARM_PORT &= ~ARM_MASK_UPDOWN;
		ARM_PORT |=  ARM_MASK_ONOFF;
		break;
	case ARM_DOWN:
		ARM_PORT |= ARM_MASK_UPDOWN;
		ARM_PORT |= ARM_MASK_ONOFF;
		break;
	}
}

void set_Water (unsigned char on)
{
	if (on)
		WATER_PORT |= WATER_MASK;
	else
		WATER_PORT &= !WATER_MASK;
}

void set_Dosage (unsigned char on)
{
	if (on)
		DOSAGE_PORT |= DOSAGE_MASK;
	else
		DOSAGE_PORT &= !DOSAGE_MASK;
}

void set_Pump (unsigned char on)
{
	if (on)
		PUMP_PORT |= PUMP_MASK;
	else
		PUMP_PORT &= !PUMP_MASK;
}


void set_Dryer	(unsigned char on)
{
	if (on)
		DRYER_PORT |= DRYER_MASK;
	else
		DRYER_PORT &= !DRYER_MASK;
}

/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void watersensor_event (unsigned char detected)
/******************************************************************************/
/* Function:	watersensor_event					      */
/*		- Handle state changes of water sensor			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	set_LED(1, detected);
}
/* watersensor_event */


static void heatsensor_event (unsigned char detected)
/******************************************************************************/
/* Function:	heatsensor_event					      */
/*		- Handle state changes of over-heat sensor		      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* heatsensor_event */


static void startbutton_event (unsigned char up)
/******************************************************************************/
/* Function:	startbutton_event					      */
/*		- Handle state changes of Start button			      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if (!up)
		set_Beeper(1,0);
	set_LED(4, !up);
	set_Water(!up);
}
/* startbutton_event */


static void setupbutton_event (unsigned char up)
/******************************************************************************/
/* Function:	setupbutton_event					      */
/*		- Handle state changes of Setup button			      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if (!up)
		set_Beeper(1,0);
	set_LED(3, !up);
	set_Pump(!up);
}
/* setupbutton_event */
