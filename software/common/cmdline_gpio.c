/******************************************************************************/
/* File    :	cmdline_gpio.c						      */
/* Function:	Command line command for operating gpio			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 1999-2010, Clockwork Engineering		      */
/* History :	31 Mar 2012 by R. Delien:				      */
/*		- First creation					      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "cmdline.h"
#include "serial.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#ifdef _16F1939)
#define HAS_LATCH_REGS
#endif /* _16F1939 */


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

struct gpioport {
	char				name;		/* Name of the GPIO port, eg. A */
	uint8_t				pinmask;	/* Mask of valid pins for the GPIO port */
	uint8_t				cache;
	volatile uint8_t		*port;		/* Port input register */
#ifdef HAS_LATCH_REGS
	volatile uint8_t		*latch;		/* Port output latch register */
#endif /* HAS_LATCH_REGS */
	volatile uint8_t		*tris;		/* Port tristate register */	
};

#ifdef HAS_LATCH_REGS
#define PORT_REGISTERS(prt)		&PORT##prt, &LAT##prt, &TRIS##prt
#else
#define PORT_REGISTERS(prt)		&PORT##prt, &TRIS##prt
#endif /* HAS_LATCH_REGS */

struct gpioport	gpioports[] = {
	{'A',	0x7f,	0x00,	PORT_REGISTERS(A)},
	{'B',	0xff,	0x00,	PORT_REGISTERS(B)},
	{'C',	0xff,	0x00,	PORT_REGISTERS(C)},
	{'D',	0xff,	0x00,	PORT_REGISTERS(D)},
	{'E',	0xff,	0x00,	PORT_REGISTERS(E)},
	{NULL}	/* Sentinel */
};


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

void hexstr (uint8_t value, char *str);


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

int gpio(int argc, char* argv[])
{
	unsigned char	port = 0;
	unsigned char	pin  = 0;

	if ((argc > 3))
		return ERR_SYNTAX;

	if (argc == 3) {
		/* First argument: GPIO pin */
		if (strlen(argv[1]) != 2)
			return ERR_SYNTAX;
		while (gpioports[port].name) {
			if ((gpioports[port].name == argv[1][0]) ||
			    (gpioports[port].name-'A'+'a' == argv[1][0]))
				break;
			port++;
		}
		if (!gpioports[port].name)
			return ERR_PARAM;

		if (argv[1][1] < '0' && argv[1][1] > '7')
			return ERR_PARAM;
		pin = argv[1][1] - '0';
		if (!((1 << pin) & gpioports[port].pinmask))
			return ERR_PARAM;

		/* Second argument: Operation */
		if (!strncmp (argv[2], "input", LINEBUFFER_MAX)) {
			*gpioports[port].tris |= 1 << pin;
			printf("Port %c, pin %d = %s\n", port + 'A', pin, (*gpioports[port].port & 1 << pin)?"1":"0");
		} else if (!strncmp (argv[2], "set", LINEBUFFER_MAX)) {
#ifdef HAS_LATCH_REGS
			*gpioports[port].latch |= 1 << pin;
#else
			*gpioports[port].port  |= 1 << pin;
#endif /* HAS_LATCH_REGS */
			*gpioports[port].tris  &= ~(1 << pin);
			printf("Port %c, pin %d set\n", port + 'A', pin);
		} else if (!strncmp (argv[2], "clr", LINEBUFFER_MAX)) {
#ifdef HAS_LATCH_REGS
			*gpioports[port].latch &= ~(1 << pin);
#else
			*gpioports[port].port  &= ~(1 << pin);
#endif /* HAS_LATCH_REGS */
			*gpioports[port].tris  &= ~(1 << pin);
			printf("Port %c, pin %d cleared\n", port + 'A', pin);
		}
	} else {
		if ((argc == 2) && strncmp(argv[1], "log", LINEBUFFER_MAX))
			return ERR_SYNTAX;

		/* First dump port registers and update cache */
		while (gpioports[port].port) {
			char	hex[3];

			gpioports[port].cache = *gpioports[port].port;
			/* Convert to hex ourselves" printf %x is broken! */
			hexstr(gpioports[port].cache, hex);
			printf("PORT%c = 0x%s, ", gpioports[port].name, hex);
#ifdef HAS_LATCH_REGS
			hexstr(*gpioports[port].latch, hex);
			printf("LAT%c = 0x%s, ", gpioports[port].name, hex);
#endif /* HAS_LATCH_REGS */
			hexstr(*gpioports[port].tris, hex);
			printf("TRIS%c = 0x%s\n", gpioports[port].name, hex);
			port++;
		}
		if (argc == 2) {
			printf("Press any key to exit logging\n");
			for (;;) {
				char	rxd;

				for (port = 0; gpioports[port].port; port++)
					if (gpioports[port].cache != *gpioports[port].port) {
						char	hex[3];

						gpioports[port].cache = *gpioports[port].port;
						/* Convert to hex ourselves" printf %x is broken! */
						hexstr(gpioports[port].cache, hex);
						printf("PORT%c = 0x%s\n", gpioports[port].name, hex);
					}
				if (readch(&rxd))
					break;
			}
		}
	}

	return ERR_OK;
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

void hexstr (uint8_t value, char *str)
{
	uint8_t	nibble;
	nibble = ((value & 0xf0) >> 4);
	str[0] = (nibble < 10)?nibble+'0':nibble-10+'A';
	nibble = value & 0x0f;
	str[1] = (nibble < 10)?nibble+'0':nibble-10+'A';
	str[2] = 0;
}
