//
//  Perft.c
//  JFresh
//
//  Created by Christian on 12/25/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include <stdio.h>
#include "Board.h"
#include "Move.h"
#include "MoveGen.h"
#include "Undo.h"
#include "Perft.h"
#include "Debug.h"
#include "Attack.h"

uint64_t perft(Board_t *board, int depth) {
    
    TTable_t *tt = shared_TT();
    uint64_t index = HASH_INDEX(board->key, tt);
    
    if (tt->keys[index] == board->key && tt->flags[index] == depth)
        return tt->scores[index];
    
    int moveCount;
    TTableKey newHash;
    uint64_t result = 0;
    Move_t list[MAX_NUM_MOVES];
    Move_t move;
    
    moveCount = generateLegalMoves(board, list);
    
    if (depth == 0)
        return moveCount;
    
    
    for (int a = 0;a < moveCount;a++) {
        move = list[a];
        
        newHash = move_hash(board, move, board->key);
        makeMove(move, &board);
        board->key = newHash;
        
        result  += perft(board, depth - 1);
        
        undoMove(&board);

    }
    
    add_to_tt(board->key, depth, result, NULL_MOVE);
    
    
    return result;
}
