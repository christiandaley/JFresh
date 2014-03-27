//
//  MoveGen.h
//  JFresh
//
//  Created by Christian on 12/8/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_MoveGen_h
#define JFresh_MoveGen_h

#include "Move.h"

#define MAX_DEPTH 100

int generateLegalMoves          (Board_t *board, Move_t *moves); // generate all legal moves at a specfic depth
int generatePsuedoLegalMoves    (Board_t *board, Move_t *moves);
int generateKingMoves           (Board_t *board, Move_t *moves);
int generateCastles             (Board_t *board, Move_t *moves);
int generateSlidingMoves        (Board_t *board, Move_t *moves);
int generatePawnMoves           (Board_t *board, Move_t *moves);
int generatePawnCaptures        (Board_t *board, Move_t *moves);
int generateKnightMoves         (Board_t *board, Move_t *moves);
int generateMovesFromBitboard   (Board_t *board, Bitboard bitboard, int from, int isPromotion, Move_t *moves);
int generateEvasions            (Board_t *board, Move_t *moves);
int generateCaptures            (Board_t *board, Move_t *moves);


#endif
