//
//  SearchResults.c
//  JFresh
//
//  Created by Christian on 1/4/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>

#include "SearchResults.h"
#include "Debug.h"
#include "Thread.h"
#include "Uci.h"
#include "Log.h"

static SearchResults_t results;

void printCurrMove() {
    send("info depth %d seldepth %d ", results.currentDepth, results.selDepth);
    send("currmove ");
    printMove(results.currentMove);
    send(" currmovenumber %d\n", results.currentMoveNumber);
}

void printPv() {
    
    send("info depth %d ", results.currentDepth);
    send("time %lu ", msec_since(shared_results()->startTime));

    if (results.mate == 0) {
        send("score cp %d ", results.score);
    } else // forced mate
        send("score mate %d ", results.mate);
    
    send("pv ");
    
    for (int a = 0;results.pv[a] != NULL_MOVE;a++) {
        printMove(results.pv[a]);
        send(" ");
    }
    
    send("\n");
}

void printSearchInfo() {
    
    static uint64_t nodes = 0;
    
    send("info nodes %llu ", results.nodesSearched);
    send("nps %llu ", (results.nodesSearched - nodes) * (CLOCKS_PER_SEC / UPDATE_SPEED));
    send("hashfull %d", tt_permill_full());
    
    send("\n");
    
    nodes = results.nodesSearched;
}

void printBestMove() {
    
    send("bestmove ");
    printMove(results.pv[0]);
    send(" ponder ");
    printMove(results.pv[1]);
    send("\n");
}

SearchResults_t *shared_results() {
    return &results;
}