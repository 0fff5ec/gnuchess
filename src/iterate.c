/* GNU Chess 5.0 - iterate.c - iterative deepening code
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "common.h"

#define WINDOW	75

static struct timeval t1, t2;
static struct timezone tz;
extern short stage;
short InChkDummy, terminal;

void Iterate (void)
/**************************************************************************
 *
 *  Begin the iterative deepening code.  The variable Idepth represents
 *  the iteration depth.
 *
 **************************************************************************/
{
   short side;
   int score, RootAlpha, RootBeta;

   side = board.side;

   /* Note computer is playing side we are making move for. */
   computerplays = board.side;

   lazyscore[white] = lazyscore[black] = 100; /* was 50 */
   maxposnscore[white] = maxposnscore[black] = 100;
   GenCnt = 0;
   NodeCnt = QuiesCnt = 0;
   EvalCnt = EvalCall = 0;
   OneRepCnt = ChkExtCnt = RcpExtCnt = PawnExtCnt = HorzExtCnt = ThrtExtCnt = 0;
   KingExtCnt = 0;
   NullCutCnt = FutlCutCnt = 0;
   TotalGetHashCnt = GoodGetHashCnt = 0;
   TotalPutHashCnt = CollHashCnt = 0;
   TotalPawnHashCnt = GoodPawnHashCnt = 0;
   RootPawns = nbits (board.b[white][pawn] | board.b[black][pawn]);
   RootPieces = nbits (board.friends[white] | board.friends[black]) -
		RootPawns;
   RootMaterial = MATERIAL;
   RepeatCnt = 0;
   et = 0.0;
   memset (ChkCnt, 0, sizeof (ChkCnt));
   memset (ThrtCnt, 0, sizeof (ThrtCnt));
   memset (history, 0, sizeof (history));
   memset (killer1, 0, sizeof (killer1));
   memset (killer2, 0, sizeof (killer2));
   TTClear (); 
   CLEAR (flags, TIMEOUT);
   if (flags & TIMECTL)
   {
      /* Time for a move is just total time remaining divided by moves left */
      /* mcriley - was no # */

      SearchTime = TimeLimit[side] * 1.25 / MoveLimit[side];
      /* Allocate 10% of total time as reserve */
      /* mcriley - was 9 * S /10 */
      SearchTime = 95 * SearchTime / 100;
      /* Reserve more if in sudden death */
      /* mcrikey was 8 * S / 10 */
      if (suddendeath) 
	SearchTime = 92 * SearchTime / 100;
/* NEW */
      /* Always target at least the increment seconds for the move! */
      /* To prevent huge surplus build-up. */
      if (TCinc != 0)
        if (SearchTime < TCinc) {
          printf("TimeLimit[%s] = %6.2f\n",TimeLimit[side]);
          if (TimeLimit[side] > 30) {   /* Only if > 15 seconds left */
            SearchTime = TCinc;
          }
        }
/* END NEW */
      ShowTime ();
   }
   Idepth = 0;
   TreePtr[2] = TreePtr[1];
   GenMoves (1);
   FilterIllegalMoves (1); 
   SortRoot ();
   gettimeofday (&t1, &tz);
   InChk[1] = SqAtakd (board.king[side], 1^side);

   /*  Are there any legal moves? */
   if (GenCnt == 0)
   {
     if (InChk[1]) {
	if (computerplays == black)
	  printf("1-0 {computer loses as black}\n");
	else if (computerplays == white)
	  printf("0-1 {computer loses as white}\n");
     } else 
	 printf("1/2-1/2 {stalemate}\n");
      fflush(stdout);
      SET (flags, TIMEOUT | ENDED);
      return;
   }
   else if (GenCnt == 1)
   {
      RootPV = TreePtr[1]->move;
      SET (flags, TIMEOUT);
   }
   lastrootscore = score = Evaluate (-INFINITY, INFINITY);
      
   wasbookmove = 0;
   if (bookmode != BOOKOFF && !(flags & SOLVE) && nmovesfrombook <= 3) {
     if (BookQuery()) {
       nmovesfrombook = 0;
       wasbookmove = 1;
       SET (flags, TIMEOUT);
     } else
       nmovesfrombook++;
   } else
     nmovesfrombook++;


/* mcriley - was 2 * searchtime */

   maxtime = 4 * SearchTime;

#ifdef HYATT
   /* Human-like: take more time after opening book, reducing into endgame */

   /* mcriley - was  maxtime = 2 * searchtime */

if (bookmode != BOOKOFF && (flags & TIMECTL)) {
     maxtime = 6 * SearchTime;
     /* Hyatt's grandmaster-like time control curve */
     if (nmovesfrombook <= 5 /* && GameCnt < 20 */) {
       SearchTime = (2 - (MIN(nmovesfrombook,5)/5)) * SearchTime;
       maxtime = SearchTime;
     }
   } 
#endif

   if (flags & POST) {
     printf ("Root = %d, ", score);
     printf ("Phase = %d ", phase);
   }
   if (ofp != stdout) {
     fprintf (ofp,"Root = %d\t", score);
     fprintf (ofp,"Phase = %d\t", phase);
   }
   if (SearchDepth == 0) {
      if (ofp != stdout) 
        fprintf (ofp,"\nTime = %.2f, Max = %.2f, Left = %.2f, Moves= %d\n", SearchTime,maxtime,TimeLimit[side],MoveLimit[side]);
      if (flags & POST) 
        printf ("\nTime = %.2f, Max = %.2f, Left = %.2f, Moves = %d\n", SearchTime,maxtime,TimeLimit[side],MoveLimit[side]);
   } else {
      if (ofp != stdout)
        fprintf (ofp,"Depth = %d\n", SearchDepth);
      if (flags & POST)
	printf ("Depth = %d\n", SearchDepth);
   }

   if (flags & POST) {
     printf("Ply   Time     Eval      Nodes   Principal-Variation\n");
     if (ofp != stdout)
       fprintf(ofp,"Ply   Time     Eval      Nodes   Principal-Variation\n");
   }
   while (!(flags & TIMEOUT)) 
   {
      threatply = 0;
      if (score > MATE-255)
      {
	 RootAlpha = score-1;
	 RootBeta = MATE;
      }
      else if (score < -MATE+255)
      {
	 RootAlpha = -MATE;
	 RootBeta  = score+1;
      }
      else
      {
	 RootAlpha = MAX (score - WINDOW, -MATE);
	 RootBeta  = MIN (score + WINDOW,  MATE);
      }
      Idepth += DEPTH;
      rootscore = -INFINITY-1;
      score = SearchRoot (Idepth, RootAlpha, RootBeta);
      if (score >= RootBeta && score < MATE && !(flags & TIMEOUT))
      {
         ShowLine (RootPV, score, '+');
         rootscore = -INFINITY-1;
         RootAlpha = RootBeta;
         RootBeta = INFINITY;
         score = SearchRoot (Idepth, RootAlpha, RootBeta);
         /*score = SearchRoot (Idepth, RootBeta, INFINITY);*/
      }
      else 
      /*  If we fail low, then research. */ 
      if (score <= RootAlpha /*&& !(flags & TIMEOUT)*/)
      {
         ShowLine (RootPV, score, '-');
         rootscore = -INFINITY-1;
	 RootBeta = RootAlpha;	
	 RootAlpha = -INFINITY;
         score = SearchRoot (Idepth, RootAlpha, RootBeta);
         /*score = SearchRoot (Idepth, -INFINITY, RootAlpha);*/
      }

      /*  Print the final PV line  */
      ShowLine (RootPV, score, '.');
      lastrootscore = score;


      /* See if we have time to start another iteration */
      /* mcriley - was 2 * S / 3 */
      if (SearchDepth == 0 && (flags & TIMECTL) && et >= 2 * SearchTime / 3)
         SET (flags, TIMEOUT);

/*
 *     if (MATESCORE (score))
 *         SET (flags, TIMEOUT);
 */
      if (abs(score) + Idepth >= MATE + 1) 
         SET (flags, TIMEOUT);

      if (Idepth == SearchDepth*DEPTH) 
         break; 
   }

/***************************************************************************
 *
 *  We've finish the search.  Do things like update the game history,
 *  time control stuff and print the search result.
 *  mcriley - MoveLimit was = 60
 ***************************************************************************/
   SANMove (RootPV, 1);
   strcpy (Game[GameCnt+1].SANmv, SANmv);
   Game[GameCnt+1].et = et;
   MakeMove (side, &RootPV);
   if (flags & TIMECTL)
   {
      if (suddendeath) {
	if (TimeLimit[side] > 0 && TimeLimit[side] <= 60)
	  MoveLimit[side] = 60;
	else
	  MoveLimit[side] = 35;
 	printf("MoveLimit is %d\n",MoveLimit[side]);
 	printf("TimeLimit is %f\n",TimeLimit[side]);
      } else
	MoveLimit[side]--;
      TimeLimit[side] -= (float) et;
      if (TCinc != 0)
	TimeLimit[side] += TCinc;
      if (MoveLimit[side] == 0)
      {
         MoveLimit[side] = TCMove;
      }
   }

   if (flags & XBOARD) 
   {
      printf ("%d. ... %s\n", GameCnt/2 + 1, AlgbrMove(RootPV));
      printf ("My move is: %s\n", AlgbrMove(RootPV));
      fflush(stdout);
      if (ofp != stdout) {
        fprintf (ofp,"%d. ... %s\n", GameCnt/2 + 1, AlgbrMove(RootPV));
        fprintf (ofp,"My move is: %s\n", AlgbrMove(RootPV));
        fflush(ofp);
      }
   }
   else
   {
      if (!wasbookmove) {
        fprintf (ofp,"\nTime = %.1f Rate=%ld Nodes=[%ld/%ld/%ld] GenCnt=%ld\n", 
	  et, et > 0 ? (unsigned long)((NodeCnt+QuiesCnt)/et) : 0, 
	  NodeCnt, QuiesCnt, NodeCnt+QuiesCnt, GenCnt);
        fprintf (ofp,"Eval=[%ld/%ld] RptCnt=%ld NullCut=%ld FutlCut=%ld\n",
          EvalCnt, EvalCall, RepeatCnt, NullCutCnt, FutlCutCnt);
        fprintf (ofp,"Ext: Chk=%ld Recap=%ld Pawn=%ld OneRep=%ld Horz=%ld Mate=%ld KThrt=%ld\n",
          ChkExtCnt, RcpExtCnt, PawnExtCnt, OneRepCnt, HorzExtCnt, ThrtExtCnt,
	  KingExtCnt);
        fprintf (ofp,"Material=[%d/%d : %d/%d] ", board.pmaterial[white], board.pmaterial[black], board.material[white], board.material[black]);
      fprintf (ofp,"Lazy=[%d/%d] ", lazyscore[white], lazyscore[black]);
      fprintf (ofp,"MaxPosnScore=[%d/%d]\n",maxposnscore[white],maxposnscore[black]);
        fprintf (ofp,"Hash: Success=%ld%% Collision=%ld%% Pawn=%ld%%\n",
           GoodGetHashCnt*100/(TotalGetHashCnt+1),
           CollHashCnt*100/(TotalPutHashCnt+1),
           GoodPawnHashCnt*100/(TotalPawnHashCnt+1));
      }
/*
      if (flags & TIMECTL)
           fprintf (ofp,"Time control: %d moves in %.2f secs\n", 
		  MoveLimit[side], TimeLimit[side]);
*/
      if (!(flags & SOLVE)) ShowBoard ();
      printf ("\nMy move is : %s\n", SANmv);
      fflush(stdout);
      if (ofp != stdout) {
        fprintf (ofp,"\nMy move is : %s\n", SANmv);
        fflush(ofp);
      }
   }

/***************************************************************************
 *
 *  Before we leave, check to see if this is mate or stalemate.
 *
 ***************************************************************************/
   TreePtr[2] = TreePtr[1];
   GenMoves (1);
   FilterIllegalMoves (1); 
   if (TreePtr[1] == TreePtr[2])
   {
     if (SqAtakd (board.king[board.side], 1^board.side)) {
       if (computerplays == black)
	 printf("0-1 {computer wins as black}\n");
       else
	 printf("1-0 {computer wins as white}\n");
     } else
       printf("1/2-1/2 {stalemate}\n");
     fflush(stdout);
     SET (flags, ENDED);
   }
   if (EvaluateDraw () || Repeat() >= 2)
   {
      printf("1/2-1/2 {draw}\n");
      fflush(stdout); 
      SET (flags, ENDED);
   }
   return;
} 


void GetElapsed (void)
{
   gettimeofday (&t2, &tz);
   et = (t2.tv_sec - t1.tv_sec) + (double) (t2.tv_usec - t1.tv_usec)/1000000.0;
}

