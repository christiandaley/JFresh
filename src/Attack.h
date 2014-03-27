//
//  Attack.h
//  JFresh
//
//  Created by Christian on 12/30/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_Attack_h
#define JFresh_Attack_h

#include "Board.h"
#include "Move.h"

extern int DeltaIncs[64][64];

void init_deltaIncs();

int isCheck                      (Board_t *board, int color);
int isAttacked                   (Board_t *board, int square, int color);
Bitboard attack_board            (Board_t *board, int color);
int attack_count                 (Board_t *board, int color, int target);

int isLegal                      (Board_t *board, Move_t move);
int isPinned                     (Board_t *board, int square, int color);
Bitboard least_valuable_attacker (Board_t *board, Bitboard attacks, int byColor, int *piece);
Bitboard attacks_to_square       (Board_t *board, int square, int byColor);

#endif
