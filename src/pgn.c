/* GNU Chess 5.0 - pgn.c - pgn game format code
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
#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "common.h"
#include "version.h"
#include "book.h"

void PGNSaveToFile (char *file, char *resultstr)
/****************************************************************************
 *
 *  To save a game into PGN format to a file.  If the file does not exist,
 *  it will create it.  If the file exists, the game is appended to the file.
 *
 ****************************************************************************/
{
   FILE *fp;
   char s[100];
   char r[10];
   char *rp;
   char *p;
   short i;
   time_t secs;
   struct tm *timestruct;

   fp = fopen (file, "a");		/*  Can we append to it?  */
   if (fp == NULL)
   {
      printf ("Cannot write to file %s\n", file);
      return;
   }

   /* Write the seven tags */
   fprintf (fp, "[Event \"\"]\n");
   fprintf (fp, "[Site \"\"]\n");
   secs=time(0);
   timestruct=localtime((time_t*) &secs);
   fprintf(fp,"[Date \"%4d.%02d.%02d\"]\n",timestruct->tm_year+1900,
           timestruct->tm_mon+1,timestruct->tm_mday);
   fprintf (fp, "[Round \"\"]\n");
   if (computerplays == white) 
     fprintf (fp, "[White \"%s %s\"]\n",PROGRAM,VERSION);
   else
     fprintf (fp, "[White \"%s\"]\n",name);
   if (computerplays == black)
     fprintf (fp, "[Black \"%s %s\"]\n",PROGRAM,VERSION);
   else
     fprintf (fp, "[Black \"%s\"]\n",name);
   fprintf (fp, "[WhiteELO \"%d\"]\n",computer==white?myrating:opprating);
   fprintf (fp, "[BlackELO \"%d\"]\n",computer==white?opprating:myrating);
   p = resultstr;
   rp = r;
   while (*p != '{' && *p != '\000' && *p != ' ')
     *rp++ = *p++;
   *rp = '\000';
   fprintf (fp, "[Result \"%s\"]\n",r);
   fprintf (fp, "\n");

   s[0] = '\0';
   for (i = 0; i <= GameCnt; i += 2)
   {
      sprintf (s, "%s%d. %s %s ", s, i/2+1, Game[i].SANmv, Game[i+1].SANmv);
      if (strlen (s) > 80)
      {
         p = s + 79;
         while (*p-- != ' ');
         *++p = '\0'; 
         fprintf (fp, "%s\n", s);
         strcpy (s, p+1);
      }
   }
   fprintf (fp, "%s", s);
   fprintf (fp, "%s", resultstr);
   fprintf (fp, "\n\n");
   fclose (fp);

}      


void PGNReadFromFile (char *file)
/****************************************************************************
 *
 *  To read a game from a PGN file.
 *
 ****************************************************************************/
{
   FILE *fp;
   char s[100], c, wmv[8], bmv[8];
   int moveno;
   leaf *p;

   fp = fopen (file, "r");
   if (fp == NULL)
   {
      printf ("Cannot open file %s\n", file);
      return;
   }

   /* Skip all the tags */
   do
   {
      if ((c = fgetc (fp)) == '[')
         fgets (s, 100, fp);
   } while (c == '[');
   ungetc (c, fp);

   InitVars ();
   while (!feof (fp))
   {
      c = fgetc(fp);
      if (c == '*') break;
      ungetc (c, fp);
      fscanf (fp, "%d. %s %s ", &moveno, wmv, bmv);
      p = ValidateMove (wmv);
      if (!p)
      {
	 printf ("Illegal move %d. %s\n", moveno, wmv);
	 break;
      }
      MakeMove (white, &p->move);
      strcpy (Game[GameCnt].SANmv, wmv);
      if (*bmv == '*' ) break;
      p = ValidateMove (bmv);
      if (!p)
      {
	 printf ("Illegal move %d. ... %s\n", moveno, bmv);
	 break;
      }
      MakeMove (black, &p->move);
      strcpy (Game[GameCnt].SANmv, bmv);
   }
   printf ("%d\n", GameCnt);
   fclose (fp);
   ShowBoard ();
   TTClear ();
   return;
}

