/******************************************************************************/
/* File    :    bluetooth.c                                                   */
/* Function:    Contains bt_init() routine to initialize the Bluetooth module */
/*              Supported module is the Roving Networks Bluetooth Mate Silver */
/*              Product Pages:                                                */
/*                http://www.rovingnetworks.com/products/RN_42                */
/*                http://www.sparkfun.com/products/10393                      */
/* Author  :    Christopher Mapes                                             */
/******************************************************************************/

#include <htc.h>
#include <stdio.h>
#include <string.h>

#include "hardware.h"			/* Flexible hardware configuration */
#include "serial.h"
#include "bluetooth.h"

// Wait timeout for response from device
#define BT_WAIT_DELAY	500UL

// Macro for integer division with proper round-off (BEWARE OF OVERFLOW!)
#define INTDIV(t,n)		((2*(t)+(n))/(2*(n)))		/* Macro for integer division with proper round-off (BEWARE OF OVERFLOW!) */

static unsigned char bt_state = 0;

void bluetooth_init(void)
{
	char s_bitrate[5];
	
	// TBD:
    // - Could write EEPROM when we've done this so we don't try again
    // - Or at least check nBOR and/or nPOR

	// Sanity check - serial_init() needs to be called first (It sets SPEN)
	if (!SPEN) goto bt_fail;
	bt_state++;

	// Convert bitrate to string format accepted by BT module
	if (BT_BITRATE <= 9600UL)
	{
		sprintf(s_bitrate, "%lu", BT_BITRATE);
	}
	else if (BT_BITRATE < 115200UL)
	{
		sprintf(s_bitrate, "%02.1f", ((float)BT_BITRATE / 1000UL));
	}
	else
	{
		sprintf(s_bitrate, "%luK", BT_BITRATE / 1000UL);
	}

	// TBD: During init phase, we don't want any XON/XOFF control!

	//DBG("=== Programming Bluetooth Module ===\n");

#ifdef USE_BT_NAME
	// Put BT module in command mode.  It's expecting to see 3 "$" chars in a 1 sec window with silence
	__delay_ms(1000);
	printf("$$$");
	if (serial_wait_s("CMD\r\n", BT_WAIT_DELAY) == 0) goto bt_fail;
	bt_state++;

	printf("S-,CatGenius\r");
	if (serial_wait_s("AOK\r\n", BT_WAIT_DELAY) == 0) goto bt_fail;
	bt_state++;

    printf("R,1\r");
	if (serial_wait_s("Reboot!\r\n", BT_WAIT_DELAY) == 0) goto bt_fail;
	bt_state++;
#endif

	// Program baud rate

	// Put BT module in command mode.  It's expecting to see exactly 3 "$" chars in a 1 sec window with no other characters
	__delay_ms(1000);
	printf("$$$");
	if (serial_wait_s("CMD\r\n", BT_WAIT_DELAY) == 0) goto bt_fail;
	bt_state++;

	printf("U,%s,N\r", s_bitrate);
	if (serial_wait_s("AOK\r\n", BT_WAIT_DELAY) == 0) goto bt_fail;

	// NOTE: XON/XOFF flow control has to be enabled on the remote end; the BT module will pass the control characters through

	bt_state = 255;

bt_fail:
	return;
}