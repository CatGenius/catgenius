/******************************************************************************/
/* File    :    bluetooth.h                                                   */
/* Function:    Include file for 'bluetooth.c'.                               */
/* Author  :    Christopher Mapes                                             */
/******************************************************************************/

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "../common/app_prefs.h"

#ifdef HAS_BLUETOOTH

#ifdef BLUETOOTH_C
#	include "../common/prot_src.h"
#else
#	include "../common/prot_inc.h"
#endif

PUBLIC_FN(void bluetooth_init(void));

#else // HAS_BLUETOOTH

#define bluetooth_init()

#endif // HAS_BLUETOOTH

#endif // !BLUETOOTH_H