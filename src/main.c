/* GNU Chess 5.0 - main.c - entry point
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
#include <stdlib.h>
#include "common.h"
#include "book.h"
#include <time.h>
#ifdef UNIVERSAL
#include "univ.h"
#include <signal.h>
#endif

short distance[64][64];
short taxicab[64][64];
unsigned char lzArray[65536];
BitBoard DistMap[64][8];
BitBoard BitPosArray[64];
BitBoard NotBitPosArray[64];
BitBoard MoveArray[8][64];
BitBoard Ray[64][8];
BitBoard FromToRay[64][64];
BitBoard RankBit[8];
BitBoard FileBit[8];
BitBoard Ataks[2][7];
BitBoard PassedPawnMask[2][64];
BitBoard IsolaniMask[8];
BitBoard SquarePawnMask[2][64];
BitBoard Rook00Atak[64][256];
BitBoard Rook90Atak[64][256];
BitBoard Bishop45Atak[64][256];
BitBoard Bishop315Atak[64][256];
BitBoard pinned;
BitBoard rings[4];
BitBoard stonewall[2];
BitBoard pieces[2];
BitBoard mask_kr_trapped_w[3];
BitBoard mask_kr_trapped_b[3];
BitBoard mask_qr_trapped_w[3];
BitBoard mask_qr_trapped_b[3];
BitBoard boardhalf[2];
BitBoard boardside[2];
short directions[64][64];
unsigned char BitCount[65536];
leaf Tree[MAXTREEDEPTH];
leaf *TreePtr[MAXPLYDEPTH];
int RootPV;
GameRec Game[MAXGAMEDEPTH];
short GameCnt;
short computer;
unsigned int flags;
short cboard[64];
short Mvboard[64];
Board board;
HashType hashcode[2][7][64];
HashType ephash[64];
HashType WKCastlehash;
HashType WQCastlehash;
HashType BKCastlehash;
HashType BQCastlehash;
HashType Sidehash;
HashType HashKey;
HashType PawnHashKey;
HashSlot *HashTab[2];
PawnSlot *PawnTab[2];
short Idepth;
short SxDec;
short Game50;
int lazyscore[2];
int maxposnscore[2];
int rootscore;
int lastrootscore;
unsigned long GenCnt;
unsigned long NodeCnt;
unsigned long QuiesCnt;
unsigned long EvalCnt;
unsigned long EvalCall;
unsigned long ChkExtCnt;
unsigned long OneRepCnt;
unsigned long RcpExtCnt;
unsigned long PawnExtCnt;
unsigned long HorzExtCnt;
unsigned long ThrtExtCnt;
unsigned long KingExtCnt;
unsigned long NullCutCnt;
unsigned long FutlCutCnt;
unsigned long RazrCutCnt;
unsigned long TotalGetHashCnt;
unsigned long GoodGetHashCnt;
unsigned long TotalPutHashCnt;
unsigned long CollHashCnt;
unsigned long TotalPawnHashCnt;
unsigned long GoodPawnHashCnt;
unsigned long RepeatCnt;
unsigned HashSize;
unsigned long TTHashMask;
unsigned long PHashMask;
char SANmv[10];
unsigned long history[2][4096];
int killer1[MAXPLYDEPTH];
int killer2[MAXPLYDEPTH];
short ChkCnt[MAXPLYDEPTH];
short ThrtCnt[MAXPLYDEPTH];
char id[32];
char solution[64];
double et;
float SearchTime;
short SearchDepth;
short MoveLimit[2];
float TimeLimit[2];
short TCMove;
short TCinc;
float TCTime;
short castled[2];
short hunged[2];
short phase;
int Hashmv[MAXPLYDEPTH];
short DebugPly;
short DebugDepth;
long DebugNode;
int Debugmv[MAXPLYDEPTH];
short Debugmvl;
short Debugline;
short RootPieces;
short RootPawns;
short RootMaterial;
short RootAlpha;
short RootBeta;
short pickphase[MAXPLYDEPTH];
short InChk[MAXPLYDEPTH];
short KingThrt[2][MAXPLYDEPTH];
short threatmv;
short threatply;
short KingSafety[2];
short bookmode;
short bookfirstlast;
FILE *ofp;
short myrating, opprating, suddendeath, TCionc;
char name[50];
short computerplays;		/* Side computer is playing */
short wasbookmove;		/* True if last move was book move */
int nmovesfrombook;		/* Number of moves since last book move */
int newpos, existpos;		/* For book statistics */
float maxtime;		/* Max time for the next searched move */
short n;		/* Last mobility returned by CTL */
short ExchCnt[2];	/* How many exchanges? */
short bookloaded = 0;  	/* Is the book loaded already into memory? */