void BookPGNReadFromFile (char *file)
/****************************************************************************
 *
 *  To read a game from a PGN file and store out the hash entries to book.
 *
 ****************************************************************************/
{
   FILE *fp;
   char s[100], c, wmv[8], bmv[8];
   char header[2000];
   int moveno, gnucolor, result, n, i, ngames = 0;
   leaf *p;
   struct timeval t1, t2;
   struct timezone tz;
   unsigned long et;
   short playercolor, examinecolor, playerfound;
/* Only players in the table below are permitted into the opening book 
   from the PGN files. Please expand the table as needed. Generally,
   I would recommend only acknowledged GM's and IM's and oneself, but
   because of the self-changing nature of the book, anything inferior
   will eventually be eliminated through automatic play as long as
   you feed the games the program plays back to itself with "book add pgnfile"
*/
#define MAXPLAYERS 103
  char *player[MAXPLAYERS] = {
  "Alekhine",
  "Adams",
  "Anand",
  "Anderssen",
  "Andersson",
  "Aronin",
  "Averbakh",
  "Balashov",
  "Beliavsky",
  "Benko",
  "Bernstein",
  "Bird",
  "Bogoljubow",
  "Bolbochan",
  "Boleslavsky",
  "Byrne",
  "Botvinnik",
  "Bronstein",
  "Browne",
  "Capablanca",
  "Chigorin",
  "Christiansen",
  "De Firmian",
  "Deep Blue",
  "Deep Thought",
  "Donner",
  "Dreev",
  "Duras",
  "Euwe",
  "Evans",
  "Filip",
  "Fine",
  "Fischer",
  "Flohr",
  "Furman",
  "Gelfand",
  "Geller",
  "Gereben",
  "Glek",
  "Gligoric",
  "GNU",
  "Golombek",
  "Gruenfeld",
  "Guimard",
  "Hodgson",
  "Ivanchuk",
  "Ivkov",
  "Janowsky",
  "Kamsky",
  "Kan",
  "Karpov",
  "Kasparov",
  "Keres",
  "Korchnoi",
  "Kortschnoj",
  "Kotov",
  "Kramnik",
  "Kupreich",
  "Lasker",
  "Lautier",
  "Letelier",
  "Lilienthal",
  "Ljubojevic",
  "Marshall",
  "Maroczy",
  "Mieses",
  "Miles",
  "Morphy",
  "Mueller",
  "Nimzowitsch",
  "Nunn",
  "Opocensky",
  "Pachman",
  "Petrosian",
  "Piket",
  "Pilnik",
  "Pirc",
  "Polgar",
  "Portisch",
  "Psakhis",
  "Ragozin",
  "Reshevsky",
  "Reti",
  "Romanish",
  "Rubinstein",
  "Saemisch",
  "Seirawan",
  "Shirov",
  "Short",
  "Silman",
  "Smyslov",
  "Sokolsky",
  "Spassky",
  "Sveshnikov",
  "Stahlberg",
  "Steinitz",
  "Tal",
  "Tarjan",
  "Tartakower",
  "Timman",
  "Topalov",
  "Torre",
  "Vidmar"

  };

   et = 0.0;
   gettimeofday (&t1, &tz);
   result = -1;
   fp = fopen (file, "r");
   if (fp == NULL)
   {
      printf ("Cannot open file %s\n", file);
      return;
   }

   BookBuilder(-1,-1, -1, -1);
   newpos = existpos = 0;

   nextgame:

   memset(header,0,sizeof(header));
   InitVars ();
   NewPosition ();
   CLEAR (flags, MANUAL);
   CLEAR (flags, THINK);
   myrating = opprating = 0;

   playerfound = 0;
   examinecolor = -1;
   playercolor = -1;
   /* Skip all the tags */
   do
   {
      if ((c = fgetc (fp)) == '[') {
         fgets (s, 100, fp);
	 strcat(header,s);
      }
      if (strncmp(s,"White ",6) == 0) { examinecolor = white; ngames++; }
      if (strncmp(s,"Black ",6) == 0) examinecolor = black;
      for (i = 0; i < MAXPLAYERS; i++)
 	if (strstr(s,player[i]) != NULL) {
	  playerfound = 1;
          playercolor = examinecolor;
	  break;
 	}
      if (strncmp(s,"Result \"1-0",10) == 0)
	result = R_WHITE_WINS;
      else if (strncmp(s,"Result \"0-1",10) == 0)
	result = R_BLACK_WINS;
      else if (strncmp(s,"Result \"1/2-1/2",10) == 0)
	result = R_DRAW;
      else if (strncmp(s,"Result",6) == 0)
	result = R_NORESULT;
   } while (c == '[');

   while (1)
   {
      /* Bare newline terminates a game */
      c = fgetc(fp);
      if (c == '\n') { break; }
      ungetc (c, fp);

      memset(wmv,0,sizeof(wmv));
      memset(bmv,0,sizeof(bmv));

      n = fscanf (fp, "%d. %s %s ", &moveno, wmv, bmv);
      if (strcmp(bmv,"1-0") == 0 || strcmp(bmv,"0-1") == 0 ||
	  strcmp(bmv,"1/2-1/2") == 0 || strcmp(bmv,"[Event") == 0 ||
 	  strcmp(bmv,"*") == 0)
        n = 2;

      if (n == 0 || n == 1) break;


      if (n > 2) {
       p = ValidateMove (wmv);
       if (!p)
       {
	 puts(header);
	 ShowBoard();
	 printf ("Illegal move %d. %s\n", moveno, wmv);


	 break;
       }
       MakeMove (white, &p->move);
       if (playercolor == white )
        BookBuilder (0, 0, result, white);
       strcpy (Game[GameCnt].SANmv, wmv);
       if (n == 3) {
        p = ValidateMove (bmv);
        if (!p)
        {
	  puts (header);
	  ShowBoard();
	  printf ("Illegal move %d. ... %s\n", moveno, bmv);


	  break;
        }
        MakeMove (black, &p->move);
        if (playercolor == black )
          BookBuilder (0, 0, result, black);
        strcpy (Game[GameCnt].SANmv, bmv);
      }
     }
      if (GameCnt/2+1 > BOOKDEPTH || n < 3 || feof(fp) ) break;
   }
   /* Read to end of game but don't parse */
   while (!feof(fp)) {
      fgets(s,100,fp);
      if (strncmp(s,"\n",1) == 0) break;
   }
   /* printf ("%d(%d)\n", GameCnt,BOOKDEPTH); */
   if (!feof(fp)) {
     if (ngames % 10 == 0) printf("%d\r",ngames);
     fflush(stdout);
     goto nextgame;
   }

   fclose (fp);
   BookBuilder(-2,-2, -2, -2);

   gettimeofday (&t2, &tz);
   et += (t2.tv_sec - t1.tv_sec);
   putchar('\n');

   /* Handle divide-by-zero problem */
   if (et == 0.0) { et = 1.0; };

   printf ("Time = %ld\n", et);
   printf("Games compiled: %d\n",ngames);
   printf("Games per second: %d\n",ngames/et);
   printf("Positions scanned: %d\n",newpos+existpos);
   printf("New & unique added: %d positions\n",newpos);
   printf("Duplicates not added: %d positions\n",existpos);
   return;
}
