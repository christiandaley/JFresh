//
//  Move.h
//  JFresh
//
//  Created by Christian on 12/6/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_Move_h
#define JFresh_Move_h

#define MOVE_FROM(move) (((move >> 6) & 0x3F))
#define MOVE_TO(move) ((move & 0x3F))
#define MOVE_FLAGS(move) ((move >> 12))
#define MOVE_IS_CAPTURE(move) ((((MOVE_FLAGS(move) & FlagCapture) != 0) && (MOVE_FLAGS(move) != FlagEnPassant)))
#define MOVE_IS_CASTLE_KINGSIDE(move) ((MOVE_FLAGS(move) == FlagCastleKingside))
#define MOVE_IS_CASTLE_QUEENSIDE(move) ((MOVE_FLAGS(move) == FlagCastleQueenside))
#define MOVE_IS_EN_PASSANT(move) ((MOVE_FLAGS(move) == FlagEnPassant))
#define MOVE_IS_PROMOTION(move) (((MOVE_FLAGS(move) & 8) != 0))
#define MOVE_IS_PROMOTE_QUEEN(move) (((MOVE_FLAGS(move) & ~FlagCapture) == FlagPromoteQueen))
#define MOVE_IS_PROMOTE_ROOK(move) (((MOVE_FLAGS(move) & ~FlagCapture) == FlagPromoteRook))
#define MOVE_IS_PROMOTE_BISHOP(move) (((MOVE_FLAGS(move) & ~FlagCapture) == FlagPromoteBishop))
#define MOVE_IS_PROMOTE_KNIGHT(move) (((MOVE_FLAGS(move) & ~FlagCapture) == FlagPromoteKnight))
#define MOVE_IS_QUIET(move) ((!MOVE_IS_CAPTURE(move) && !MOVE_IS_EN_PASSANT(move)))

#define MAX_NUM_MOVES 200
#define NULL_MOVE 0

#include <stdint.h>
#include "Board.h"

typedef uint8_t Flag_t;
typedef uint16_t Move_t;

extern const uint8_t FlagNone;
extern const uint8_t FlagPawn;
extern const uint8_t FlagCastleKingside;
extern const uint8_t FlagCastleQueenside;
extern const uint8_t FlagCapture;
extern const uint8_t FlagEnPassant;
extern const uint8_t FlagPromoteQueen;
extern const uint8_t FlagPromoteRook;
extern const uint8_t FlagPromoteBishop;
extern const uint8_t FlagPromoteKnight;
extern const uint8_t FlagCheck;

void makeMove       (Move_t move, Board_t **ptr);
void make_null_move (Board_t **ptr);
Move_t getMove      (Board_t *board, int from, int to, char promote); // gets a move for the given board, based on from/to squares and and promotion info

#endif
