/* GNU Chess 5.0 - common.h - common symbolic definitions
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

typedef unsigned long long BitBoard;
typedef unsigned long long HashType;
typedef unsigned long KeyType;

typedef struct 
{
   BitBoard b[2][7];
   BitBoard friend[2];
   BitBoard blocker;
   BitBoard blockerr90;
   BitBoard blockerr45;
   BitBoard blockerr315;
   short ep;
   short flag;
   short side;
   short material[2];
   short pmaterial[2];
   short castled[2];
   short king[2];
} Board; 

typedef struct
{
   int move;
   int score; 
} leaf;

typedef struct
{
   int move;
   short epsq; 
   short bflag;
   short Game50;
   short mvboard;
   float et;
   HashType hashkey;
   HashType phashkey;
   char SANmv[8];
} GameRec;

typedef struct
{
   KeyType key; 
   int move;
   int score;
   short flag;
   short depth;
} HashSlot;   

typedef struct
{
   KeyType pkey;  
   BitBoard passed;
   BitBoard weaked;
   short score;
   short phase;
} PawnSlot;


/*  MACRO definitions */

#define MAX(a,b)     ((a) > (b) ? (a) : (b))
#define MIN(a,b)     ((a) < (b) ? (a) : (b))
#define SET(a,b)     (a |= b)
#define CLEAR(a,b)   (a &= ~b)
#define	DRAWSCORE    (computerplays == board.side ? (opprating - myrating) / 4 : (myrating - opprating ) / 4)
#define MATERIAL     (board.material[side] - board.material[1^side])
#define PHASE	     (8 - (board.material[white]+board.material[black]) / 1150)
#define KEY(a)	     (a >> 32) 

/*  Attack MACROS */

#define BishopAttack(sq) \
	(Bishop45Atak[sq][(board.blockerr45 >> Shift45[sq]) & Mask45[sq]] | \
	 Bishop315Atak[sq][(board.blockerr315 >> Shift315[sq]) & Mask315[sq]])
#define RookAttack(sq)	\
	(Rook00Atak[sq][(board.blocker >> Shift00[sq]) & 0xFF] | \
         Rook90Atak[sq][(board.blockerr90 >> Shift90[sq]) & 0xFF])
#define QueenAttack(sq)	\
	(BishopAttack(sq) | RookAttack(sq))


/*  Some bit macros  */
#define SETBIT(b,i) ((b) |= BitPosArray[i])
#define CLEARBIT(b,i) ((b) &= NotBitPosArray[i])

#define RANK(i) ((i) >> 3)
#define FILE(i) ((i) & 7)
#define trailz(b) (leadz ((b) & ((~b) + 1)))

#define PROMOTEPIECE(a) ((a >> 12) & 0x0007)
#define CAPTUREPIECE(a) ((a >> 15) & 0x0007)
#define TOSQ(a)         ((a) & 0x003F)
#define FROMSQ(a)       ((a >> 6) & 0x003F)
#define MOVE(a,b)       (((a) << 6) | (b))

#define white  0
#define black  1
#define false  0
#define true   1
#define ks 0
#define qs 1
#define INFINITY  32767
#define MATE	  32767
#define MATESCORE(a)	((a) > MATE-255  || (a) < -MATE+255)

/* constants for Board */
#define WKINGCASTLE   0x0001
#define WQUEENCASTLE  0x0002
#define BKINGCASTLE   0x0004
#define BQUEENCASTLE  0x0008
#define WCASTLE	      (WKINGCASTLE | WQUEENCASTLE)
#define BCASTLE	      (BKINGCASTLE | BQUEENCASTLE)

/* Material values */
#define ValueP   100	
#define ValueN   350
#define ValueB   350
#define ValueR   550
#define ValueQ   1100
#define ValueK   2000

/* constants for move description */
#define KNIGHTPRM     0x00002000
#define BISHOPPRM     0x00003000 
#define ROOKPRM       0x00004000
#define QUEENPRM      0x00005000
#define PROMOTION     0x00007000
#define PAWNCAP       0x00008000
#define KNIGHTCAP     0x00010000 
#define BISHOPCAP     0x00018000
#define ROOKCAP       0x00020000 
#define QUEENCAP      0x00028000 
#define CAPTURE       0x00038000 
#define NULLMOVE      0x00100000 
#define CASTLING      0x00200000 
#define ENPASSANT     0x00400000
#define MOVEMASK      (CASTLING | ENPASSANT | PROMOTION | 0x0FFF)

