/******************************************************************************/
/* File    :	catgenie120.h						      */
/* Function:	Include file of 'catgenie120.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CATGENIE120_H			/* Include file already compiled? */
#define CATGENIE120_H

#if !(defined _16F877A) && !(defined _16F1939)
#  error Unsupported processor selected!
#endif

#ifdef _16F1939
/* Analog water sensor readout is NOT supported on a 16F877A */
/* On a 16F1939 it is optional */
#  define WATERSENSOR_ANALOG
#endif /* _16F1939 */

/* Version number */
#define VERSION			(2)		/* NVM layout version */

/* Miscelaneous */
#define BIT(n)			(1U << (n))	/* Bit mask for bit 'n' */
#define DBG			printf

/* Frequencies */
#define KHZ			(1000UL)
#define MHZ			(1000UL * (KHZ))
#define _XTAL_FREQ		(4UL * MHZ)	/* Crystal frequency */
#define MAINS_FREQ		50		/* 50 Hz mains frequency */

/* Serial port */
#define BITRATE			19200UL

/* Times */
#define MAX_FILLTIME		((2*60+15)*SECOND)
#define MAX_DRAINTIME		((0*60+10)*SECOND)

/* EEPROM layout */
#define NVM_VERSION		(0)
#define NVM_MODE		(1)
#define NVM_KEYUNDLOCK		(2)
#define NVM_BOXSTATE		(3)

/* Init return flags */
#define START_BUTTON		(0x01 << 0)
#define SETUP_BUTTON		(0x01 << 1)
#define RESERVED_1		(0x01 << 2)
#define RESERVED_2		(0x01 << 3)
#define RESERVED_3		(0x01 << 4)
#define RESERVED_4		(0x01 << 5)
#define RESERVED_5		(0x01 << 6)
#define POWER_FAILURE		(0x01 << 7)
#define BUTTONS			(START_BUTTON | SETUP_BUTTON)


/* Bowl modi */
#define BOWL_STOP		0
#define BOWL_CW			1
#define BOWL_CCW		2

/* Arm modi */
#define ARM_STOP		0
#define ARM_DOWN		1
#define ARM_UP			2

/* Mechanics */
#define DOSAGE_SECONDS_PER_ML	10	/* For 1 ml of cleaning liquid, 10 seconds of pumping */
#define CARTRIDGECAPACITY_ML	450	/* A full cartridge contains 450 ml of detergent */

#define BOWL_MOTOR_RPM		52
#define BOWL_MOTOR_TEETH	12
#define BOWL_TEETH_PER_SECOND	((BOWL_MOTOR_RPM) * (BOWL_MOTOR_TEETH) / 60)
#define BOWL_TEETH_REV		174
#define BOWL_REV_MSEC		(1000 * (BOWL_TEETH_REV) / (BOWL_TEETH_PER_SECOND))

#define ARM_MOTOR_RPM		5
#define ARM_MOTOR_TEETH		16
#define ARM_TEETH_PER_SECOND	((ARM_MOTOR_RPM) * (ARM_MOTOR_TEETH) / 60))
#define ARM_TEETH_STOKE		18
#define ARM_STROKE_MSEC		13500

/* Free pins */
#define	NOT_USED_1(reg)		reg##A	/* Not used (R39, Absent) */
#define	NOT_USED_1_MASK		BIT(0)
#define	NOT_USED_2(reg)		reg##A	/* Not used (R1, Absent) */
#define	NOT_USED_2_MASK		BIT(4)
#define	NOT_USED_3(reg)		reg##B	/* Not used, PGM Clock */
#define	NOT_USED_3_MASK		BIT(6)
#define	NOT_USED_4(reg)		reg##B	/* Not used, PGM Data */
#define	NOT_USED_4_MASK		BIT(7)

/* Special purpose pins */
#define	I2C_SCL(reg)		reg##C	/* I2C SCL */
#define	I2C_SCL_MASK		BIT(3)
#define	I2C_SDA(reg)		reg##C	/* I2C SDA */
#define	I2C_SDA_MASK		BIT(4)
#define	UART_TXD(reg)		reg##C	/* UART TxD */
#define	UART_TXD_MASK		BIT(6)
#define	UART_RXD(reg)		reg##C	/* UART RxD */
#define	UART_RXD_MASK		BIT(7)

/* Buttons */
#define STARTBUTTON(reg)	reg##B	/* Button Start/Pause */
#define STARTBUTTON_MASK	BIT(0)
#define SETUPBUTTON(reg)	reg##B	/* Button Auto setup */
#define SETUPBUTTON_MASK	BIT(5)

