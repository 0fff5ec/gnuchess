/* GNU Chess 5.0 - util.c - utility routine code
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
 *
 */


#include <stdio.h>
#include <string.h>
#include "common.h"
#include <signal.h>

inline short leadz (BitBoard b)
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
#ifdef i386
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
#endif
   return (-1);
}


inline short nbits (BitBoard b)
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


inline void UpdateFriends ()
/***************************************************************************
 *
 *  Update friend and enemy bitboard.
 *
 ***************************************************************************/
{
   register BitBoard *w, *b;

   w = board.b[white];
   b = board.b[black];
   board.friends[white] = 
      w[pawn] | w[knight] | w[bishop] | w[rook] | w[queen] | w[king];
   board.friends[black] = 
      b[pawn] | b[knight] | b[bishop] | b[rook] | b[queen] | b[king];
   board.blocker = board.friends[white] | board.friends[black];
}
   

void UpdateCBoard ()
/**************************************************************************
 *
 *  Updates cboard[].  cboard[i] returns the piece on square i.
 *
 **************************************************************************/
{
   BitBoard b;
   short piece, sq;

   memset (cboard, 0, sizeof (cboard));
   for (piece = pawn; piece <= king; piece++)
   {
      b = board.b[white][piece] | board.b[black][piece];
      while (b)
      {
         sq = leadz (b);
         CLEARBIT (b, sq);
         cboard[sq] = piece;
      }
   }
}


static const short OrigCboard[64] = 
{ rook,  knight, bishop, queen, king,  bishop, knight, rook,
  pawn,  pawn,   pawn,   pawn,  pawn,  pawn,   pawn,   pawn,
  empty, empty,  empty,  empty, empty, empty,  empty,  empty,
  empty, empty,  empty,  empty, empty, empty,  empty,  empty,
  empty, empty,  empty,  empty, empty, empty,  empty,  empty,
  empty, empty,  empty,  empty, empty, empty,  empty,  empty,
  pawn,  pawn,   pawn,   pawn,  pawn,  pawn,   pawn,   pawn,
  rook,  knight, bishop, queen, king,  bishop, knight, rook };

void UpdateMvboard ()
/**************************************************************************
 *
 *  Updates Mvboard[].  Mvboard[i] returns the number of times the piece
 *  on square i have moved.  When loading from EPD, if a piece is not on
 *  its original square, set it to 1, otherwise set to 0.
 *
 **************************************************************************/
{
   short sq;
 
   for (sq = 0; sq < 64; sq++)
   {
      if (cboard[sq] == empty || cboard[sq] == OrigCboard[sq])
         Mvboard[sq] = 0;
      else
         Mvboard[sq] = 1;
   } 
}


void EndSearch (int sig)
/***************************************************************************
 *
 *  User has pressed Ctrl-C.  Just set flags TIMEOUT to be true.
 *
 ***************************************************************************/
{
   SET (flags, TIMEOUT);
   signal (SIGINT, EndSearch);
}


short ValidateBoard ()
/***************************************************************************
 *
 *  Check the board to make sure that its valid.  Some things to check are
 *  a.  Both sides have only 1 king.
 *  b.  Side not on the move must not be in check.
 *  c.  If en passant square is set, check it is possible.
 *  d.  Check if castling status are all correct.
 *
 ***************************************************************************/
{
   short side, xside, sq;

   if (nbits (board.b[white][king]) != 1) 
      return (false);
   if (nbits (board.b[black][king]) != 1) 
      return (false);

   side = board.side;  
   xside = 1^side;
   if (SqAtakd (board.king[xside], side))
      return (false);

   if (board.ep > -1)
   {
      sq = board.ep + (xside == white ? 8 : -8);
      if (!(BitPosArray[sq] & board.b[xside][pawn]))
	 return (false);
   }

   if (board.flag & WKINGCASTLE)
   {
      if (!(BitPosArray[E1] & board.b[white][king]))
	 return (false);
      if (!(BitPosArray[H1] & board.b[white][rook]))
	 return (false);
   }
   if (board.flag & WQUEENCASTLE)
   {
      if (!(BitPosArray[E1] & board.b[white][king]))
	 return (false);
      if (!(BitPosArray[A1] & board.b[white][rook]))
	 return (false);
   }
   if (board.flag & BKINGCASTLE)
   {
      if (!(BitPosArray[E8] & board.b[black][king]))
	 return (false);
      if (!(BitPosArray[H8] & board.b[black][rook]))
	 return (false);
   }
   if (board.flag & BQUEENCASTLE)
   {
      if (!(BitPosArray[E8] & board.b[black][king]))
	 return (false);
      if (!(BitPosArray[A8] & board.b[black][rook]))
	 return (false);
   }

   return (true);
}
