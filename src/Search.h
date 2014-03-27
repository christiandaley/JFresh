//
//  Search.h
//  JFresh
//
//  Created by Christian on 1/2/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#ifndef JFresh_Search_h
#define JFresh_Search_h

#include <stdint.h>
#include "SearchOptions.h"
#include "SearchResults.h"
#include "TT.h"

#define ASPIRATION_WINDOW 25
#define NULL_MOVE_REDUCTION 3
#define NULL_VER_DEPTH 5
#define NULL_VER_REDUCTION 1
#define DELTA_PRUNING_MARGINS 200
#define LMR_DEPTH 3
#define LMR_COUNT 4
#define LMR_REDUCTION 1
#define IID_DEPTH 6
#define IID_REDUCTION 2

typedef int NodeType;

const static NodeType NodeTypePV = 0;
const static NodeType NodeTypeNotPV = 1;

const static int FUTILITY_MARGINS[4] = {0, 300, 500, 900};

typedef struct {
    Move_t moves[MAX_NUM_MOVES];
    int count;
} Line_t;

void *search            (void *args);
int root_search         (Board_t *board, int depth, int alpha, int beta);
int full_search         (Board_t *board, int depth, int alpha, int beta, int useNullMove, NodeType ntype, Line_t *pv);
int mate_search         (Board_t *board, int depth, int alpha, int beta, Line_t *pv);
int quiescenceSearch    (Board_t *board, int alpha, int beta, int depth);
int internal_iterative  (Board_t *board, int depth, int alpha, int beta, int useNullMove, Move_t *list, int moveCount);
int verify_null         (Board_t *board);

#endif
