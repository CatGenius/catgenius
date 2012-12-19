/*
 * This file's sole intention is to easily use the same common
 * files on different hardware platforms in the same repository.
 */
#if defined HW_CATGENIE120
#  include "../common/catgenie120.h"
#elif defined HW_CATGENIE120PLUS
#  include "../common/catgenie120.h"
#elif defined HW_CARTRIDGEGENIUS_WS0
#  include "../common/cartridgegenius.h"
#elif defined HW_CARTRIDGEGENIUS_WS1
#  include "../common/cartridgegenius.h"
#elif defined HW_CARTRIDGEGENIUS_WS2
#  include "../common/cartridgegenius.h"
#else
   /*
    * If you get this error, you need to add either one of the above
    * (HW_CATGENIE120, HW_CATGENIE120PLUS,
    *  HW_CARTRIDGEGENIUS_WS0 or HW_CARTRIDGEGENIUS_WS1 or HW_CARTRIDGEGENIUS_WS2
    * to your project settings:
    * Project -> Build Options... -> Project
    * Tab 'Compiler', Define marcros, Add
    */
#  error No hardware platform defined!
#endif