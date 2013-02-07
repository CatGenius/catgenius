/* Local preferences for customizing CatGenius */

#ifndef CATGENIUS_PREFS_H
#define CATGENIUS_PREFS_H

#ifndef INCLUDED_FROM_APP_PREFS_H
#  error This file should only be included from app_prefs.h
#endif

// ------
// Select the options you wish to enable here
// ------
#define HAS_CR14
#define HAS_I2C
#define HAS_SERIAL
#define HAS_DEBUG
//#define HAS_BLUETOOTH
#define HAS_COMMANDLINE
#define HAS_COMMANDLINE_BOX
#define HAS_COMMANDLINE_GPIO
#define HAS_COMMANDLINE_EXTRA
#define HAS_COMMANDLINE_TAG
#define HAS_COMMANDLINE_COMTESTS
#define HAS_EVENTLOG

// ------
// 16F877A has tight memory - force features disabled
// ------
#ifdef _16F877A
#	undef HAS_BLUETOOTH
#	undef HAS_COMMANDLINE_BOX
#	undef HAS_COMMANDLINE_GPIO
#	undef HAS_COMMANDLINE_EXTRA
#	undef HAS_COMMANDLINE_TAG
#	undef HAS_COMMANDLINE_COMTESTS
#	undef HAS_EVENTLOG
#endif

// ------
// Uncomment one of the following to set inital serial line terminator
// ------
//#define SERIAL_LINETERM_LF_FORCE
//#define SERIAL_LINETERM_CR_FORCE
#define SERIAL_LINETERM_CRLF_FORCE

// -----
// Uncomment/change the following line to set the name your Bluetooth device will advertise
// -----
#define BLUETOOTH_NAME	"CatGenius"

// ------
// DO NOT CHANGE -- Automatically include dependencies
// ------
#if (defined HAS_COMMANDLINE_BOX) || (defined HAS_COMMANDLINE_GPIO) || (defined HAS_COMMANDLINE_EXTRA) || (defined HAS_COMMANDLINE_TAG)
#  define HAS_COMMANDLINE
#endif
#ifdef HAS_COMMANDLINE_TAG
#  define HAS_CR14
#endif
#ifdef HAS_CR14
#  define HAS_I2C
#endif
#if (defined HAS_BLUETOOTH) || (defined HAS_COMMANDLINE) || (defined HAS_DEBUG)
#  define HAS_SERIAL
#endif

// Temporary hacks to get things working for testing
//#define _CMM_TEMP_HACKS_


#endif // CATGENIUS_PREFS_H
