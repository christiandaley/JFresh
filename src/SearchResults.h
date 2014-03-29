//
//  SearchResults.h
//  JFresh
//
//  Created by Christian on 1/4/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#ifndef JFresh_SearchResults_h
#define JFresh_SearchResults_h

#include "Move.h"
#include "MoveGen.h"
#include <time.h>
#include <sys/time.h>

typedef struct {
    
    Move_t currentMove;
    Move_t ponderMove;
    Move_t pv[MAX_DEPTH];
    uint64_t nodesSearched;
    int currentMoveNumber;
    int score;
    int currentDepth;
    int selDepth;
    int mate;
    struct timeval startTime;
    
} SearchResults_t;

void printCurrMove              ();
void printPv                    ();
void printSearchInfo            ();
void printBestMove              ();
SearchResults_t *shared_results ();

#endif
