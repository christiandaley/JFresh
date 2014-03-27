//
//  Bitmask.h
//  JFresh
//
//  Created by Christian on 12/27/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_Bitmask_h
#define JFresh_Bitmask_h

#include "Bitboard.h"

#define MASK(pieceType, square) ((pieceType == ROOK ? rookMasks[square] : bishopMasks[square]))
#define OCCUPANCY(pieceType, square, var) ((pieceType == ROOK ? occupancyVariationsRook[square][var] : occupancyVariationsBishop[square][var]))
#define ATTACKS(pieceType, square, var) ((pieceType == ROOK ? rookAttacks[square][var] : bishopAttacks[square][var]))

extern Bitboard FileMasks[8];
extern Bitboard RankMasks[8];
extern Bitboard ABCDMask;
extern Bitboard EFGHMask;

extern Bitboard bishopMasks[64];
extern Bitboard rookMasks[64];

extern Bitboard occupancyVariationsRook[64][4096];
extern Bitboard occupancyVariationsBishop[64][512];

extern Bitboard bishopAttacks[64][512];
extern Bitboard rookAttacks[64][4096];

extern Bitboard knightAttacks[64];
extern Bitboard pawnAttacks[2][64];
extern Bitboard kingAttacks[64];

extern Bitboard moveBoards[64][64];

void init_bitmasks();

#endif
