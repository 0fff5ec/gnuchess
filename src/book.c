/* GNU Chess 5.0 - book.c - book code
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
#include "common.h"
#include "book.h"
#include <unistd.h>

#define vcol(c) (c>='a' && c<='h')
#define vpiece(p) (p=='N'||p=='B'||p=='R'||p=='Q'||p=='K')

#define MAXMOVES 200
#define MAXLINE 512
#define MAXVAR 10
#define MAXBOOK 900000
#define MAXMATCH 100
#define MAXPEEK 10
			   
static int bookcnt, bigbookcnt = 0, variations = 0, bigvariations = 0;
static int lastbookcnt;
static int runningbookcnt = 0;
static char linebuf[MAXVAR][MAXLINE];  
static char smallbuf[MAXLINE/2];
HashType posshash[MAXMOVES];
struct hashtype {
  short wins;
  short losses;
  short draws;
  HashType key;
} bookpos[MAXBOOK];

int compare(leaf *, leaf *);
int compare(a, b)
leaf *a;
leaf *b;
{
    if (b->score > a->score) return(1);
    else if (b->score < a->score) return(-1);
    else return(0);
}

/*
 * Store the position and results of last search
 * if and only if in the first 10 moves of the game.
 * This routine is called after the computer makes its
 * most recent move. Lastly, after the 10th move, on 
 * the 11th move, store the data out to the running file.
 */
void BookBuilder(short depth, int score, short result, short side)
{
  FILE *wfp,*rfp;
  register i;
  int targetslot, found = 0, storeit = 0, tot;
  if (depth == -1 && score == -1) {
    if ((rfp = fopen(BOOKRUN,"r+b")) != NULL) {
      printf("Opened existing book!\n");
    } else {
      printf("Created new book!\n");
      wfp = fopen(BOOKRUN,"w+b");
      fclose(wfp);
      if ((rfp = fopen(BOOKRUN,"r+b")) == NULL) {
        printf("Could not create %s file\n",BOOKRUN);
	return;
      }
    }
    fread(&bookpos,sizeof(struct hashtype),runningbookcnt,wfp);
    runningbookcnt = fread(&bookpos,sizeof(struct hashtype),MAXBOOK,rfp);
    fclose(rfp);
    printf("Read %d book positions\n",runningbookcnt);
    return;
  }
  /* Only first BOOKDEPTH moves */
  if (GameCnt/2+1 < BOOKDEPTH) {
    CalcHashKey();
    for(i = 0; i < runningbookcnt; i++) {
      /* If a match occurs, it means we've found the current game
	 position in the running positions book */
      if (HashKey == bookpos[i].key) {
	found = 1;
        break;
      }
    }
    /* Now store score and depth if depth is greater */
    if (found) {
      targetslot = i;
      storeit = 1;
      existpos++;
    } else {
      targetslot = runningbookcnt;
      storeit = 1;
      newpos++;
    }
    bookpos[i].key = HashKey;
    if (side == white) {
      if (result == R_WHITE_WINS)
	  bookpos[i].wins++;
      else if (result == R_BLACK_WINS)
	  bookpos[i].losses++;
      else if (result == R_DRAW)
	  bookpos[i].draws++;
    } else {
      if (result == R_WHITE_WINS)
	  bookpos[i].losses++;
      else if (result == R_BLACK_WINS)
	  bookpos[i].wins++;
      else if (result == R_DRAW)
	  bookpos[i].draws++;
    }
    if (!found) runningbookcnt = i+1;
  }
  if ( (depth != 0 && score != 0) || (depth == -2 && score == -2)) {
    wfp = fopen(BOOKRUNT,"w+b");
    fwrite(&bookpos,sizeof(struct hashtype),runningbookcnt,wfp);
    fclose(wfp);
    rename(BOOKRUNT,BOOKRUN);
  }
}

