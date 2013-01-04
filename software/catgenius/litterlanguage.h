/******************************************************************************/
/* File    :	litterlanguage.h					      */
/* Function:	Header file of 'litterlanguage.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef LIITERLANGUAGE_H			/* Include file already compiled? */
#define LIITERLANGUAGE_H

#define EVENT_LEVEL_CHANGED	0	/* Washing detergent level has changed */
#define EVENT_ERR_FILLING	1	/* A timeout occured while filling the box, program is paused */
#define EVENT_ERR_DRAINING	2	/* A timeout occured while draining the box, program is paused */
#define EVENT_ERR_OVERHEAT	3	/* The dryer overheated, program is paused */
#define EVENT_ERR_EXECUTION	4	/* An execution error occured, program is stopped */
#define EVENT_ERR_FLOOD		5	/* Water was detected spontaneously */

#define SRC_ROM			0	/* Program is executed from ROM (.text) */
#define SRC_FLASH		1	/* Program is executed from FLASH memory */
#define SRC_RFID		2	/* Program is executed from RF-ID tag */
#define SRC_EEPROM		3	/* Program is executed from EEPROM */

#define FLAGS_AUTORUN		0x0100	/* Start program without user intervention */
#define FLAGS_DRYRUN		0x0200	/* Program supports dry mode */
#define FLAGS_WETRUN		0x0400	/* Program supports wet mode */
#define FLASH_OVERWRITE		0x0800	/* Program will be copied to FLASH to replace standard program */

#define INS_START		0x00	/* Designates the start of a program. Argument is INS_LAST or-ed with flags */
#define INS_BOWL		0x01	/* Controls the bowl. Argument is what the bowl should do */
#define INS_ARM			0x02	/* Controls the arm. Argument is what the arm should do */
#define INS_WATER		0x03	/* Controls the water valve. Argument is 1 for on, 0 for off */
#define INS_PUMP		0x04	/* Controls the hopper pump. Argument is 1 for on, 0 for off */
#define INS_DRYER		0x05	/* Controls the dryer fan. Argument is 1 for on, 0 for off */
#define INS_AUTODOSE		0x06	/* Controls the dosage pump. Argument x100 is amount in microliters */
#define INS_WAITTIME		0x07	/* Waits a period of time. Argument is period in milliseconds */
#define INS_WAITWATER		0x08	/* Waits for a water sensor state. Argument is state: 1 for high, 0 for low */
#define INS_WAITDOSAGE		0x09	/* Waits for autodosage to complete. Argument is ignored */
#define INS_SKIPIFDRY		0x0A	/* Skips argument instructions if the program runs in dry mode */
#define INS_SKIPIFWET		0x0B	/* Skips argument instructions if the program runs in wet mode */
#define INS_CALL		0x0C	/* Call a subroutine. Argument is the address on the program medium */
#define INS_RETURN		0x0D	/* Return from all a subroutine. Argument is ignored */
#define INS_END			0x0E


/* Types */
struct instruction {
	unsigned char	opcode;
	unsigned int	operant;
};


/* Generic */
void		litterlanguage_init	(unsigned char	flags) ;
void		litterlanguage_work	(void) ;

/* Control */
void		litterlanguage_start	(unsigned char	wet) ;
unsigned char	litterlanguage_running	(void) ;
void		litterlanguage_pause	(unsigned char	pause) ;
unsigned char	litterlanguage_paused	(void) ;
void		litterlanguage_stop	(void) ;


#endif /* LIITERLANGUAGE_H */