/*  Some special BitBoards  */
#define NULLBITBOARD  (0x0000000000000000ULL)
#define WHITESQUARES  (0x55AA55AA55AA55AAULL)
#define BLACKSQUARES  (0xAA55AA55AA55AA55ULL)
#define CENTRESQUARES (0x0000001818000000ULL)
#define COMPUTERHALF  (0xFFFFFFFF00000000ULL)
#define OPPONENTHALF  (0x00000000FFFFFFFFULL)

/*  Game flags */
#define QUIT    0x0001
#define TESTT   0x0002
#define THINK   0x0004
#define MANUAL  0x0008
#define TIMEOUT 0x0010
#define DEBUGG  0x0020
#define ENDED   0x0040
#define USEHASH 0x0080
#define SOLVE   0x0100
#define USENULL 0x0200
#define XBOARD  0x0400
#define TIMECTL 0x0800
#define POST    0x1000

/*  Node types  */ 
#define PV  0
#define ALL 1
#define CUT 2

/*  Transposition table flags  */
#define EXACTSCORE  1
#define LOWERBOUND  2
#define UPPERBOUND  3
#define POORDRAFT   4
#define QUIESCENT   5
#define STICKY      8

/*  Book modes */
#define BOOKOFF 0
#define BOOKRAND 1
#define BOOKBEST 2
#define BOOKWORST 3
#define BOOKPREFER 4

/*  The various phases during move selection  */
#define PICKHASH    1
#define PICKGEN1    2
#define PICKCAPT    3
#define PICKKILL1   4
#define PICKKILL2   5
#define PICKGEN2    6
#define PICKHIST    7
#define PICKREST    8
#define PICKCOUNTER 9

#define MAXTREEDEPTH  2000
#define MAXPLYDEPTH   65
#define MAXGAMEDEPTH  600


#define HASHSLOTS 262144*6
#define PAWNSLOTS 65536*6
//#define HASHSLOTS 32668/48
//#define PAWNSLOTS 32668/48

//#define HASHSLOTS  4000
//#define PAWNSLOTS  2000

#define DEPTH	12

