/* GNU Chess 5.0 - cmd.c - command parser
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
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "version.h"
#include "common.h"
#include "eval.h"
#ifdef UNIVERSAL
#include "univ.h"
#include <conio.h>
char fromboard[10];
#endif

#define prompt ':'

extern short stage, InChkDummy, terminal;
static char inputstr[128], cmd[128], file[128], s[128], logfile[128], 
            gamefile[128],userinput[128];

char subcmd[128],setting[128],subsetting[128];


#ifdef UNIVERSAL
void univ_check (int signal_type)
{
  if (univ_read(fromboard)) {
    printf("Read %s from Universal.\n",fromboard);
    if (strcmp(fromboard,"J") == 0) strcpy(fromboard,"go");
    else if (strcmp(fromboard,"N") == 0) strcpy(fromboard,"new");
    else if (strcmp(fromboard,"T") == 0) strcpy(fromboard,"undo");
  }
  alarm(1);
}
#endif

void InputCmd ()
/*************************************************************************
 *
 *  This is the main user command interface driver.
 *
 *************************************************************************/
{
   char color[2][6] = { "White", "Black" };
   short suffix;
   int i;
   leaf *ptr; 
   int ncmds;
   char *x,*trim;
#ifdef UNIVERSAL
   char *p,c;
#endif

   CLEAR (flags, THINK);
  memset(userinput,0,sizeof(userinput));
  memset(cmd,0,sizeof(cmd));
  memset(inputstr,0,sizeof(inputstr));
#ifdef UNIVERSAL
  if (1) {
    if (flags & UNIV) {
      userinput[0] = '\000';
      p = userinput;
      if (kbhit()) {
        for (;;) {
	  c = getchar();
	  while (c != '\015' && c != '\012' && c != EOF) {
	    *p++ = c;
	    c = getchar();
	  }
	  *p = '\000';
	  break;
	}
      }
      if (strlen(fromboard) != 0) {
	strcpy(userinput,fromboard);
	fromboard[0] = '\0';
      }
      if (strlen(userinput) != 0) {
#else
	if (!(flags & XBOARD)) {
	  printf ("%s (%d) %c ", color[board.side], (GameCnt+1)/2 + 1, prompt);
	  fflush(stdout);
        }
	if (fgets (inputstr, 64, stdin) && inputstr[0])
	    inputstr[strlen(inputstr)-1] = '\000';
	cmd[0] = '\n';
	strcpy(userinput,inputstr);
	sscanf (inputstr, "%s %[^\n]", cmd, inputstr);
	if (cmd[0] == '\n')
	  return;
#endif
	cmd[0] = subcmd[0] = setting[0] = subsetting[0] = '\0';
        ncmds = sscanf (userinput,"%s %s %s %[^\n]",
			cmd,subcmd,setting,subsetting);
#ifdef UNIVERSAL
        userinput[0] = '\000';
        printf ("%s (%d) %c ", color[board.side], (GameCnt+1)/2 + 1, prompt);
        fflush(stdout);
      }
    } else {
      printf ("%s (%d) %c ", color[board.side], (GameCnt+1)/2 + 1, prompt);
      fflush(stdout);
      gets(userinput);
      cmd[0] = subcmd[0] = setting[0] = subsetting[0] = '\0';
      ncmds = sscanf (userinput,"%s %s %s %s",cmd,subcmd,setting,subsetting);
    }
  }
#endif

   /* Put options after command back in inputstr - messy */
   sprintf(inputstr,"%s %s %s",subcmd,setting,subsetting);

   trim = inputstr + strlen(inputstr) - 1;
   while (*trim==' ' && trim>=inputstr)
                *trim--='\0';

   if (strcmp (cmd, "quit") == 0 || strcmp (cmd, "exit") == 0)
      SET (flags, QUIT);
   else if (strcmp (cmd, "help") == 0)
      ShowHelp ();
   else if (strcmp (cmd, "show") == 0)
      ShowCmd (inputstr);
   else if (strncmp (cmd, "book", 4) == 0) {
      if (strncmp(inputstr, "add",3) == 0) {
        sscanf (inputstr, "add %s", file);
        if (access(file,F_OK) < 0) {
	  printf("The syntax to add a new book is:\n\n\tbook add file.pgn\n");
        } else {
          BookPGNReadFromFile (file);
	}
      } else if (strncmp (inputstr, "on", 2) == 0) {
	bookmode = BOOKBEST;
	printf("book now on.\n");
      } else if (strncmp (inputstr, "off", 3) == 0) {
	bookmode = BOOKOFF;
	printf("book now off.\n");
      }
   } else if (strcmp (cmd, "test") == 0)
      TestCmd (inputstr);
   else if (strcmp (cmd, "version") == 0)
      ShowVersion ();
   else if (strcmp (cmd, "pgnsave") == 0)
           {     
		if ( strlen(inputstr) > 0 )
      		  PGNSaveToFile (inputstr,"");
		else
		  printf("Invalid filename.\n");
	   }
   else if (strcmp (cmd, "pgnload") == 0)
      PGNReadFromFile (inputstr);
   else if (strcmp (cmd, "manual") == 0)
      SET (flags, MANUAL);
   else if (strcmp (cmd, "debug") == 0)
   {
      SET (flags, DEBUGG);
      Debugmvl = 0;
      if (strcmp (inputstr, "debug") == 0)
      {
         while (strcmp (inputstr, s))
         {
            sscanf (inputstr, "%s %[^\n]", s, inputstr);
            ptr = ValidateMove (s);
            Debugmv[Debugmvl++] = ptr->move;
            MakeMove (board.side, &ptr->move);
         } 
         i = Debugmvl;
         while (i)
         {
            UnmakeMove (board.side, &Debugmv[--i]);
         } 
      }
   }
   else if (strcmp (cmd, "force") == 0)
	SET (flags, MANUAL);
   else if (strcmp (cmd, "white") == 0)
	;
   else if (strcmp (cmd, "black") == 0)
	;
   else if (strcmp (cmd, "hard") == 0)
	;
   else if (strcmp (cmd, "easy") == 0)
	;
   else if (strcmp (cmd, "list") == 0) {
	if (inputstr[0] == '?')
	{
	  printf("name    - list known players alphabetically\n");
	  printf("score   - list by GNU best result first \n");
	  printf("reverse - list by GNU worst result first\n");
	} else {
          sscanf (inputstr, "%s %[^\n]", cmd, inputstr);
          if (inputstr == '\000') DBListPlayer("rscore");
	  else DBListPlayer(inputstr);
	}
   }
   else if (strcmp (cmd, "post") == 0)
	SET (flags, POST);
   else if (strcmp (cmd, "nopost") == 0)
	CLEAR (flags, POST);
   else if (strcmp (cmd, "name") == 0) {
      strcpy(name, inputstr);
      x = name;
      while (*x != '\000') {
        if (*x == ' ') {
	  *x = '\000';
	  break;
	}
      }
      suffix = 0;
      for (;;) {
	sprintf(logfile,"log.%03d",suffix);
 	sprintf(gamefile,"game.%03d",suffix);
	if (access(logfile,F_OK) < 0) {
	  ofp = fopen(logfile,"w");
	  break;
	} else 
	  suffix++;
      }
   }
   else if (strcmp (cmd, "result") == 0) {
     if (ofp != stdout) {  
	fprintf(ofp, "result: %s\n",inputstr);
	fclose(ofp); 
	ofp = stdout;
        printf("Save to %s\n",gamefile);
        PGNSaveToFile (gamefile, inputstr);
	DBUpdatePlayer (name, inputstr);
     }
   }	
   else if (strcmp (cmd, "rating") == 0) {
      sscanf(inputstr,"%d %d",&myrating,&opprating); 
      fprintf(ofp,"my rating = %d, opponent rating = %d\n",myrating,opprating); 
      /* Change randomness of book based on opponent rating. */
      /* Basically we play narrower book the higher the opponent */
      if (opprating >= 1700) bookfirstlast = 2;
      else if (opprating >= 1700) bookfirstlast = 2;
      else bookfirstlast = 2;
   }
   else if (strcmp (cmd, "activate") == 0) {
	CLEAR (flags, TIMEOUT);
	CLEAR (flags, ENDED);
   }
   else if (strcmp (cmd, "new") == 0) {
     InitVars ();
     NewPosition ();
     CLEAR (flags, MANUAL);
     CLEAR (flags, THINK);
     bookloaded = 0;
     myrating = opprating = 0;
   }
   else if (strcmp (cmd, "time") == 0) {
     sscanf (inputstr, "%s %[^\n]", s, inputstr);
     TimeLimit[1^board.side] = atoi(s) / 100;
   }
   else if (strcmp (cmd, "otim") == 0)
	;
   else if (strcmp (cmd, "random") == 0)
	;
   else if (strcmp (cmd, "debugply") == 0)
      DebugPly = atoi (inputstr);
   else if (strcmp (cmd, "debugdepth") == 0)
      DebugDepth = atoi (inputstr);
   else if (strcmp (cmd, "debugnode") == 0)
      DebugNode = atoi (inputstr);
   else if (strcmp (cmd, "hash") == 0)
   {
      sscanf (inputstr, "%s %[^\n]", cmd, inputstr);
      if (strcmp (cmd, "off") == 0)
         CLEAR (flags, USEHASH);
      else if (strcmp (cmd, "on") == 0)
         SET (flags, USEHASH);
      printf ("Hashing %s\n", flags & USEHASH ? "on" : "off");
   }
   else if (strcmp (cmd, "hashsize") == 0)
   {
      i = atoi (inputstr);
      TTHashMask = 0;
      while ((i >>= 1) > 0)
      {
         TTHashMask <<= 1;
         TTHashMask |= 1;
      }
      HashSize = TTHashMask + 1;
      printf ("Adjusting HashSize to %d slots\n", HashSize);
      InitHashTable (); 
   }
   else if (strcmp (cmd, "null") == 0)
   {
      sscanf (inputstr, "%s %[^\n]", cmd, inputstr);
      if (strcmp (cmd, "off") == 0)
         CLEAR (flags, USENULL);
      else if (strcmp (cmd, "on") == 0)
         SET (flags, USENULL);
      printf ("Null moves %s\n", flags & USENULL ? "on" : "off");
   }
   else if (strcmp (cmd, "xboard") == 0)
   {
      sscanf (inputstr, "%s %[^\n]", cmd, inputstr);
      if (strcmp (cmd, "off") == 0)
         CLEAR (flags, XBOARD);
      else if (strcmp (cmd, "on") == 0)
         SET (flags, XBOARD);
      else if (!(flags & XBOARD)) { /* set if unset and only xboard called */
	 SET (flags, XBOARD);	    /* like in xboard/winboard usage */
      }
   }
   else if (strcmp (cmd, "protover") == 0)
   {
      if (flags & XBOARD) {
	/* Note: change this if "analyze" or "draw" commands are added, etc. */
	printf("feature setboard=1 analyze=0 ping=1 draw=0"
	       " variants=\"normal\" myname=\"%s %s\" done=1\n",
	       PROGRAM, VERSION);
	fflush(stdout);
      }
   }
#ifdef UNIVERSAL
  else if (strcmp(cmd,"universal") == 0) {
    if (strcmp(subcmd,"on") == 0) {
      SET (flags, UNIV);
      univ_init ();
      signal (SIGALRM, univ_check);
      alarm(1);
    } else if (strcmp(subcmd,"off") == 0) {
      CLEAR (flags, UNIV);
      signal (SIGALRM, SIG_IGN);
    }
  }
#endif
   else if (strcmp (cmd, "depth") == 0) {
      SearchDepth = atoi (inputstr);
      printf("Search to a depth of %d\n",SearchDepth);
   }
   else if (strcmp (cmd, "level") == 0)
   {
      SearchDepth = 0;
      sscanf (inputstr, "%hd %f %hd", &TCMove, &TCTime, &TCinc);
      if (TCMove == 0) {
	TCMove =  35 /* MIN((5*(GameCnt+1)/2)+1,60) */;
	printf("TCMove = %d\n",TCMove);
	suddendeath = 1;
      } else
	suddendeath = 0;
      if (TCTime == 0) {
         SET (flags, TIMECTL);
	 SearchTime = TCinc / 2;
         printf("Fischer increment of %d seconds\n",TCinc);
      }
      else
      {
         SET (flags, TIMECTL);
         MoveLimit[white] = MoveLimit[black] = TCMove - (GameCnt+1)/2;
         TimeLimit[white] = TimeLimit[black] = TCTime * 60;
	 if (!(flags & XBOARD)) {
	   printf ("Time Control: %d moves in %.2f secs\n", 
	          MoveLimit[white], TimeLimit[white]);
	   printf("Fischer increment of %.2f seconds\n",TCinc);
	 }
      }
   }
   else if (strcmp (cmd, "load") == 0 || strcmp (cmd, "epdload") == 0)
   {
      LoadEPD (inputstr);
      if (!ValidateBoard())
      {
	 SET (flags, ENDED);
         printf ("Board is wrong!\n");
      }
   }
   else if (strcmp (cmd, "save") == 0 || strcmp (cmd, "epdsave") == 0)
	{  
	   if ( strlen(inputstr) > 0 )
             SaveEPD (inputstr);
	   else
	     printf("Invalid filename.\n");
	}
   else if (strcmp (cmd, "epd") == 0)
   {
      ParseEPD (inputstr);
      NewPosition();
      ShowBoard();
      printf ("\n%s : Best move = %s\n", id, solution); 
   }
   else if (strcmp (cmd, "setboard") == 0)
   {
      /* setboard uses FEN, not EPD, but ParseEPD will accept FEN too */
      ParseEPD (inputstr);
      NewPosition();
   }
   else if (strcmp (cmd, "ping") == 0)
   {
      /* If ping is received when we are on move, we are supposed to 
	 reply only after moving.  In this version of GNU Chess, we
	 never read commands while we are on move, so we don't have to
	 worry about that here. */
      printf("pong %s\n", inputstr);
      fflush(stdout);
   }
   else if (strcmp (cmd, "switch") == 0)
   {
      board.side = 1^board.side;
      printf ("%s to move\n", board.side == white ? "White" : "Black");
   }
   else if (strcmp (cmd, "go") == 0)
   {
      SET (flags, THINK);
      CLEAR (flags, MANUAL);
      CLEAR (flags, TIMEOUT);
      CLEAR (flags, ENDED);
      computer = board.side;
   }
   else if (strcmp (cmd, "solve") == 0 || strcmp (cmd, "solveepd") == 0)
      Solve (inputstr);
   else if (strcmp (cmd,"remove") == 0) {
    if (GameCnt >= 0) {
       CLEAR (flags, ENDED);
       CLEAR (flags, TIMEOUT);
       UnmakeMove (board.side, &Game[GameCnt].move);
       if (GameCnt >= 0) {
         UnmakeMove (board.side, &Game[GameCnt].move);
         if (!(flags & XBOARD))
           ShowBoard ();
       }
       PGNSaveToFile ("game.log","");
    } else
       printf ("No moves to undo! \n");
   }
   else if (strcmp (cmd, "undo") == 0)
   {
      if (GameCnt >= 0)
         UnmakeMove (board.side, &Game[GameCnt].move);
      else
	 printf ("No moves to undo! \n");
      MoveLimit[board.side]++;
      TimeLimit[board.side] += Game[GameCnt+1].et;
      if (!(flags & XBOARD)) ShowBoard ();
   }

   /* everything else must be a move */
   else
   {
      ptr = ValidateMove (cmd);
      if (ptr != NULL) 
      {
	 SANMove (ptr->move, 1);
	 MakeMove (board.side, &ptr->move);
	 strcpy (Game[GameCnt].SANmv, SANmv);
	 printf("%d. ",GameCnt/2+1);
	 printf("%s",cmd);
	 if (ofp != stdout) {
	   fprintf(ofp,"%d. ",GameCnt/2+1);
	   fprintf(ofp,"%s",cmd);
	 }
	 putchar('\n');
	 fflush(stdout);
  	 if (ofp != stdout) {
	   fputc('\n',ofp);
	   fflush(ofp);
         }
         if (!(flags & XBOARD)) ShowBoard (); 
	 SET (flags, THINK);
      }
      else {
      }
   }
}



