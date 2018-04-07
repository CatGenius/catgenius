/* Local preferences for customizing common files */

#ifndef IOTESTER_PREFS_H
#define IOTESTER_PREFS_H

#ifndef INCLUDED_FROM_APP_PREFS_H
#  error This file should only be included from app_prefs.h
#endif

/*
 * Select the options you wish to enable here
 * Approximate program memory requirements are listed to the right
 */
#define HAS_COMMANDLINE_GPIO

/*
 * Automatically include dependencies (DO NOT CHANGE)
 */
#if (defined HAS_COMMANDLINE_GPIO)
#  define HAS_COMMANDLINE
#endif

#if (defined HAS_COMMANDLINE)
#  define HAS_SERIAL
#endif

#endif /* IOTESTER_PREFS_H */
