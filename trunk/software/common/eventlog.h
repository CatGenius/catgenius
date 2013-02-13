/* Tracks UI and sensor/actuator events */

#ifndef EVENTLOG_H
#define EVENTLOG_H

#include "../common/app_prefs.h"

#ifdef HAS_EVENTLOG

#ifdef EVENTLOG_C
#  include "../common/prot_src.h"
#else
#  include "../common/prot_inc.h"
#endif

#include "../common/types.h"

#define EVENTLOG_BOWL			0
#define EVENTLOG_ARM			1
#define EVENTLOG_DOSAGE			2
#define EVENTLOG_PUMP			3
#define EVENTLOG_DRYER			4
#define EVENTLOG_TAP			5
#define EVENTLOG_WET_SENSOR		6
#define EVENTLOG_HEAT_SENSOR	7
#define EVENTLOG_START_BUTTON	8
#define EVENTLOG_SETUP_BUTTON	9
#define EVENTLOG_CAT_SENSOR		10
#define EVENTLOG_LED			11	/* 11 .. 14 */
#define EVENTLOG_PACER			15	/* 15 .. 20 */
#define EVENTLOG_WATER_SENSOR	21
#define EVENTLOG_LL_ADDR		22
#define EVENTLOG_MAX			23

PUBLIC_FN(void eventlog_init(void));
PUBLIC_FN(void eventlog_work(void));
PUBLIC_FN(void eventlog_track(_U08 item, _U16 value));
PUBLIC_FN(void eventlog_start(void));
PUBLIC_FN(void eventlog_stop(void));
PUBLIC_FN(int cmd_evt(int argc, char *argv[]));

#else

#define eventlog_init()
#define eventlog_work()
#define eventlog_track(x, y)

#endif

#endif