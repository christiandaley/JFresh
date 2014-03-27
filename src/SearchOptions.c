//
//  SearchOptions.c
//  JFresh
//
//  Created by Christian on 1/3/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#include "SearchOptions.h"
#include "Uci.h"
#include "Piece.h"
#include "Thread.h"

static SearchOptions_t search_options;

void init_options() {
    
    search_options.stop = 0;
    search_options.wTime = 0;
    search_options.bTime = 0;
    search_options.wInc = 0;
    search_options.bInc = 0;
    search_options.movesToGo = 0;
    search_options.searchMoves[0] = NULL_MOVE;
    search_options.mode = SearchModeInfinite;
    search_options.pondering = 0;
    search_options.info = 0;
    search_options.terminated = 0;
}

SearchOptions_t *shared_search_options() {
    return &search_options;
}

void setThinkTime() {
    
    if (search_options.mode == SearchModeTime) {
        if (search_options.info == 0) {
            if (shared_board()->turn == WHITE)
                search_options.info = search_options.wTime / 40;
            else
                search_options.info = search_options.bTime / 40;
            
        }
    }
    
}