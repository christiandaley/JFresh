//
//  SearchOptions.h
//  JFresh
//
//  Created by Christian on 1/3/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#ifndef JFresh_SearchOptions_h
#define JFresh_SearchOptions_h

#include "Move.h"

typedef uint8_t SearchMode;

const static SearchMode SearchModeNULL = 0;
const static SearchMode SearchModeInfinite = 1;
const static SearchMode SearchModeDepth = 2;
const static SearchMode SearchModeNodes = 3;
const static SearchMode SearchModeTime = 4;
const static SearchMode SearchModeMate = 5;

typedef struct {
    int wTime;
    int bTime;
    int wInc;
    int bInc;
    int movesToGo; // moves to next time control
    int stop;
    SearchMode mode;
    int pondering;
    uint64_t info;
    int terminated;
    
    Move_t searchMoves[MAX_NUM_MOVES]; // list of moves that should be searched
    
} SearchOptions_t;

void init_options                       ();
SearchOptions_t *shared_search_options  ();
void setThinkTime                       ();

#endif
