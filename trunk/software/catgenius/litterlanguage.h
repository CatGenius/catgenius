/******************************************************************************/
/* File    :	litterlanguage.h					      */
/* Function:	Header file of 'litterlanguage.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef LIITERLANGUAGE_H			/* Include file already compiled? */
#define LIITERLANGUAGE_H

#define SRC_ROM		0
#define SRC_FLASH	1
#define SRC_RFID	2
#define SRC_EEPROM	3

#define FLAGS_AUTORUN	0x0100	/* Start program without user intervention */
#define FLAGS_DRYRUN	0x0200	/* Program supports dry mode */
#define FLAGS_WETRUN	0x0400	/* Program supports wet mode */
#define FLASH_OVERWRITE	0x0800	/* Program will be copied to FLASH to replace standard program */

#define CMD_START	0x00	/* Designates the start of a program. Argument is CMD_LAST or-ed with flags */
#define CMD_BOWL	0x01	/* Controls the bowl. Argument is what the bowl should do */
#define CMD_ARM		0x02	/* Controls the arm. Argument is what the arm should do */
#define CMD_WATER	0x03	/* Controls the water valve. Argument is 1 for on, 0 for off */
#define CMD_PUMP	0x04	/* Controls the hopper pump. Argument is 1 for on, 0 for off */
#define CMD_DRYER	0x05	/* Controls the dryer fan. Argument is 1 for on, 0 for off */
#define CMD_AUTODOSE	0x06	/* Controls the dosage pump. Argument x100 is amount in microliters */
#define CMD_WAITTIME	0x07	/* Waits a period of time. Argument is period in milliseconds */
#define CMD_WAITWATER	0x08	/* Waits for a water sensor state. Argument is state: 1 for high, 0 for low */
#define CMD_WAITDOSAGE	0x09	/* Waits for autodosage to complete. Argument is ignored */
#define CMD_SKIPIFDRY	0x0A	/* Skips argument instructions if the program runs in dry mode */
#define CMD_SKIPIFWET	0x0B	/* Skips argument instructions if the program runs in wet mode */
#define CMD_CALL	0x0C	/* Call a subroutine. Argument is the address on the program medium */
#define CMD_RETURN	0x0D	/* Return from all a subroutine. Argument is ignored */
#define CMD_END		0x0E


/* Types */
struct command {
	unsigned char	cmd;
	unsigned int	arg;
};


/* Generic */
void		litterlanguage_init	(void) ;
void		litterlanguage_work	(void) ;
void		litterlanguage_term	(void) ;

/* Control */
void		litterlanguage_start	(unsigned char	wet) ;
unsigned char	litterlanguage_running	(void) ;
void		litterlanguage_pause	(unsigned char	pause) ;


#endif /* LIITERLANGUAGE_H */
