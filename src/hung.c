/* GNU Chess 5.0 - hung.c - hung piece evaluation code
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
 *  Hung piece evaluation.
 */

#include <stdio.h>
#include "common.h"

short EvalHung(short side)
/****************************************************************************
 *
 *  Calculate the number of hung pieces for a side.
 *
 ****************************************************************************/
{
   BitBoard c;
   short xside;

   xside = 1 ^ side;
   hunged[side] = 0;

   /* Knight */
   c = (Ataks[xside][pawn] & board.b[side][knight]);
   c |= (Ataks[xside][0] & board.b[side][knight] & ~Ataks[side][0]);
   if (c)
      hunged[side] += nbits (c);

   /* Bishop */
   c = (Ataks[xside][pawn] & board.b[side][bishop]);
   c |= (Ataks[xside][0] & board.b[side][bishop] & ~Ataks[side][0]);
   if (c)
      hunged[side] += nbits (c);

   /* Rook */
   c = Ataks[xside][pawn] | Ataks[xside][knight] | Ataks[xside][bishop];
   c = (c & board.b[side][rook]);
   c |= (Ataks[xside][0] & board.b[side][rook] & ~Ataks[side][0]);
   if (c)
      hunged[side] += nbits (c);

   /* Queen */
   c = Ataks[xside][pawn] | Ataks[xside][knight] | Ataks[xside][bishop] |
       Ataks[xside][rook];
   c = (c & board.b[side][queen]);
   c |= (Ataks[xside][0] & board.b[side][queen] & ~Ataks[side][0]);
   if (c)
      hunged[side] += nbits (c);

   /* King */
   if (Ataks[xside][0] & board.b[side][king])
      hunged[side] ++;

   return (hunged[side]);
}
