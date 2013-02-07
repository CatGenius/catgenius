/* Redirect file to preferences include file for current target application */

#ifndef APP_PREFS_H
#define APP_PREFS_H

#define INCLUDED_FROM_APP_PREFS_H

#if defined APP_CARTRIDGEGENIUS
#  include "../cartridgegenius/cartridgegenius_prefs.h"
#elif defined APP_CATGENIUS
#  include "../catgenius/catgenius_prefs.h"
#elif defined APP_GENIEDIAG
#  include "../catgenius/geniediag_prefs.h"
#elif defined APP_IOTESTER
#  include "../catgenius/iotester_prefs.h"
#else
   /*
    * If you get this error, you need to add one of the above
    * to your project settings:
    * Project -> Build Options... -> Project
    * Tab 'Compiler', Define marcros, Add
    */
#  error No application defined!
#endif

#undef INCLUDED_FROM_APP_PREFS_H

#endif // APP_PREFS_H