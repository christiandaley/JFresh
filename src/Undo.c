//
//  Undo.c
//  JFresh
//
//  Created by Christian on 12/30/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include <string.h>
#include "Undo.h"
#include "TT.h"

#define STACK_SIZE 200

Board_t *boardStack[STACK_SIZE];
static int depth = 0;

void init_undo(Board_t *orig) {
    for (int a = 1;a < STACK_SIZE;a++) {
        boardStack[a] = malloc(BoardSize);
    }
    
    boardStack[0] = orig;
}

void free_undo() {
    for (int a = 1;a < STACK_SIZE;a++) {
        free(boardStack[a]);
    }
    
}

void saveBoard(Board_t **ptr) {
    depth++;
    memcpy(boardStack[depth], *ptr, BoardSize);
    *ptr = boardStack[depth];
}

void undoMove(Board_t **ptr) {
    depth--;
    *ptr = boardStack[depth];
}

void reset_undo() {
    depth = 0;
}

Board_t **undo_stack(int *length) {
    *length = depth;
    
    return boardStack;
}
