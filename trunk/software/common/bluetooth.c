/******************************************************************************/
/* File    :    bluetooth.c                                                   */
/* Function:    Contains bt_init() routine to initialize the Bluetooth module */
/*              Supported module is the Roving Networks Bluetooth Mate Silver */
/*              Product Pages:                                                */
/*                http://www.rovingnetworks.com/products/RN_42                */
/*                http://www.sparkfun.com/products/10393                      */
/* Author  :    Christopher Mapes                                             */
/******************************************************************************/

#include "../common/app_prefs.h"

#ifdef HAS_BLUETOOTH
#define BLUETOOTH_C

#include <htc.h>
#include <stdio.h>
#include <string.h>

#include "hardware.h"			/* Flexible hardware configuration */
#include "serial.h"
#include "bluetooth.h"
#include "types.h"

// Wait timeout for response from device
#define BT_WAIT_DELAY	1000UL	// 500UL

static const _U16 bt_bitrates[] = {BITRATE, 115200UL, 9600UL};

static void bluetooth_puts(const char *s)
{
	for (_U08 i=0; i<strlen(s); i++)
	{
		putch(s[i]);
		__delay_us(1);	// ~8000bps
	}
}

void bluetooth_init(void)
{
	const unsigned char *s_bitrate; //_U08 s_bitrate[5];
	_U08 i;
	_U08 ok;
	
	// TBD:
    // - Could write EEPROM when we've done this so we don't try again
    // - Or at least check nBOR and/or nPOR

	// Attempt to contact the BT module at different bitrates
	ok = false;
	for (i=0; i<3; i++)
	{
		// Set the bitrate
		serial_init(bt_bitrates[i], SERIAL_FLOW_NONE);

		// Put BT module in command mode.  It's expecting to see 3 "$" chars in a 1 sec window with silence
		bluetooth_puts("$$$");
		if (serial_wait_s("CMD\r\n", 1500) > 0)
		{
			ok = true;
			break;
		}
	}
	if (!ok) return;

	// If a preferred bluetooth device name was provided, set it now
#ifdef BLUETOOTH_NAME
	bluetooth_puts("S-,"); bluetooth_puts(BLUETOOTH_NAME); bluetooth_puts("%s\r");
	if (serial_wait_s("AOK\r\n", BT_WAIT_DELAY) == 0) return;

    bluetooth_puts("R,1\r");
	if (serial_wait_s("Reboot!\r\n", BT_WAIT_DELAY) == 0) return;

	// Put BT module back in command mode.  It's expecting to see exactly 3 "$" chars in a 1 sec window with no other characters
	__delay_ms(1000);
	bluetooth_puts("$$$");
	if (serial_wait_s("CMD\r\n", BT_WAIT_DELAY) == 0) return;
#endif

	if (bt_bitrates[i] != BITRATE)
	{
		// Bitrate is not what we want, so change it

		// Convert bitrate to string format accepted by BT module
		// TBD: Optimize this code.  I think it takes up a lot of program memory
		/*
		if (BITRATE <= 9600UL)
		{
			sprintf(s_bitrate, "%lu", BITRATE);
		}
		else if (BITRATE < 115200UL)
		{
			sprintf(s_bitrate, "%02.1f", ((float)BITRATE / 1000UL));
		}
		else
		{
			sprintf(s_bitrate, "%luK", BITRATE / 1000UL);
		}
		*/

		switch (BITRATE)
		{
			case   9600 : s_bitrate = "9600"; break;
			case  14400 : s_bitrate = "14.4"; break;
			case  19200 : s_bitrate = "19.2"; break;
			case  38400 : s_bitrate = "38.4"; break;
			case  57600 : s_bitrate = "57.6"; break;
			default		: s_bitrate = "115K"; break;			
		}

		// Store new bitrate for next reboot
		bluetooth_puts("SU,"); bluetooth_puts(s_bitrate); bluetooth_puts("N\r");
		if (serial_wait_s("AOK\r\n", BT_WAIT_DELAY) == 0) return;

		// Change current bitrate
		bluetooth_puts("U,"); bluetooth_puts(s_bitrate); bluetooth_puts("N\r");
		//if (serial_wait_s("AOK\r\n", BT_WAIT_DELAY) == 0) goto bt_fail;
	}
	else
	{
		// Break out of command mode
		bluetooth_puts("---");
		if (serial_wait_s("END\r\n", BT_WAIT_DELAY) == 0) return;
	}
}

#endif // HAS_BLUETOOTH