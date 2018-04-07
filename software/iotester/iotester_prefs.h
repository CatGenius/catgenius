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
#define HAS_SERIAL
#define HAS_COMMANDLINE
#define HAS_COMMANDLINE_GPIO

#endif /* IOTESTER_PREFS_H */