/* Indicators */
#define LED_1(reg)		reg##C	/* LED 1 */
#define LED_1_MASK		BIT(5)
#define LED_2(reg)		reg##A	/* LED 2 */
#define LED_2_MASK		BIT(2)
#define LED_3(reg)		reg##A	/* LED 3 */
#define LED_3_MASK		BIT(3)
#define LED_4(reg)		reg##A	/* LED 4 */
#define LED_4_MASK		BIT(5)
#define LED_ERROR(reg)		reg##C	/* LED Error */
#define LED_ERROR_MASK		BIT(0)
#define BEEPER(reg)		reg##C	/* Beeper */
#define BEEPER_MASK		BIT(1)
#define LED_CARTRIDGE(reg)	reg##E	/* LED Cartridge */
#define LED_CARTRIDGE_MASK	BIT(0)
#define LED_CAT(reg)		reg##E	/* LED Cat */
#define LED_CAT_MASK		BIT(1)
#define LED_LOCKED(reg)		reg##E	/* LED Child Lock */
#define LED_LOCKED_MASK		BIT(2)

/* Actuators */
#define WATERSENSORANALOG(reg)	reg##A	/* Analog water sensor input */
#define WATERSENSORANALOG_MASK	BIT(1)
#define	WATERVALVEPULLUP(reg)	reg##D	/* Pull-up for water valve driver */
#define	WATERVALVEPULLUP_MASK	BIT(0)
#define PUMP(reg)		reg##D	/* Pump on/off (RL3) */
#define PUMP_MASK		BIT(1)
#define DRYER(reg)		reg##D	/* Blow dryer on/off (RL2) */
#define DRYER_MASK		BIT(2)
#define DOSAGE(reg)		reg##D	/* Dosage pump on/off (RL4) */
#define DOSAGE_MASK		BIT(3)
#define BOWL(reg)		reg##D
#define BOWL_CWCCW_MASK		BIT(4)	/* Bowl cw/ccw (RL7) */
#define BOWL_ONOFF_MASK		BIT(5)	/* Bowl on/off (RL5) */
#define ARM(reg)		reg##D
#define ARM_UPDOWN_MASK		BIT(6)	/* Arm up/down (RL8) */
#define ARM_ONOFF_MASK		BIT(7)	/* Arm on/off (RL6) */

/* Sensors */
#define CATSENSOR_LED(reg)	reg##C	/* Cat Sensor LED */
#define CATSENSOR_LED_MASK	BIT(2)
#define CATSENSOR(reg)		reg##B	/* Cat Sensor */
#define CATSENSOR_MASK		BIT(4)
#define WATERSENSOR_LED(reg)	reg##B	/* LED Water sensor */
#define WATERSENSOR_LED_MASK	BIT(2)
#define WATERVALVE(reg)		reg##B	/* Water sensor, directly controls Water valve (RL1) */
#define WATERVALVE_MASK		BIT(3)
#define HEATSENSOR(reg)		reg##B	/* Over heat detector (U4) */
#define HEATSENSOR_MASK		BIT(1)

#if (defined _16F877A)
/* Substitute non-available output latch registers with port registers */
#define LATA			PORTA
#define LATB			PORTB
#define LATC			PORTC
#define LATD			PORTD
#define LATE			PORTE
#endif /* _16F877A */


/* Generic */
unsigned char	catgenie_init		(void) ;
void		catgenie_work		(void) ;

/* Indicators */
void		set_LED			(unsigned char led,
					 unsigned char on);
void		set_LED_Cat		(unsigned char pattern,
					 unsigned char repeat);
void		set_LED_Error		(unsigned char pattern,
					 unsigned char repeat);
void		set_LED_Cartridge	(unsigned char pattern,
					 unsigned char repeat);
void		set_LED_Locked		(unsigned char pattern,
					 unsigned char repeat);
void		set_Beeper		(unsigned char pattern,
					 unsigned char repeat) ;

/* Actuators */
void		set_Bowl		(unsigned char mode);
unsigned char	get_Bowl		(void);
void		set_Arm			(unsigned char mode);
unsigned char	get_Arm			(void);
void		set_Dosage		(unsigned char on);
unsigned char	get_Dosage		(void);
void		set_Pump		(unsigned char on);
unsigned char	get_Pump		(void);
void		set_Dryer		(unsigned char on);
unsigned char	get_Dryer		(void);

#endif /* CATGENIE120_H */
