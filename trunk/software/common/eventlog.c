/* Tracks UI and sensor/actuator events */

#include "../common/app_prefs.h"

#ifdef HAS_EVENTLOG

#define EVENTLOG_C

#include <string.h>
#include <stdio.h>
#include "../common/eventlog.h"
#include "../common/cmdline.h"
#include "../common/serial.h"

static BOOL eventlog_tracking = false;

// TBD: Compress the memory for this
static _U08 eventlog_state[EVENTLOG_MAX];

static void eventlog_write(_U08 index, _U16 value)
{
	TX3("<e i=%d v=%d />\n", index, value);
}

void eventlog_init(void)
{
	for (_U08 i=0; i<EVENTLOG_MAX; i++) eventlog_state[i] = 0;
}

void eventlog_work(void)
{
	// Nothing to do
}

void eventlog_track(_U08 index, _U16 value)
{
	// If there is no change, there is nothing to do
	if (value == eventlog_state[index]) return;

	// Log event & Update
	if (eventlog_tracking) eventlog_write(index, value);
	eventlog_state[index] = value;
}

void eventlog_start()
{
	// - Go ahead and re-send event info if requested - if (eventlog_tracking) return;
	eventlog_tracking = true;
	
	//TX("<l n=e>\n");
	for (_U08 i=0; i<EVENTLOG_MAX; i++)
		if (eventlog_state[i] != 0) eventlog_write(i, eventlog_state[i]);
	//TX("</l>\n");
}

void eventlog_stop()
{
	eventlog_tracking = false;
}

int cmd_evt(int argc, char* argv[])
{
	if (argc > 2) return ERR_SYNTAX;

	if (argc == 2)
	{
		if (stricmp(argv[1], "on"))
			eventlog_stop();
		else
			eventlog_start();
	}

	TX2("Event: %s\n", eventlog_tracking ? "on" : "off");

	return ERR_OK;
}

#endif