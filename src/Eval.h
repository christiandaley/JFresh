//
//  Eval.h
//  JFresh
//
//  Created by Christian on 1/3/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#ifndef JFresh_Eval_h
#define JFresh_Eval_h

#define MULTIPLIER(board, color) ((board->turn == color ? 1 : -1))
#define IS_WINNING_CHECKMATE(value) (((value) > 8000 ))
#define IS_LOSING_CHECKMATE(value) (((value) < -8000))
#define CHECKMATE 9001 // the value of checkmate is....OVER 9000!!!!!
#define DRAW 0
#define PAWN_TABLE_LENGTH 20011 // prime
#define MG 0
#define EG 1

#include "Board.h"

const static int pieceValues[] = {0, 100, 500, 300, 310, 950, 9001};

typedef uint64_t PTKey;

typedef struct {
    PTKey *keys;
    Bitboard *passed;
    int *scores;
    int length;
} PawnTable_t;

#define OPENING_MATERIAL (((double)(16 * pieceValues[PAWN] + 4 * pieceValues[ROOK] + 4 * pieceValues[KNIGHT] + 4 * pieceValues[BISHOP] + 2 * pieceValues[QUEEN] + 2 * pieceValues[KING])))
#define ENDING_MATERIAL  ((double)(2 * pieceValues[KING]))
#define SIMPL_BONUS 1
#define CONTEMPT_FACTOR -50
#define PAWN_TABLE_INDEX(hash, length) (((hash) % (length)))

void init_eval          ();
void dealloc_eval       ();
int eval                (Board_t *board);
int evalKings           (Board_t *board, int color);
int evalPawns           (Board_t *board, int color);
int evalKnights         (Board_t *board, int color);
int evalBishops         (Board_t *board, int color);
int evalRooks           (Board_t *board, int color);
int evalQueens          (Board_t *board, int color);
int evalDev             (Board_t *board, int color);
int evalPassedPawns     (Board_t *board, int color);
int evalKingTropism     (Board_t *board, int color);
int evalKingPawnTropism (Board_t *board, int color);
int isDraw              (Board_t *board);
PTKey pawn_hash         (Board_t *board, int color);

#endif
