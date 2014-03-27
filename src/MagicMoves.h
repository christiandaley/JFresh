//
//  MagicMoves.h
//  JFresh
//
//  Created by Christian on 12/27/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_MagicMoves_h
#define JFresh_MagicMoves_h

#include "Bitboard.h"

extern Bitboard magicMovesRook[64][4096];
extern Bitboard magicMovesBishop[64][512];

void init_magicMoves();

#endif
