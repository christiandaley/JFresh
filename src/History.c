//
//  History.c
//  JFresh
//
//  Created by Christian on 2/9/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#include "History.h"

static int history_moves[64][64];

void add_history(Move_t move, int depth) {
    if (MOVE_IS_QUIET(move))
        history_moves[MOVE_FROM(move)][MOVE_TO(move)] += depth * depth;
}

void clear_history() {
    for (int a = 0;a < 64;a++) {
        for (int b = 0;b < 64;b++) {
            history_moves[a][b] = NULL_MOVE;
        }
    }
}

int history_score(Move_t move) {
    
    return history_moves[MOVE_FROM(move)][MOVE_TO(move)];
}


