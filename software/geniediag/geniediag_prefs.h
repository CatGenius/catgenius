/* Local preferences for customizing GenieDiag */

#ifndef GENIEDIAG_PREFS_H
#define GENIEDIAG_PREFS_H

#ifndef INCLUDED_FROM_APP_PREFS_H
#  error This file should only be included from app_prefs.h
#endif

/*
 * Select the options you wish to enable here
 * Approximate program memory requirements are listed to the right
 */
#define HAS_DEBUG
#define HAS_COMMANDLINE_BOX
#if (defined _16F1939)
#  define HAS_COMMANDLINE_TAG
#endif

/*
 * Automatically include dependencies (DO NOT CHANGE)
 */
#if (defined HAS_COMMANDLINE_BOX) || \
    (defined HAS_COMMANDLINE_TAG)
#  define HAS_COMMANDLINE
#endif

#if (defined HAS_COMMANDLINE)
#  define HAS_SERIAL
#endif

#ifdef HAS_COMMANDLINE_TAG
#  define HAS_CR14
#endif

#ifdef HAS_CR14
#  define HAS_I2C
#endif

#endif // GENIEDIAG_PREFS_H
