/******************************************************************************/
/* File    :	userinterface.h						      */
/* Function:	Definition of ports, pins and their functions.		      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef USERINTERFACE_H				/* Include file already compiled? */
#define USERINTERFACE_H

#ifdef USERINTERFACE_C
#  include "../common/prot_src.h"
#else
#  include "../common/prot_inc.h"
#endif

#define AUTO_MANUAL			0	/* Nothing automatic, manual only */
#define AUTO_TIMED1			1	/* Full wash every 24 hours */
#define AUTO_TIMED2			2	/* Full wash every 12 hours */
#define AUTO_TIMED3			3	/* Full wash every 8 hours */
#define AUTO_TIMED4			4	/* Full wash every 6 hours */
#define AUTO_DETECTED1ON1	5	/* Full wash every use */
#define AUTO_DETECTED1ON2	6	/* Full wash/Scoop only 1:2 uses */
#define AUTO_DETECTED1ON3	7	/* Full wash/Scoop only 1:3 uses */
#define AUTO_DETECTED1ON4	8	/* Full wash/Scoop only 1:4 uses */
#define AUTO_DETECTED		9	/* Scoop only every use*/

#define PANEL_AUTOMODE		0	/* Display/button mode in normal operation */
#define PANEL_CARTRIDGELEVEL	1	/* Display/button mode showing/altering cartridge level */
#define PANEL_ERROR		2	/* Display/button mode showing error(s) */
#ifdef HAS_DIAG
#define PANEL_DIAG		3	/* GenieDiag Mode */
#endif

/* Global Data */
PUBLIC_VAR(bit cat_detected, 0);
PUBLIC_VAR(bit error_overheat, 0);
PUBLIC_VAR(bit locked, 0);
PUBLIC_VAR(unsigned char auto_mode, AUTO_MANUAL);
PUBLIC_VAR(unsigned char panel_mode, PANEL_AUTOMODE);

/* Generic */
PUBLIC_FN(void userinterface_init(unsigned char flags));
PUBLIC_FN(void userinterface_work(void));

/* Control */
PUBLIC_FN(void userinterface_set_mode(unsigned char mode));
PUBLIC_FN(void update_display (void));
PUBLIC_FN(void setup_short(void));
PUBLIC_FN(void setup_long(void));
PUBLIC_FN(void start_short(void));
PUBLIC_FN(void start_long(void));
PUBLIC_FN(void both_short(void));
PUBLIC_FN(void both_long(void));

#endif /* USERINTERFACE_H */
