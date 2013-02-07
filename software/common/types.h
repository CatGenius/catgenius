// Copyright (c) 2013 Christopher Mapes <apresence@hotmail.com>
// Feel free to use, re-use, modify and redistribute as you see fit.  Please give credit, however

#ifndef TYPES_H
#define TYPES_H

/* ANSI-standard integer types */

#include <stddef.h>	// For size_t

#ifdef WIN32

#ifndef true
#define true (1)
#endif

#ifndef false
#define false (0)
#endif

#define int8_t		signed char
#define int16_t		signed short
#define int32_t		signed int
#define uint8_t		unsigned char
#define uint16_t	unsigned short
#define uint32_t	unsigned int
#define BOOL		unsigned char

#else // !WIN32

#include <stdbool.h>
#include <stdint.h>
#define BOOL		bit

#endif // WIN32

#define _CONSOLE_DEBUG WIN32

// Cleaner versions with uniformed width for easier readability
#define _I08 int8_t
#define _I16 int16_t
#define _I32 int32_t
#define _U08 uint8_t
#define _U16 uint16_t
#define _U32 uint32_t

//#define BITSIZE_ENABLED
#if BITSIZE_ENABLED
#define BITSIZE(n) : n
#else
#define BITSIZE(n)
#endif

#endif