/******************************************************************************/
/* File    :	catgenie120.h						      */
/* Function:	Include file of 'catgenie120.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CATGENIE120_H			/* Include file already compiled? */
#define CATGENIE120_H

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
#define BITRATE			19200
#define	BITRATE_ERROR		4

/* Times */
#define MAX_FILLTIME		(( 2*60+15)*SECOND)
#define MAX_DRAINTIME		((10*60+ 0)*SECOND)

/* EEPROM layout */
#define NVM_VERSION		(0)
#define NVM_MODE		(1)
#define NVM_KEYUNDLOCK		(2)
#define NVM_BOXSTATE		(3)

/* Init return flags */
#define START_BUTTON_HELD	(0x01 << 0)
#define SETUP_BUTTON_HELD	(0x01 << 1)
#define RESERVED_1		(0x01 << 2)
#define RESERVED_2		(0x01 << 3)
#define RESERVED_3		(0x01 << 4)
#define RESERVED_4		(0x01 << 5)
#define RESERVED_5		(0x01 << 6)
#define POWER_FAILURE		(0x01 << 7)
#define BUTTONS			(START_BUTTON_HELD | SETUP_BUTTON_HELD)


/* Bowl modi */
#define BOWL_STOP		0
#define BOWL_CW			1
#define BOWL_CCW		2

/* Arm modi */
#define ARM_STOP		0
#define ARM_DOWN		1
#define ARM_UP			2

/* Mechanics */
#define DOSAGE_SECONDS_PER_ML	10		/* For 1 ml of cleaning liquid, 10 seconds of pumping */

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
#define	NOT_USED_1_PORT		PORTA	/* Not used (R39, Absent) */
#define	NOT_USED_1_MASK		BIT(0)
#define	NOT_USED_2_PORT		PORTA	/* Not used (R1, Absent) */
#define	NOT_USED_2_MASK		BIT(4)
#define	NOT_USED_3_PORT		PORTB	/* Not used, PGM Clock */
#define	NOT_USED_3_MASK		BIT(6)
#define	NOT_USED_4_PORT		PORTB	/* Not used, PGM Data */
#define	NOT_USED_4_MASK		BIT(7)

/* Special purpose pins */
#define	I2C_SCL_PORT		PORTC	/* I2C SCL */
#define	I2C_SCL_MASK		BIT(3)
#define	I2C_SDA_PORT		PORTC	/* I2C SDA */
#define	I2C_SDA_MASK		BIT(4)
#define	UART_TXD_PORT		PORTC	/* UART TxD */
#define	UART_TXD_MASK		BIT(6)
#define	UART_RXD_PORT		PORTC	/* UART RxD */
#define	UART_RXD_MASK		BIT(7)

/* Buttons */
#define STARTBUTTON_PORT	PORTB	/* Button Start/Pause */
#define STARTBUTTON_MASK	BIT(0)
#define SETUPBUTTON_PORT	PORTB	/* Button Auto setup */
#define SETUPBUTTON_MASK	BIT(5)

/* Indicators */
#define LED_1_PORT		PORTC	/* LED 1 */
#define LED_1_MASK		BIT(5)
#define LED_2_PORT		PORTA	/* LED 2 */
#define LED_2_MASK		BIT(2)
#define LED_3_PORT		PORTA	/* LED 3 */
#define LED_3_MASK		BIT(3)
#define LED_4_PORT		PORTA	/* LED 4 */
#define LED_4_MASK		BIT(5)
#define LED_ERROR_PORT		PORTC	/* LED Error */
#define LED_ERROR_MASK		BIT(0)
#define BEEPER_PORT		PORTC	/* Beeper */
#define BEEPER_MASK		BIT(1)
#define LED_CARTRIDGE_PORT	PORTE	/* LED Cartridge */
#define LED_CARTRIDGE_MASK	BIT(0)
#define LED_CAT_PORT		PORTE	/* LED Cat */
#define LED_CAT_MASK		BIT(1)
#define LED_LOCKED_PORT		PORTE	/* LED Child Lock */
#define LED_LOCKED_MASK		BIT(2)

/* Actuators */
#define WATERSENSORANALOG_PORT	PORTA	/* Analog water sensor input */
#define WATERSENSORANALOG_MASK	BIT(1)
#define	WATERSENSORPULLUP_PORT	PORTD	/* Pull-up for water sensor input */
#define	WATERSENSORPULLUP_MASK	BIT(0)
#define PUMP_PORT		PORTD	/* Pump on/off (RL3) */
#define PUMP_MASK		BIT(1)
#define DRYER_PORT		PORTD	/* Blow dryer on/off (RL2) */
#define DRYER_MASK		BIT(2)
#define DOSAGE_PORT		PORTD	/* Dosage pump on/off (RL4) */
#define DOSAGE_MASK		BIT(3)
#define BOWL_PORT		PORTD
#define BOWL_CWCCW_MASK		BIT(4)	/* Bowl cw/ccw (RL7) */
#define BOWL_ONOFF_MASK		BIT(5)	/* Bowl on/off (RL5) */
#define ARM_PORT		PORTD
#define ARM_UPDOWN_MASK		BIT(6)	/* Arm up/down (RL8) */
#define ARM_ONOFF_MASK		BIT(7)	/* Arm on/off (RL6) */

/* Sensors */
#define CATSENSOR_LED_PORT	PORTC	/* Cat Sensor LED */
#define CATSENSOR_LED_MASK	BIT(2)
#define CATSENSOR_PORT		PORTB	/* Cat Sensor */
#define CATSENSOR_MASK		BIT(4)
#define WATERSENSOR_LED_PORT	PORTB	/* LED Water sensor */
#define WATERSENSOR_LED_MASK	BIT(2)
#define WATERSENSOR_PORT	PORTB	/* Water sensor, directly controls Water valve (RL1) */
#define WATERSENSOR_MASK	BIT(3)
#define HEATSENSOR_PORT		PORTB	/* Over heat detector (U4) */
#define HEATSENSOR_MASK		BIT(1)

/* Generic */
unsigned char	catgenie_init		(void) ;
void		catgenie_work		(void) ;
void		catgenie_term		(void) ;

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
void		set_Water		(unsigned char on);
unsigned char	get_Water		(void);
void		set_Dosage		(unsigned char on);
unsigned char	get_Dosage		(void);
void		set_Pump		(unsigned char on);
unsigned char	get_Pump		(void);
void		set_Dryer		(unsigned char on);
unsigned char	get_Dryer		(void);

#endif /* CATGENIE120_H */