extern short distance[64][64];
extern short taxicab[64][64];
extern short lzArray[65536];
extern short Shift00[64];
extern short Shift90[64];
extern short Shift45[64];
extern short Shift315[64];
extern BitBoard DistMap[64][8];
extern BitBoard BitPosArray[64];
extern BitBoard NotBitPosArray[64];
extern BitBoard MoveArray[8][64];
extern BitBoard Ray[64][8];
extern BitBoard FromToRay[64][64];
extern BitBoard RankBit[8];
extern BitBoard FileBit[8];
extern BitBoard Ataks[2][7];
extern BitBoard PassedPawnMask[2][64];
extern BitBoard IsolaniMask[8];
extern BitBoard SquarePawnMask[2][64];
extern BitBoard Rook00Atak[64][256]; 
extern BitBoard Rook90Atak[64][256]; 
extern BitBoard Bishop45Atak[64][256];
extern BitBoard Bishop315Atak[64][256];
extern BitBoard pinned;
extern BitBoard rings[4];
extern BitBoard stonewall[2];
extern BitBoard pieces[2];
extern BitBoard mask_kr_trapped_w[3];
extern BitBoard mask_kr_trapped_b[3];
extern BitBoard mask_qr_trapped_w[3];
extern BitBoard mask_qr_trapped_b[3];
extern BitBoard boardhalf[2];
extern BitBoard boardside[2];
extern short directions[64][64];
extern short BitCount[65536];
extern leaf Tree[MAXTREEDEPTH];
extern leaf *TreePtr[MAXPLYDEPTH];
extern int RootPV;
extern GameRec Game[MAXGAMEDEPTH];
extern short GameCnt;
extern short computer;
extern unsigned int flags;
extern Board board;
extern short cboard[64];
extern short Mvboard[64];
extern HashType hashcode[2][7][64];
extern HashType ephash[64];
extern HashType WKCastlehash;
extern HashType WQCastlehash;
extern HashType BKCastlehash;
extern HashType BQCastlehash;
extern HashType Sidehash;
extern HashType HashKey;
extern HashType PawnHashKey;
extern HashSlot *HashTab[2];
extern PawnSlot *PawnTab[2];
extern short Idepth;
extern short SxDec;
extern short Game50;
extern int lazyscore[2];
extern int maxposnscore[2];
extern int rootscore;
extern int lastrootscore;
extern unsigned long GenCnt;
extern unsigned long NodeCnt;
extern unsigned long QuiesCnt;
extern unsigned long EvalCnt;
extern unsigned long EvalCall;
extern unsigned long ChkExtCnt;
extern unsigned long OneRepCnt;
extern unsigned long RcpExtCnt;
extern unsigned long PawnExtCnt;
extern unsigned long HorzExtCnt;
extern unsigned long ThrtExtCnt;
extern unsigned long KingExtCnt;
extern unsigned long NullCutCnt;
extern unsigned long FutlCutCnt;
extern unsigned long RazrCutCnt;
extern unsigned long TotalGetHashCnt;
extern unsigned long GoodGetHashCnt;
extern unsigned long TotalPutHashCnt;
extern unsigned long CollHashCnt;
extern unsigned long TotalPawnHashCnt;
extern unsigned long GoodPawnHashCnt;
extern unsigned long RepeatCnt;
extern unsigned HashSize;
extern unsigned long TTHashMask;
extern unsigned long PHashMask;
extern short slider[8];
extern short Value[7];
extern char SANmv[10];
extern unsigned long history[2][4096];
extern int killer1[MAXPLYDEPTH];
extern int killer2[MAXPLYDEPTH];
extern short ChkCnt[MAXPLYDEPTH];
extern short ThrtCnt[MAXPLYDEPTH];
extern char id[32];
extern char solution[64];
extern double et;
extern float SearchTime;
extern short SearchDepth;
extern short MoveLimit[2];
extern float TimeLimit[2];
extern short TCMove;
extern short TCinc;
extern float TCTime;
extern short hunged[2];
extern short phase;
extern int Hashmv[MAXPLYDEPTH];
extern short DebugPly;
extern short DebugDepth;
extern long DebugNode;
extern int Debugmv[MAXPLYDEPTH];
extern short Debugmvl;
extern short Debugline;
extern short RootPieces;
extern short RootPawns;
extern short RootMaterial;
extern short RootAlpha;
extern short RootBeta;
extern short pickphase[MAXPLYDEPTH];
extern short InChk[MAXPLYDEPTH];
extern short KingThrt[2][MAXPLYDEPTH];
extern short threatmv;
extern short threatply;
extern short KingSafety[2];
extern short pscore[64];

extern short bookmode;
extern short bookfirstlast;

extern short range[8];
extern short ptype[2];
extern char algbr[64][3];
extern char algbrfile[9];
extern char algbrrank[9];
extern char notation[8];
extern char lnotation[8];
extern short r90[64];
extern short r45[64];
extern short r315[64];
extern short Mask45[64];
extern short Mask315[64];
extern FILE *ofp;
extern short myrating, opprating, suddendeath, TCinc;
extern char name[50];
extern short computerplays;
extern short wasbookmove;
extern int nmovesfrombook;
extern float maxtime;
extern short n; 		/* Last mobility returned by CTL */
extern short ExchCnt[2];
extern int newpos, existpos;		/* For book statistics */
extern short bookloaded;

enum Piece { empty, pawn, knight, bishop, rook, queen, king, bpawn };

enum Square { A1, B1, C1, D1, E1, F1, G1, H1,
	      A2, B2, C2, D2, E2, F2, G2, H2,
	      A3, B3, C3, D3, E3, F3, G3, H3,
	      A4, B4, C4, D4, E4, F4, G4, H4,
	      A5, B5, C5, D5, E5, F5, G5, H5,
	      A6, B6, C6, D6, E6, F6, G6, H6,
	      A7, B7, C7, D7, E7, F7, G7, H7,
	      A8, B8, C8, D8, E8, F8, G8, H8 };

enum File { A_FILE, B_FILE, C_FILE, D_FILE, E_FILE, F_FILE, G_FILE, H_FILE };

/****************************************************************************
 *
 *  The various function prototypes.  They are group into the *.c files
 *  in which they are defined.
 *
 ****************************************************************************/

/*  The initialization routines  */
void Initialize (void);
void InitLzArray (void);
void InitBitPosArray (void);
void InitMoveArray (void);
void InitRay (void);
void InitFromToRay (void);
void InitRankFileBit (void);
void InitBitCount (void);
void InitPassedPawnMask (void);
void InitIsolaniMask (void);
void InitSquarePawnMask (void);
void InitRandomMasks (void);
void InitRotAtak (void);
void InitDistance (void);
void InitVars (void);
void InitHashCode (void);
void InitHashTable (void);
void NewPosition (void);
void InitFICS (void);

