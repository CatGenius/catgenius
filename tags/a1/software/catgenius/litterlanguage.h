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
#define FLAGS_DRYRUN	0x0200	/* Program supports dry cleaning */
#define FLAGS_WETRUN	0x0400	/* Program supports wet cleaning */
#define FLASH_OVERWRITE	0x0800	/* Program will be copied to FLASH to replace standard program */

#define CMD_START	0x00	/* Designates the start of a program. Argument is CMD_LAST or-ed with flags */
#define CMD_BOWL	0x01	/* Controls the bowl. Argument is what the bowl should do */
#define CMD_ARM		0x02	/* Controls the arm. Argument is what the arm should do */
#define CMD_WATER	0x03	/* Controls the water valve. Argument is */
#define CMD_DOSAGE	0x04
#define CMD_PUMP	0x05
#define CMD_DRYER	0x06

#define CMD_WAITTIME	0x08
#define CMD_WAITWATER	0x09
#define CMD_SKIPIFDRY	0x10
#define CMD_SKIPIFWET	0x11
#define CMD_AUTODOSE	0x12
#define CMD_LAST	CMD_AUTODOSE
#define CMD_END		0xFE
#define CMD_RESERVED	0xFF


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
void		litterlanguage_mode	(unsigned char	justscoop) ;
void		litterlanguage_start	(void) ;
unsigned char	litterlanguage_running	(void) ;
void		litterlanguage_pause	(unsigned char	pause) ;


#endif /* LIITERLANGUAGE_H */
