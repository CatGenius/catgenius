/*
 * This file's sole intention is to easily use the same common
 * files on different hardware platforms in the same repository.
 */
#if defined HW_CATGENIE120
#  include "catgenie120.h"
#elif defined HW_CARTRIDGEGENIUS_WS0
#  include "cartridgegenius.h"
#elif defined HW_CARTRIDGEGENIUS_WS1
#  include "cartridgegenius.h"
#else
   /*
    * If you get this error, you need to add either one of the above
    * (HW_CATGENIE120, HW_CARTRIDGEGENIUS_WS0 or HW_CARTRIDGEGENIUS_WS1)
    * to your project settings:
    * Project -> Build Options... -> Project
    * Tab 'Compiler', Define marcros, Add
    */
#  error No hardware platform defined!
#endif