void ShowCmd (char *subcmd)
/************************************************************************
 *
 *  The show command driver section.
 *
 ************************************************************************/
{
   char cmd[10];

   sscanf (subcmd, "%s %[^\n]", cmd, subcmd);
   if (strcmp (cmd, "board") == 0)
      ShowBoard ();
   else if (strcmp (cmd, "rating") == 0)
   {
      printf("My rating = %d\n",myrating);
      printf("Opponent rating = %d\n",opprating);
   } 
   else if (strcmp (cmd, "time") == 0)
      ShowTime ();
   else if (strcmp (cmd, "moves") == 0)
   {
      GenCnt = 0;
      TreePtr[2] = TreePtr[1];
      GenMoves (1);      
      ShowMoveList (1);
      printf ("No. of moves generated = %ld\n", GenCnt);
   }
   else if (strcmp (cmd, "escape") == 0)
   {
      GenCnt = 0;
      TreePtr[2] = TreePtr[1];
      GenCheckEscapes (1);      
      ShowMoveList (1);
      printf ("No. of moves generated = %ld\n", GenCnt);
   }
   else if (strcmp (cmd, "noncapture") == 0)
   {
      GenCnt = 0;
      TreePtr[2] = TreePtr[1];
      GenNonCaptures (1);      
      FilterIllegalMoves (1);
      ShowMoveList (1);
      printf ("No. of moves generated = %ld\n", GenCnt);
   }
   else if (strcmp (cmd, "capture") == 0)
   {
      GenCnt = 0;
      TreePtr[2] = TreePtr[1];
      GenCaptures (1);      
      FilterIllegalMoves (1);
      ShowMoveList (1);
      printf ("No. of moves generated = %ld\n", GenCnt);
   }
   else if (strcmp (cmd, "eval") == 0 || strcmp (cmd, "score") == 0)
   {
      int s, wp, bp, wk, bk;
      short r, c, sq;
      BitBoard *b;

      phase = PHASE;
      GenAtaks ();
      FindPins (&pinned);
      hunged[white] = EvalHung(white);
      hunged[black] = EvalHung(black);
      b = board.b[white];
      pieces[white] = b[knight] | b[bishop] | b[rook] | b[queen]; 
      b = board.b[black];
      pieces[black] = b[knight] | b[bishop] | b[rook] | b[queen]; 
      wp = ScoreP (white);
      bp = ScoreP (black);
      wk = ScoreK (white);
      bk = ScoreK (black);
      printf ("White:  Mat:%4d/%4d  P:%d  N:%d  B:%d  R:%d  Q:%d  K:%d  Dev:%d  h:%d x:%d\n",
	board.pmaterial[white], board.material[white], wp, ScoreN(white), 
        ScoreB(white), ScoreR(white), ScoreQ(white), wk, 
        ScoreDev(white), hunged[white], ExchCnt[white]);
      printf ("Black:  Mat:%4d/%4d  P:%d  N:%d  B:%d  R:%d  Q:%d  K:%d  Dev:%d  h:%d x:%d\n",
	board.pmaterial[black], board.material[black], bp, ScoreN(black), 
        ScoreB(black), ScoreR(black), ScoreQ(black), bk,
        ScoreDev(black), hunged[black], ExchCnt[black]);
      printf ("Phase: %d\t", PHASE);
      s = ( EvaluateDraw () ? DRAWSCORE : Evaluate (-INFINITY, INFINITY));
      printf ("score = %d\n", s);
      printf ("\n");
      return;
      for (r = 56; r >= 0; r -= 8)
      {
         printf ("     +---+---+---+---+---+---+---+---+\n");
         printf ("   %d |", (r >> 3) + 1);
         for (c = 0; c < 8; c++)
         {
            sq = r + c;
	    if (cboard[sq] == 0)
 	       printf ("   |");
	    else
               printf ("%3d|", pscore[sq]);
         }
         printf ("\n");
      }

      printf ("     +---+---+---+---+---+---+---+---+\n");
      printf ("       a   b   c   d   e   f   g   h  \n");
   }
   else if (strcmp (cmd, "game") == 0)
     ShowGame ();
   else if (strcmp (cmd, "pin") == 0)
   {
      BitBoard b;
      GenAtaks ();
      FindPins (&b);
      ShowBitBoard (&b);
   }
}


