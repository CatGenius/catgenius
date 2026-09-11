/* Preincluded only by the host integration build; never by a PIC build. */
#ifndef TEST_INTEGRATION_H
#define TEST_INTEGRATION_H

#include <stdio.h>
#include "../common/hardware.h"

/* Native host instruction pointers do not support PIC address formatting. */
#undef LL_DEBUG
int integration_printf(const char *format, ...);
#define printf integration_printf

#endif