/*  The book routines  */
void MakeBinBook (char *, short);
short GetNextMove (FILE *, char *);
int BookQuery (void);
int GenBook (void);
void BookBuilder (short, int, short, short);

/*  The move generation routines  */
void GenMoves (short);
void GenCaptures (short);
void GenNonCaptures (short);
void GenCheckEscapes (short);
void BitToMove (short, BitBoard);
void FilterIllegalMoves (short);

/*  The move routines  */
void MakeMove (short, int *);
void UnmakeMove (short, int *);
void MakeNullMove (short);
void UnmakeNullMove (short);
void SANMove (int, short);
leaf *ValidateMove (char *);
leaf *IsInMoveList (short, short, short, char);
short IsLegalMove (int);
char *AlgbrMove (int);

/*  The attack routines  */
short SqAtakd (short, short);
void GenAtaks (void);
BitBoard AttackTo (short, short);
BitBoard AttackXTo (short, short);
BitBoard AttackFrom (short, short, short);
BitBoard AttackXFrom (short, short);
short PinnedOnKing (short, short);
void FindPins (BitBoard *);
short MateScan (short);

/*  The swap routines  */
short SwapOff (int);
void AddXrayPiece (short, short, short, BitBoard *, BitBoard *);

/*  The EPD routines  */
short ReadEPDFile (char *, short);
void ParseEPD (char *);
void LoadEPD (char *);
void SaveEPD (char *);

/*  The command routines */
void InputCmd (void);
void ShowCmd (char *);
void BookCmd (char *);
void TestCmd (char *);

/*  Some utility routines  */
short leadz (BitBoard);
short nbits (BitBoard);
void UpdateFriends (void);
void UpdateCBoard (void);
void UpdateMvboard (void);
void EndSearch (int);
short ValidateBoard (void);

/*  PGN routines  */
void PGNSaveToFile (char *, char *);
void PGNReadFromFile (char *);
void BookPGNReadFromFile (char *);

/*  The hash routines  */
void CalcHashKey (void);
void ShowHashKey (HashType);

/*  The evaluation routines  */
int ScoreP (short);
int ScoreN (short);
int ScoreB (short);
int ScoreR (short);
int ScoreQ (short);
int ScoreK (short);
int ScoreDev (short);
int Evaluate (int, int);
short EvaluateDraw (void);

/*  Hung routines  */
short EvalHung (short);

/*  The search routines  */
void Iterate (void);
int Search (short, short, int, int, short);
int SearchRoot (short, int, int);
int Quiesce (short, int, int);
void pick (leaf *, short);
short Repeat (void);
void ShowLine (int, int, char);
void GetElapsed (void);

/*  The transposition table routies */
void TTPut (short, short, short, int, int, int, int);
short TTGet (short, short, short, int, int, int *, int *);
short TTGetPV (short, short, int, int *);
void TTClear (void);
void PTClear (void);

/*  Sorting routines  */
void SortCaptures (short);
void SortMoves (short);
void SortRoot (void);
short PhasePick (leaf **, short);
short PhasePick1 (leaf **, short);

/*  Some output routines */
void ShowMoveList (short);
void ShowSmallBoard (void);
void ShowBoard (void);
void ShowBitBoard (BitBoard *);
void ShowCBoard (void);
void ShowMvboard (void);

/*  Random numbers routines */
unsigned int long Rand32 (void);
HashType Rand64 (void);

/*  Solver routines  */
void Solve (char *);

/*  Test routines  */
void TestMoveGenSpeed (void);
void TestNonCaptureGenSpeed (void);
void TestCaptureGenSpeed (void);
void TestMoveList (void);
void TestNonCaptureList (void);
void TestCaptureList (void);
void TestEvalSpeed (void);
void TestEval (void);

/*  Miscellaneous routines  */
void ShowVersion (void);
void ShowHelp (void);

/* Player database */
void DBSortPlayer (char *style);
void DBListPlayer (char *style); 	
void DBReadPlayer (void);	
void DBWritePlayer (void);
int DBSearchPlayer (char *player);
void DBUpdatePlayer (char *player, char *resultstr);
void DBTest (void);