void BookCmd (char *subcmd)
/*************************************************************************
 *
 *  The book command driver section.
 *
 *************************************************************************/
{
   char cmd[10];
   char bookfile[64];
   short bookply;

   sscanf (subcmd, "%s %[^\n]", cmd, subcmd);
   if (strcmp (cmd, "make") == 0)
   {
      sscanf (subcmd, "%s %hd\n", bookfile, &bookply);
      /* MakeBinBook (bookfile, bookply); */
   }
}



void TestCmd (char *subcmd)
/*************************************************************************
 *
 *  The test command driver section.
 *
 *************************************************************************/
{
   char cmd[10];

   sscanf (subcmd, "%s %[^\n]", cmd, subcmd);
   if (strcmp (cmd, "movelist") == 0)
      TestMoveList ();
   else if (strcmp (cmd, "capture") == 0)
      TestCaptureList ();
   else if (strcmp (cmd, "movegenspeed") == 0)
      TestMoveGenSpeed ();
   else if (strcmp (cmd, "capturespeed") == 0)
      TestCaptureGenSpeed ();
   else if (strcmp (cmd, "eval") == 0)
      TestEval ();
   else if (strcmp (cmd, "evalspeed") == 0)
      TestEvalSpeed ();
}


void ShowHelp ()
/**************************************************************************
 *
 *  Display all the help commands.
 *
 **************************************************************************/
{
   printf ("+-----------------------------------------------+\n");
   printf ("|                    H E L P                    |\n");
   printf ("+-----------------------------------------------+\n");
}

