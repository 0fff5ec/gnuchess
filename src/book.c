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
#include <errno.h>
#include <unistd.h>

#include "common.h"
#include "book.h"

#define MAXMOVES 200
#define MAXMATCH 100

/* Right now, bigbookcnt is always the same as bookcnt */			   
static int bookcnt, bigbookcnt = 0;
static int runningbookcnt = 0;
static HashType posshash[MAXMOVES];

/*
 * The last byte of magic_str should be the version
 * number of the format, in case we have to change it.
 *
 * Format 0x01 had an index field which is now gone.
 */

#define MAGIC_LENGTH 5

static const char magic_str[] = "\x42\x23\x08\x15\x02";

static int check_magic(FILE *f)
{
  char buf[MAGIC_LENGTH];
  int r;

  r = fread(&buf, 1, MAGIC_LENGTH, f);
  return (r == MAGIC_LENGTH &&
	  memcmp(buf, magic_str, MAGIC_LENGTH) == 0);
}

static int write_magic(FILE *f)
{
  if (MAGIC_LENGTH != fwrite(&magic_str, 1, MAGIC_LENGTH, f)) {
    return BOOK_EIO;
  } else {
    return BOOK_SUCCESS;
  }
}

/*
 * MAXBOOK now has to be a power of 2, as we use the
 * lower bits to calculate the index of some hash in
 * this array. NOTE: Due to this change, the binary
 * format of book files has changed. We write some
 * magic string in order to check if the file has the
 * correct format.
 */

#define MAXBOOK (1UL<<20)
#define DIGEST_BITS 20
#define DIGEST_MASK (MAXBOOK-1)

static struct hashtype {
  uint16_t wins;
  uint16_t losses;
  uint16_t draws;
  HashType key;
} bookpos[MAXBOOK];

static inline int is_empty(uint32_t index)
{
  return 
    bookpos[index].key    == 0 &&
    bookpos[index].wins   == 0 &&
    bookpos[index].draws  == 0 &&
    bookpos[index].losses == 0;
}

/*
 * Initial hash function, relies on the quality of the lower
 * bits of the 64 bit hash function
 */
#define DIGEST_START(i,key) \
  ((i) = (key) & DIGEST_MASK)

/*
 * See if there is already a matching entry
 */
#define DIGEST_MATCH(i,the_key) \
  ((the_key) == bookpos[i].key)

/*
 * See if the entry is empty
 */
#define DIGEST_EMPTY(i) is_empty(i)

/*
 * Check for collision
 */
#define DIGEST_COLLISION(i,key) \
  (!DIGEST_MATCH(i,key) && !DIGEST_EMPTY(i))

/*
 * The next macro is used in the case of hash collisions.
 * We use double hashing with the higher bits of the key.
 * In order to have the shift relatively prime to the hash
 * size, we OR by 1.
 */
#define DIGEST_NEXT(i,key) \
  ((i) = ((i) + (((key) >> DIGEST_BITS) | 1)) & DIGEST_MASK)


/* Mainly for debugging purposes */
static int bookhashcollisions = 0;

/*
 * This is considered to be the limit for the hash, I chose
 * 95% because it is Monday... No, I am open for suggestions
 * for the right value, I just don't know.
 */
#define BOOK_HASH_LIMIT (0.95 * MAXBOOK)

/*
 * This is the record as it will be written in the binary
 * file in network byte order. HashType is uint64_t. To
 * avoid endianness and padding issues, we do not read or
 * write structs but put the values in an unsigned char array.
 */

static unsigned char buf[2+2+2+8];

/* Offsets */
static const int wins_off   = 0;
static const int losses_off = 2;
static const int draws_off  = 4;
static const int key_off    = 6;

static void buf_to_book(void)
{
  HashType key;
  uint32_t i;

  key = ((uint64_t)buf[key_off] << 56)
    | ((uint64_t)buf[key_off+1] << 48)
    | ((uint64_t)buf[key_off+2] << 40)
    | ((uint64_t)buf[key_off+3] << 32)
    | ((uint64_t)buf[key_off+4] << 24)
    | ((uint64_t)buf[key_off+5] << 16)
    | ((uint64_t)buf[key_off+6] << 8)
    | ((uint64_t)buf[key_off+7]);
  /*
   * This is an infinite loop if the hash is 100% full,
   * but other parts should check that this does not happen.
   */
  for (DIGEST_START(i, key);
       DIGEST_COLLISION(i, key);
       DIGEST_NEXT(i, key))
    /* Skip */
    bookhashcollisions++;

  bookpos[i].wins   += (buf[wins_off]   << 8) | buf[wins_off  +1];
  bookpos[i].draws  += (buf[draws_off]  << 8) | buf[draws_off +1];
  bookpos[i].losses += (buf[losses_off] << 8) | buf[losses_off+1];
  bookpos[i].key = key;
}