short slider[8] = { 0, 0, 0, 1, 1, 1, 0, 0 };
short Value[7] = { 0, ValueP, ValueN, ValueB, ValueR, ValueQ, ValueK};
short range[8] = { 0, 0, 0, 1, 1, 1, 0, 0 };
short ptype[2] = { pawn, bpawn };
char algbr[64][3] =
{ "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8" 
};
char algbrfile[9] = "abcdefgh";
char algbrrank[9] = "12345678";
  
char notation[8] = { " PNBRQK" };
char lnotation[8] = { " pnbrqk" };

short Shift00[64] =
{ 56, 56, 56, 56, 56, 56, 56, 56,
  48, 48, 48, 48, 48, 48, 48, 48,
  40, 40, 40, 40, 40, 40, 40, 40,
  32, 32, 32, 32, 32, 32, 32, 32,
  24, 24, 24, 24, 24, 24, 24, 24,
  16, 16, 16, 16, 16, 16, 16, 16,
   8,  8,  8,  8,  8,  8,  8,  8,
   0,  0,  0,  0,  0,  0,  0,  0
};

short r90[64] =
{ A8, A7, A6, A5, A4, A3, A2, A1,
  B8, B7, B6, B5, B4, B3, B2, B1,
  C8, C7, C6, C5, C4, C3, C2, C1,
  D8, D7, D6, D5, D4, D3, D2, D1,
  E8, E7, E6, E5, E4, E3, E2, E1,
  F8, F7, F6, F5, F4, F3, F2, F1,
  G8, G7, G6, G5, G4, G3, G2, G1,
  H8, H7, H6, H5, H4, H3, H2, H1 };

short Shift90[64] =
{ 0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56,
  0, 8, 16, 24, 32, 40, 48, 56
};

short r45[64] =
{ E4, F3, H2, C2, G1, D1, B1, A1,
  E5, F4, G3, A3, D2, H1, E1, C1,
  D6, F5, G4, H3, B3, E2, A2, F1, 
  B7, E6, G5, H4, A4, C3, F2, B2,
  G7, C7, F6, H5, A5, B4, D3, G2, 
  C8, H7, D7, G6, A6, B5, C4, E3, 
  F8, D8, A8, E7, H6, B6, C5, D4, 
  H8, G8, E8, B8, F7, A7, C6, D5 };

short Shift45[64] =
{ 28, 36, 43, 49, 54, 58, 61, 63,
  21, 28, 36, 43, 49, 54, 58, 61,
  15, 21, 28, 36, 43, 49, 54, 58,
  10, 15, 21, 28, 36, 43, 49, 54,
   6, 10, 15, 21, 28, 36, 43, 49,
   3,  6, 10, 15, 21, 28, 36, 43,
   1,  3,  6, 10, 15, 21, 28, 36,
   0,  1,  3,  6, 10, 15, 21, 28 };

