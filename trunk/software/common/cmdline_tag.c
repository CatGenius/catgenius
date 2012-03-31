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

/* Primitive operations */
static int tag_init(void);
static void tag_print_uid(void);
static void tag_print_block(char block);
static void tag_term(void);

/* Sub commands */
static void tag_id(void);
static void tag_dump(void);


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

int tag(char *args)
{
	int	result = 0;
	char	*cmd = args;
	char	*arg = args;

	/* Find the separating space */
	while (*arg != ' ' && *arg != 0)
		arg++;

	/* Replace space(s) with 0-termination for command */
	while (*arg == ' ') {
		*arg = 0;
		arg++;
	}

	if (strlen(args)) {
		if (!strncmp (cmd, "uid", LINEBUFFER_MAX))
			tag_id();
		else if (!strncmp (cmd, "dump", LINEBUFFER_MAX))
			tag_dump();
		else
			/* Syntax error */
			result = -1;
	} else
		/* Syntax error */
		result = -1;

	return (result);
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

	/* Request the unique ID */
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