static void book_to_buf(uint32_t index)
{
  int k;

  for (k=0; k<2; k++) {
    buf[wins_off + k]   = ((bookpos[index].wins)   >> ((1-k)*8)) & 0xff;
    buf[draws_off + k]  = ((bookpos[index].draws)  >> ((1-k)*8)) & 0xff;
    buf[losses_off + k] = ((bookpos[index].losses) >> ((1-k)*8)) & 0xff;
  }
  for (k=0; k<8; k++) {
    buf[key_off + k] = ((bookpos[index].key) >> ((7-k)*8)) & 0xff;
  }
}

static int compare(const void *aa, const void *bb)
{
  const leaf *a = aa;
  const leaf *b = bb;
  
  if (b->score > a->score) return(1);
  else if (b->score < a->score) return(-1);
  else return(0);
}

/* 
 * Return values are defined in common.h
 */

int BookBuilderOpen(void)
{
  FILE *rfp, *wfp;

  if ((rfp = fopen(BOOKRUN,"rb")) != NULL) {
    printf("Opened existing book!\n");
    if (!check_magic(rfp)) {
      fprintf(stderr, 
	      "File %s does not conform to the new format.\n"
	      "Consider rebuilding your book.\n",
	      BOOKRUN);
      return BOOK_EFORMAT;
    }
    runningbookcnt = 0;
    bookhashcollisions = 0;
    while ( 1 == fread(&buf, sizeof(buf), 1, rfp) ) {
      buf_to_book();
      runningbookcnt++;
    }
    printf("Read %d book positions\n", runningbookcnt);
    printf("Got %d hash collisions\n", bookhashcollisions);
    fclose(rfp);
  } else {
    wfp = fopen(BOOKRUN,"wb");
    if (wfp == NULL) {
      fprintf(stderr, "Could not create %s file: %s\n",
		 BOOKRUN, strerror(errno));
      return BOOK_EIO;
    }
    if (write_magic(wfp) != BOOK_SUCCESS) {
      fprintf(stderr, "Could not write to %s: %s\n",
	      BOOKRUN, strerror(errno));
      return BOOK_EIO;
    }
    if (fclose(wfp) != 0) {
      fprintf(stderr, "Could not write to %s: %s\n",
	      BOOKRUN, strerror(errno));
      return BOOK_EIO;
    }
    printf("Created new book %s!\n", BOOKRUN);
    rfp = fopen(BOOKRUN, "rb");
    if (rfp == NULL) {
      fprintf(stderr, "Could not open %s for reading: %s\n",
	      BOOKRUN, strerror(errno));
      return BOOK_EIO;
    }
  }
  return BOOK_SUCCESS;
}

/*
 * Store the position and results of last search
 * if and only if in the first 10 moves of the game.
 * This routine is called after the computer makes its
 * most recent move. Lastly, after the 10th move, on 
 * the 11th move, store the data out to the running file.
 */
/*
 * NOTE: Before you build a book, you have to call
 * BookBuilderOpen() now, after building it BookBuilderClose()
 * in order to actually write the book to disk.
 */

int BookBuilder(short result, short side)
{
  uint32_t i;
  
  /* Only first BOOKDEPTH moves */
  if (GameCnt/2+1 >= BOOKDEPTH) 
    return BOOK_EMIDGAME;
  CalcHashKey();
  for (DIGEST_START(i, HashKey);
       ; 
       DIGEST_NEXT(i, HashKey)) {
    if (DIGEST_MATCH(i, HashKey)) {
      existpos++;
      break;
    } else if (DIGEST_EMPTY(i)) {
      if (runningbookcnt > BOOK_HASH_LIMIT)
	return BOOK_EFULL;
      bookpos[i].key = HashKey;
      newpos++;
      runningbookcnt++;
      break;
    } else {
      bookhashcollisions++;
    }
  }
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
  return BOOK_SUCCESS;
}

int BookBuilderClose(void)
{
  /*
   * IMHO the following part needs no temporary file.
   * If two gnuchess invocations try to write the same
   * file at the same time, this goes wrong anyway.
   * Please correct me if I am wrong. If you generate
   * a temporary file, try to generate it in some writable
   * directory.
   */
  FILE *wfp;
  unsigned int i;
  int res;

  wfp = fopen(BOOKRUN, "wb");
  if (wfp == NULL) {
    perror("Could not open %s for writing");
    return BOOK_EIO;
  }
  write_magic(wfp);
  for (i = 0; i < MAXBOOK; i++) {
    if (is_empty(i)) continue;
    book_to_buf(i);
    /* Should we check the return value here, too? */
    fwrite(&buf, sizeof(buf), 1, wfp);
  }
  res = fclose(wfp);
  if (res != 0) {
    perror("Error writing opening book");
    return BOOK_EIO;
  }
  printf("Got %d hash collisions\n", bookhashcollisions);
  return BOOK_SUCCESS;
}

/*
 * Return values are defined in common.h
 * NOTE: This function now returns BOOK_SUCCESS on
 * success, which is 0. So all the possible callers
 * should adjust to this. (At present it is only called
 * in iterate.c.) It used to return 1 on success before.
 */

