/*
 * inlines.h - static inline version of performance-critical
 * functions. Is included by common.h unless NO_INLINE is
 * defined.
 */
/* GNU Chess 5.0 - inlines.h - static inline functions
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

#ifndef INLINES_H
#define INLINES_H

static inline short leadz (BitBoard b)
/**************************************************************************
 *
 *  Returns the leading bit in a bitboard.  Leftmost bit is 0 and
 *  rightmost bit is 1.  Thanks to Robert Hyatt for this algorithm.
 *
 ***************************************************************************/
{
   register union
   {
      BitBoard bitboard;
      unsigned short v[4];
   } a;

   a.bitboard = b;
#ifndef WORDS_BIGENDIAN
   if (a.v[3] != 0)
      return (lzArray[a.v[3]]);
   if (a.v[2] != 0)
      return (lzArray[a.v[2]] + 16);
   if (a.v[1] != 0)
      return (lzArray[a.v[1]] + 32);
   if (a.v[0] != 0)
      return (lzArray[a.v[0]] + 48);
#else
   if (a.v[0] != 0)
      return (lzArray[a.v[0]]);
   if (a.v[1] != 0)
      return (lzArray[a.v[1]] + 16);
   if (a.v[2] != 0)
      return (lzArray[a.v[2]] + 32);
   if (a.v[3] != 0)
      return (lzArray[a.v[3]] + 48);
#endif /* WORDS_BIGENDIAN */
   return (-1);
}


static inline short nbits (BitBoard b)
/***************************************************************************
 *
 *  Count the number of bits in b.
 *
 ***************************************************************************/
{
   register union
   {
      BitBoard bitboard;
      unsigned short v[4];
   } a;

   a.bitboard = b;
   return (BitCount[a.v[0]] + BitCount[a.v[1]] +
		BitCount[a.v[2]] + BitCount[a.v[3]]);

}

#endif /* !INLINES_H */
