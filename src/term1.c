/* GNU Chess 5.0 - term1.c - test COM port interface code
   Copyright (c) 1999 Free Software Foundation, Inc.

   GNU Chess is based on the two research programs 
   Cobalt by Chua Kong-Sian and Gazebo by Stuart Cracraft.

   GNU Chess is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GNU Chess is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Chess; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Contact Info: 
     bug-gnu-chess@gnu.org
     cracraft@ai.mit.edu, cracraft@stanfordalumni.org, cracraft@earthlink.net
*/
/*
	Demo-Term.

	This is a short example of how to use this library.
	The COM port settings are hard coded as defines
	but you don't have to do it this way.  You could read them
	in from a file, etc..

	This this is just a sample program, I didn't take the time
	to emulate any known terminals.  Everything is output through
	DOS, so if you've got ansi.sys loaded, most of the ansi/vt100
	codes should work, but your keyboard isn't going to be
	mapped correctly. (i.e. the arrow keys)

	You can add that yourself if you wish to use this thing
	for anything other than a confirmation that the routines work.

	Have fun,

	Sam
*/

#include <stdio.h>
#include <stdlib.h>
#include <pc.h>
#include <conio.h>
#include <unistd.h>
#include "svasync.h"


#define COM_PORT        COM1
#define BAUD_RATE       9600
#define COM_SETTINGS    (BITS_8 | STOP_1 | NO_PARITY)

int main(void) {
	unsigned char c;

	printf("Demo-Term.  Press F10 to Quit.\n\n");

	if(SVAsyncInit( COM_PORT)) {
		printf("Error initalizing COM Port.\n");
		return(1);
	}
	
	SVAsyncFifoInit();

	SVAsyncSet(BAUD_RATE, COM_SETTINGS);
	SVAsyncHand(DTR | RTS);

	for(;;) {
		/* If a character was recieved, send it to stdout */
		if( (c=SVAsyncIn()) != 0  ) {
			putchar(c);
		}
		/* You could set stdout to nonbuffered, or do a fflush
		   after every character is sent.  I chose to use stdout
		   instead of direct console i/o so it would support
		   ansi.sys */
		fflush(stdout);

		/* If a key has been pressed       */
		if(kbhit()) {
			c=getch();
			if(!c) {  /* Check for 'extended characters */
				c=getch();      /* Get the extended code */
				if( c==0x44)    /* Exit if it is a F10 */
					break;
			}
			else 
				SVAsyncOut(c);
		}



		/* If the buffer has a lot in it, drop RTS and empty the buffer */
		if( SVAsyncInStat()>4096) {
			SVAsyncHand(DTR);        /* Drop RTS */
			while(SVAsyncInStat()>0)
				putch(SVAsyncIn());
			SVAsyncHand(DTR | RTS);
		}
	}

	SVAsyncStop();
	return(0);
}