int BookQuery(int BKquery)
{
  /*
   * BKquery denotes format; 0 text, 1 engine 
   * In general out put is engine compliant, lines start with a blank
   * and end with emtpy line
   */
  int i,j,k,icnt = 0, mcnt, found, tot, maxdistribution;
  int matches[MAXMATCH], best = 0;
  short bestsofar;
  leaf m[MAXMOVES];
  leaf pref[MAXMOVES];
  struct {
    uint16_t wins;
    uint16_t losses;
    uint16_t draws;
  } r[MAXMOVES];
  FILE *rfp;
  leaf *p;
  short side,xside,temp;

  bestsofar = 0;
  mcnt = -1;
  side = board.side;
  xside = 1^side;
  rfp = fopen(BOOKBIN,"rb");
  if (rfp == NULL) {
    if (!(flags & XBOARD) || BKquery == 1 )
      fprintf(ofp," no book (%s).\n\n",BOOKBIN);
    return BOOK_ENOBOOK;
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
      fprintf(ofp,"Read opening book (%s)... ",BOOKBIN);
    fseek(rfp,0L,SEEK_SET);
    bookcnt = 0;
    bookhashcollisions = 0;
    if (check_magic(rfp)) {
      while ( 1 == fread(&buf, sizeof(buf), 1, rfp) ) {
	buf_to_book();
	bookcnt++;
      }
      bigbookcnt += bookcnt;
      bookloaded = 1;
      if (!(flags & XBOARD))
        fprintf(ofp,"%d hash collisions... ", bookhashcollisions);
    } else {
      /*
       * The following doesn't need to exit() because the old
       * book will not be overwritten.
       */
      fprintf(stderr, 
	      " File %s does not conform to the new format.\n"
	      " Consider rebuilding the book.\n\n",
	      BOOKBIN);
      return BOOK_EFORMAT;
    }
  }      
  for (i = 0; i < icnt; i++) {
    for (DIGEST_START(j,posshash[i]);
	 !DIGEST_EMPTY(j);
	 DIGEST_NEXT(j, posshash[i])) {
      if (DIGEST_MATCH(j, posshash[i])) {
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
	  fprintf(ofp," Too many matches in book.\n\n");
	  goto fini;
	}
	break; /* If we found a match, the following positions can not match */
      }
    }
  }

fini:  
  if (!(flags & XBOARD) || BKquery == 1)
  {
    fprintf(ofp," Opening database: %d book positions.\n",bigbookcnt);
    if (mcnt+1 == 0) {
      fprintf(ofp," In this position, there is no book move.\n\n");
    } else if (mcnt+1 == 1) {
      fprintf(ofp," In this position, there is one book move:\n");
    } else {
      fprintf(ofp," In this position, there are %d book moves:\n", mcnt+1);
    }
  }
  /* No book moves */
  if (mcnt == -1) {
    return BOOK_ENOMOVES;
  }
  k = 0;
  if (bookmode == BOOKPREFER) best = -INFINITY;
  if (mcnt+1) {
    if ( !(flags & XBOARD) || BKquery == 1 ) {
      for (i = 0; i <= mcnt; i++) {
	if (!(flags & XBOARD) || BKquery == 1 ) {
	  SANMove(m[matches[i]].move,1);
          tot = r[matches[i]].wins+r[matches[i]].draws+r[matches[i]].losses;
	  fprintf(ofp," %s(%2.0f/%d/%d/%d) ",SANmv,
		100.0*(r[matches[i]].wins+(r[matches[i]].draws/2.0))/tot,
		r[matches[i]].wins,
		r[matches[i]].losses,
		r[matches[i]].draws);
          if ((i+1) % 4 == 0) fputc('\n',ofp);
	}
      }
      if (!(flags & XBOARD) || BKquery == 1 )
        if (i % 4 != 0) fprintf(ofp," \n \n");
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
	   100.0*(r[matches[k]].wins+r[matches[k]].draws)/tot);
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
	if (!(flags & XBOARD) || BKquery == 1) {
	  SANMove(pref[i].move,1);
          printf(" %s(%d) ",SANmv,pref[i].score);
	  if (pref[i].score > best) best = pref[i].score;
	}
	m[i].move = pref[i].move;
	if (!(flags & XBOARD) || BKquery == 1) 
          if ((i+1) % 8 == 0) fputc('\n',ofp);
      }
      if (!(flags & XBOARD) || BKquery == 1)
	if (i % 8 != 0) fprintf(ofp," \n \n");
        temp = (bookfirstlast > mcnt+1 ? mcnt+1 : bookfirstlast);
      /* Choose from the top preferred moves based on distribution */
      maxdistribution = 0;
      for (i = 0; i < temp; i++)
        maxdistribution += pref[i].score;
      /* Do not play moves that have only losses! */
      if (maxdistribution == 0) 
	return BOOK_ENOMOVES;
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
  return BOOK_SUCCESS;
}
