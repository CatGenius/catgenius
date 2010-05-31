/******************************************************************************/
/* File    :	romwashprogram.h					      */
/* Function:	Header file of 'std_washprogram.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef ROMWASHPROGRAM_H			/* Include file already compiled? */
#define ROMWASHPROGRAM_H


/* Control */
void		       romwashprogram_reqcmd		(struct command const *       address) ;
unsigned char	       romwashprogram_getcmd		(struct command       * const command) ;


#endif /* ROMWASHPROGRAM_H */