int BookQuery()
{
  int i,j,k,icnt = 0, mcnt, found, tot, maxdistribution;
  int matches[MAXMATCH], best = 0;
  short bestsofar;
  leaf m[MAXMOVES];
  leaf pref[MAXMOVES];
  struct {
    short wins;
    short losses;
    short draws;
  } r[MAXMOVES];
  FILE *rfp;
  leaf *p;
  short side,xside,temp;

  bestsofar = 0;
  mcnt = -1;
  side = board.side;
  xside = 1^side;
  rfp = fopen(BOOKRUN,"r+b");
  if (rfp == NULL) {
    if (!(flags & XBOARD))
      fprintf(ofp," no book (%s).\n",BOOKRUN);
    return(0);
  }
  TreePtr[2] = TreePtr[1];
  GenMoves(1);
  FilterIllegalMoves (1);
  for (p = TreePtr[1]; p < TreePtr[2]; p++) {
    MakeMove(side, &p->move);
    m[icnt].move = p->move;
    posshash[icnt] = HashKey;
    icnt++;
    UnmakeMove(xside,&p->move);
  }
  if (!bookloaded) {
    bigbookcnt = 0;
    if (!(flags & XBOARD))
      fprintf(ofp,"Read opening book (%s)... ",BOOKRUN);
    fseek(rfp,0L,SEEK_SET);
    bookcnt = fread(&bookpos,sizeof(struct hashtype),MAXBOOK,rfp);
    fclose(rfp); 
    bigbookcnt += bookcnt;
    bookloaded = 1;
  }
  for (j = 0; j < bookcnt; j++) {
    for (i = 0; i < icnt; i++) {
      if (bookpos[j].key == posshash[i]) {
	found = 0;
	for (k = 0; k < mcnt; k++) 
	    if (matches[k] == i) {
	      found = 1;
	      break;
	    }
	/* Position must have at least some wins to be played by book */
	if (!found) {
	    matches[++mcnt] = i;
	    pref[mcnt].move = m[matches[mcnt]].move;
	    r[i].losses = bookpos[j].losses;
	    r[i].wins = bookpos[j].wins;
	    r[i].draws = bookpos[j].draws;

/* by percent score starting from this book position */

           pref[mcnt].score = m[i].score = 
		100*(r[i].wins+(r[i].draws/2))/
		   (MAX(r[i].wins+r[i].losses+r[i].draws,1)) + r[i].wins/2;

	}
	if (mcnt >= MAXMATCH) {
	    fprintf(ofp,"Too many matches in book.\n");
	    goto fini;
	}
      }
    }
  }
fini:  
  if (!(flags & XBOARD))
  {
    fprintf(ofp,"Opening database: %d book positions.\n",bigbookcnt);
    fprintf(ofp,"In this position, %d move%c %s book moves%c\n\n",
	mcnt+1,mcnt+1!=1?'s':(char)NULL,mcnt+1!=1?"are":"is",mcnt+1>0?':':'.');
  }
  /* No book moves */
  if (mcnt == -1) {return(0); }
  k = 0;
  if (bookmode == BOOKPREFER) best = -INFINITY;
  if (mcnt+1) {
    if ( !(flags & XBOARD)) {
      for (i = 0; i <= mcnt; i++) {
	if (!(flags & XBOARD)) {
	  SANMove(m[matches[i]].move,1);
          tot = r[matches[i]].wins+r[matches[i]].draws+r[matches[i]].losses;
	  fprintf(ofp,"%s(%2.0f/%d/%d/%d) ",SANmv,
		100.0*(r[matches[i]].wins+(r[matches[i]].draws/2))/tot,
		r[matches[i]].wins,
		r[matches[i]].losses,
		r[matches[i]].draws);
          if ((i+1) % 4 == 0) fputc('\n',ofp);
	}
      }
      if (!(flags & XBOARD))
        if (i % 4 != 0) fprintf(ofp,"\n\n");
    }
    if (bookmode == BOOKRAND) {
      k = rand();
      k = k % (mcnt+1);
      RootPV = m[matches[k]].move;
      if (!(flags & XBOARD)) {
        printf("\n(Random picked move #%d %s%s from above list)\n",k,
	  algbr[FROMSQ(RootPV)],algbr[TOSQ(RootPV)]);
        tot = r[matches[k]].wins+r[matches[k]].draws+r[matches[k]].losses;
        if (tot != 0)
          printf("B p=%2.0f\n",
	   100*(r[matches[k]].wins+r[matches[k]].draws)/tot);
        else
          printf("p=NO EXPERIENCES\n");
      }
    } else if (bookmode == BOOKBEST) {
      temp = (bookfirstlast > mcnt+1 ? mcnt+1 : bookfirstlast);
      k = rand() % temp;
      RootPV = m[matches[k]].move;
    } else if (bookmode == BOOKWORST) {
      temp = (bookfirstlast > mcnt+1 ? mcnt+1 : bookfirstlast);
      k = rand() % temp;
      RootPV = m[matches[k]].move;
    } else if (bookmode == BOOKPREFER) {
      qsort(&pref,mcnt+1,sizeof(leaf),compare);
      for (i = 0; i <= mcnt; i++) {
	if (!(flags & XBOARD)) {
	  SANMove(pref[i].move,1);
          printf("%s(%d) ",SANmv,pref[i].score);
	  if (pref[i].score > best) best = pref[i].score;
	}
	m[i].move = pref[i].move;
	if (!(flags & XBOARD)) 
          if ((i+1) % 8 == 0) fputc('\n',ofp);
      }
      if (!(flags & XBOARD))
	if (i % 8 != 0) fprintf(ofp,"\n\n");
        temp = (bookfirstlast > mcnt+1 ? mcnt+1 : bookfirstlast);
      /* Choose from the top preferred moves based on distribution */
      maxdistribution = 0;
      for (i = 0; i < temp; i++)
        maxdistribution += pref[i].score;
      /* Do not play moves that have only losses! */
      if (maxdistribution == 0) return(0);
      k = rand() % maxdistribution;
      maxdistribution = 0;
      for (i = 0; i < temp; i++) {
	maxdistribution += pref[i].score;
	if (k >= maxdistribution - pref[i].score &&
	    k < maxdistribution)
	{
	  k = i;
	  RootPV = m[k].move;
	  break;
	}
      }
    }
  }
  return(1);
}
