/******************************************************************************/
/* File    :    bluetooth.h                                                   */
/* Function:    Include file for 'bluetooth.c'.                               */
/* Author  :    Christopher Mapes                                             */
/******************************************************************************/

#ifndef BLUETOOTH_H

// Uncomment to have the BT module name itself "CatGenius"
//#define USE_BT_NAME

// Default Bluetooth Bitrate
#define BT_BITRATE		115200UL

void bluetooth_init(void);

#endif