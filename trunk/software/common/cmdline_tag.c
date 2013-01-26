/******************************************************************************/
/* File    :	cmdline_tag.c						      */
/* Function:	Command line command 'tag'				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 1999-2010, Clockwork Engineering		      */
/* History :	31 Mar 2012 by R. Delien:				      */
/*		- First creation					      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "hardware.h"			/* Flexible hardware configuration */

#include "cmdline.h"
#include "cr14.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define MAX_RETRIES	10


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

/* Helpers */
static int hex2val(char *str, uint32_t *val);

/* Primitive operations */
static int tag_init(void);
static void tag_print_uid(void);
static void tag_print_block(char block);
static void tag_write_block(char block, unsigned char *data);
static void tag_term(void);

/* Sub commands */
static void tag_id(void);
static int tag_block_read(int argc, char* argv[]);
static int tag_block_write(int argc, char* argv[]);
static void tag_dump(void);


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

int tag(int argc, char* argv[])
{
	int	result = ERR_OK;

	if (argc < 2)
		return ERR_SYNTAX;

	if (!strncmp (argv[1], "uid", LINEBUFFER_MAX)) {
		if (argc != 2)
			return ERR_SYNTAX;
		tag_id();
	} else if (!strncmp (argv[1], "read", LINEBUFFER_MAX)) {
		if (argc != 3)
			return ERR_SYNTAX;
		result = tag_block_read(argc, argv);
	} else if (!strncmp (argv[1], "write", LINEBUFFER_MAX)) {
		if (argc != 3)
			return ERR_SYNTAX;
		result = tag_block_write(argc, argv);
	} else if (!strncmp (argv[1], "dump", LINEBUFFER_MAX)) {
		if (argc != 2)
			return ERR_SYNTAX;
		tag_dump();
	} else
		return ERR_SYNTAX;

	return result;
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static int tag_init(void)
{
	unsigned char	frame[4];
	unsigned char	length;
	unsigned char	retries;
	unsigned char	id;

	/* Turn on the carrier */
	if (cr14_writeparamreg(0x10)) {
		printf("NAck from reader\n");
		return (1);
	}

	/* Send an Init frame */
	frame[0] = 0x06;
	frame[1] = 0x00;
	if (cr14_writeframe(frame, 2)) {
		printf("Unable to send init\n");
		return (-1);
	}

	/* Read the assigned node ID */
	retries = 0;
	length = sizeof(frame);
	while (cr14_readframe(frame, &length)) {
		retries++;
		if (retries >= MAX_RETRIES) {
			printf("Unable to get ID\n");
			return (-1);
		}
		__delay_us(100);
	}
	if (length != 1) {
		printf("No tag detected\n");
		return (-1);
	}
	id = frame[0];

	/* Select the node ID */
	frame[0] = 0x0E;
	frame[1] = id;
	if (cr14_writeframe(frame, 2)) {
		printf("Unable to select ID\n");
		return (-1);
	}

	/* Read the response */
	retries = 0;
	length = sizeof(frame);
	while (cr14_readframe(frame, &length)) {
		retries++;
		if (retries >= MAX_RETRIES) {
			printf("Unable to read ID response\n");
			return (-1);
		}
		__delay_us(100);
	}
	if ((length != 1) || (frame[0] != id)) {
		printf("Tag interference\n");
		return (-1);
	}
	
	return (0);
}


static void tag_print_uid(void)
{
	unsigned char	frame[8];
	unsigned char	length;
	unsigned char	retries;

	/* Request the unique ID */
	frame[0] = 0x0B;	/* Command 0x0B */
	if (cr14_writeframe(frame, 1)) {
		printf("Unable to request UID\n");
		return;
	}

	retries = 0;
	length = 8;
	while (cr14_readframe(frame, &length)) {
		retries++;
		if (retries >= MAX_RETRIES) {
			printf("Unable to read UID\n");
			return;
		}
		__delay_us(100);
	}
	if (length != 8) {
		printf("Invalid UID\n");
		return;
	}

	printf("UniqueID:");
	while (length) {
		length--;
		printf(" %.2X", frame[length]);
	}
	printf("\n");

	return;
}


static void tag_print_block(char block)
{
	unsigned char	frame[4];
	unsigned char	length;
	unsigned char	retries;

	/* Request a block */
	frame[0] = 0x08;	/* Command 0x08 */
	frame[1] = block;
	if (cr14_writeframe(frame, 2)) {
		printf("Unable to request block\n");
		return;
	}

	retries = 0;
	length = sizeof(frame);
	while (cr14_readframe(frame, &length)) {
		retries++;
		if (retries >= MAX_RETRIES) {
			printf("Unable to read block\n");
			return;
		}
		__delay_us(100);
	}
	if (length != 4) {
		/* No error if no data */
		if (length)
			printf("Invalid block\n");
		return;
	}

	printf("Block %.2X:", block);
	while (length) {
		length--;
		printf(" %.2X", frame[length]);
	}
	printf("\n");

	return;
}


static void tag_write_block(char block, unsigned char *data)
{
	unsigned char	frame[6];

	/* Request the unique ID */
	frame[0] = 0x09;	/* Command 0x09 */
	frame[1] = block;
	frame[2] = data[0];
	frame[3] = data[1];
	frame[4] = data[2];
	frame[5] = data[3];
	if (cr14_writeframe(frame, sizeof(frame))) {
		printf("Unable to write block\n");
		return;
	}
	__delay_us(10000);

	return;
}


static void tag_term(void)
{
	(void)cr14_writeparamreg(0x00);
}


static void tag_id(void)
{
	int		result;

	result = tag_init();
	if (result > 0)
		return;

	if (!result)
		tag_print_uid();

	if (result <= 0)
		tag_term();
}


static int hex2val(char *str, uint32_t *val)
{
	unsigned char	index;
	unsigned int	*value = (unsigned int*)val;

	*val = 0;
	for (index = 0; index < strlen(str); index ++) {
		value[1] <<= 4;
		value[1] |= value[0] >> 12;
		value[0] <<= 4;
		if ( (str[index] >= '0') && (str[index] <= '9') )
			value[0] |= str[index] - '0';
		else if ( (str[index] >= 'A') && (str[index] <= 'F') )
			value[0] |= 10 + str[index] - 'A';
		else if ( (str[index] >= 'a') && (str[index] <= 'f') )
			value[0] |= 10 + str[index] - 'a';
		else
			return (-1);
	}

	return (0);
}


static int tag_block_read(int argc, char* argv[])
{
	int		result;
	uint32_t	block;

	if (hex2val(argv[2], &block))
		return ERR_SYNTAX;

	if (block > 0xFF)
		return ERR_SYNTAX;

	result = tag_init();
	if (result > 0)
		return ERR_IO;

	if (!result)
		tag_print_block(block);

	if (result <= 0)
		tag_term();

	return ERR_OK;
}


static int tag_block_write(int argc, char* argv[])
{
	int		result;
	uint32_t	temp;
	unsigned char	block;

	/* Retrieve the block number */
	if (hex2val(argv[2], &temp))
		return ERR_SYNTAX;

	if (temp > 0xFF)
		return ERR_SYNTAX;
	block = temp;

	/* Retrieve the data */
	if (hex2val(argv[3], &temp))
		return ERR_SYNTAX;

	result = tag_init();
	if (result > 0)
		return ERR_IO;

	if (!result)
		tag_write_block(block, (unsigned char *)&temp);

	if (!result)
		tag_print_block(block);

	if (result <= 0)
		tag_term();

	return ERR_OK;
}


static void tag_dump(void)
{
	int	result;
	int	block;

	result = tag_init();
	if (result > 0)
		return;

	if (!result) {
		tag_print_uid();
		for (block = 0; block < 0x100; block++)
			tag_print_block(block);
	}

	if (result <= 0)
		tag_term();
}