short Mask45[64] =
{ 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01,
  0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 
  0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 
  0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 
  0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 
  0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 
  0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 
  0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

short r315[64] =
{ A1, C1, F1, B2, G2, E3, D4, D5,
  B1, E1, A2, F2, D3, C4, C5, C6,
  D1, H1, E2, C3, B4, B5, B6, A7,
  G1, D2, B3, A4, A5, A6, H6, F7,
  C2, A3, H3, H4, H5, G6, E7, B8,
  H2, G3, G4, G5, F6, D7, A8, E8,
  F3, F4, F5, E6, C7, H7, D8, G8,
  E4, E5, D6, B7, G7, C8, F8, H8 };

short Shift315[64] =
{ 63, 61, 58, 54, 49, 43, 36, 28,
  61, 58, 54, 49, 43, 36, 28, 21,
  58, 54, 49, 43, 36, 28, 21, 15,
  54, 49, 43, 36, 28, 21, 15, 10,
  49, 43, 36, 28, 21, 15, 10,  6,
  43, 36, 28, 21, 15, 10,  6,  3,
  36, 28, 21, 15, 10,  6,  3,  1,
  28, 21, 15, 10,  6,  3,  1,  0 };

short Mask315[64] =
{ 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF,
  0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F,
  0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F,
  0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F,
  0x1F, 0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F,
  0x3F, 0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07,
  0x7F, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03,
  0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

int main (int argc, char *argv[])
{
  int compilebook = 0;
  time_t now; 
  int i;

#ifdef UNIVERSAL
  char buf[INPUT_SIZE];
#endif

  /* Initialize random number generator */
  time(&now);
  srand((unsigned) now);
  
  /* initialize control flags */
  flags = 0ull;

  /* output for thinking */
  ofp = stdout;

  if (argc > 1) {
    for (i = 0; i < argc; i++) {
      if (strcmp(argv[i],"xboard") == 0) {
	SET (flags, XBOARD);
      } else if (strcmp(argv[i],"post") == 0) {
	SET (flags, POST);
      } else if (strcmp(argv[i],"compile") == 0)
	compilebook++;
    }
  }
  
  if (!(flags & XBOARD))
    SET (flags, POST);
  ShowVersion ();
  Initialize ();

  if (argc > 1) {
    for (i = 0; i < argc; i++) {
      if (strcmp(argv[i],"xboard") == 0) {
	SET (flags, XBOARD);
      } else if (strcmp(argv[i],"post") == 0) {
	SET (flags, POST);
      } 
    }
  }

  bookmode = BOOKPREFER;
  bookfirstlast = 3;

   while (!(flags & QUIT))
   {
      InputCmd (); 
      if ((flags & THINK) && !(flags & MANUAL) && !(flags & ENDED)) {
#ifdef UNIVERSAL
      if (flags & UNIV) {
	signal(SIGALRM,SIG_IGN);
      }
#endif
        
      if (!(flags & XBOARD)) printf("Thinking...\n");
      Iterate ();
#ifdef UNIVERSAL
      if (flags & UNIV /* && !(flags & SELF) */) {
	  if (Game[GameCnt].move & PROMOTION) {
	    sprintf(buf,"M%s-%s/%c\r\n",
		 algbr[FROMSQ(Game[GameCnt].move)],
		 algbr[TOSQ(Game[GameCnt].move)],
	  	 notation[PROMOTEPIECE(Game[GameCnt].move)]);
	    printf("Sent M%s-%s/%c to Universal\n",
		 algbr[FROMSQ(Game[GameCnt].move)],
		 algbr[TOSQ(Game[GameCnt].move)],
	  	 notation[PROMOTEPIECE(Game[GameCnt].move)]);
	  } else {
	    sprintf(buf,"M%s-%s\r\n",
		 algbr[FROMSQ(Game[GameCnt].move)],
		 algbr[TOSQ(Game[GameCnt].move)]);
	    printf("Sent M%s-%s to Universal\n",
		 algbr[FROMSQ(Game[GameCnt].move)],
		 algbr[TOSQ(Game[GameCnt].move)]);
	  }
  	  univ_write(buf);
          signal(SIGALRM, univ_check);
	  alarm(1);
      }
#endif
      }
   }

   /*  Some cleaning up  */
   free (HashTab[0]);
   free (HashTab[1]);
   free (PawnTab[0]);
   free (PawnTab[1]);
   return (0);
}
