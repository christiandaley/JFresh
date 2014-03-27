//
//  Sort.h
//  JFresh
//
//  Created by Christian on 1/5/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#ifndef JFresh_MoveSort_h
#define JFresh_MoveSort_h

#include "Move.h"
#include "Board.h"
#include "Eval.h"

#define TTMOVE 0
#define WINNING_CAPTURE 1
#define KILLER_MOVE 2
#define HISTORY_MOVE 3
#define QUIET_MOVE 4
#define LOSING_CAPTURE 5

#define MVVLVA_SCORE(board, from, to) ((pieceValues[board->pieces[to]] * 100 - pieceValues[board->pieces[from]]))

int see                 (Board_t *board, Move_t capt);
void sort_moves         (Board_t *board, Move_t *list, Move_t tt_move, int length, int depth);
void sort_scores        (Move_t *list, int *scores, int length);
void sort_root_moves    (Move_t *list, int length);
void save_root_move     (int score, int num);
void clear_root_scores  ();

#endif
