//
//  Killer.c
//  JFresh
//
//  Created by Christian on 2/8/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#include "Killer.h"
#include "MoveGen.h"

static Move_t killers[MAX_DEPTH][2];

void add_killer(Move_t move, int depth) {
    if (MOVE_IS_QUIET(move) == 0 || MOVE_IS_PROMOTION(move))
        return;
    
    if (killers[depth][0] != move && killers[depth][1] != move) {
        killers[depth][1] = killers[depth][0];
        killers[depth][0] = move;
    }
}

void clear_killers() {
    for (int a = 0;a < MAX_DEPTH;a++) {
        killers[a][0] = NULL_MOVE;
        killers[a][1] = NULL_MOVE;
    }
}

int is_killer(Move_t move, int depth) {
    return killers[depth][0] == move || killers[depth][1] == move;
}