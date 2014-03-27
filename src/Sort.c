//
//  Sort.c
//  JFresh
//
//  Created by Christian on 1/5/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#include <string.h>

#include "Sort.h"
#include "Eval.h"
#include "Move.h"
#include "Debug.h"
#include "Killer.h"
#include "History.h"
#include "Attack.h"
#include "Piece.h"
#include "Bitscan.h"

static int root_scores[MAX_NUM_MOVES];

int see(Board_t *board, Move_t capt) {
    int piece, from, to;
    from = MOVE_FROM(capt);
    to = MOVE_TO(capt);
    int value = pieceValues[board->pieces[to]];
    Move_t oppCapt;
    
    makeMove(capt, &board);
    
    Bitboard fromBoard = least_valuable_attacker(board, attacks_to_square(board, to, board->turn), board->turn, &piece);
    
    if (piece) {
        oppCapt = getMove(board, ls1b(fromBoard), to, '\0');
        value -= see(board, oppCapt);
    }
    
    undoMove(&board);
    
    return value;
}

void sort_moves(Board_t *board, Move_t *list, Move_t tt_move, int length, int depth) {
    int flags[MAX_NUM_MOVES] = {0};
    int scores[MAX_NUM_MOVES] = {0};
    int s, fail, temp_s, temp_f, attempts = 0;
    Move_t temp_m;
    Move_t move;
    
    for (int a = 0;a < length;a++) {
        move = list[a];
        if (move == tt_move)
            flags[a] = TTMOVE;
        else if (MOVE_IS_QUIET(move)) {
            flags[a] = QUIET_MOVE;
            
            if (is_killer(move, depth))
                flags[a] = KILLER_MOVE;
            else if ((scores[a] = history_score(move)))
                flags[a] = HISTORY_MOVE;
            
        } else {
            s = see(board, move);
            scores[a] = s;
            if (s > 0)
                flags[a] = WINNING_CAPTURE;
            else
                flags[a] = LOSING_CAPTURE;
            
        }
    }
    
    do {
        fail = 0;
        for (int a = 0;a < length - 1 - attempts;a++) {
            if (flags[a] > flags[a + 1] || (flags[a] == flags[a + 1] && scores[a] < scores[a + 1])) {
                temp_m = list[a];
                temp_s = scores[a];
                temp_f = flags[a];
                
                list[a] = list[a + 1];
                list[a + 1] = temp_m;
                
                scores[a] = scores[a + 1];
                scores[a + 1] = temp_s;
                
                flags[a] = flags[a + 1];
                flags[a + 1] = temp_f;
                
                fail = 1;
            }
        }
        
        attempts++;
        
    } while (fail);
}

void sort_scores(Move_t *list, int *scores, int length) {
    Move_t tempMove;
    int tempScore;
    int fail, attempts = 0;
    
    do {
        fail = 0;
        for (int a = 0;a < length - attempts - 1;a++) {
            if (scores[a] < scores[a + 1]) {
                tempMove = list[a];
                tempScore = scores[a];
                list[a] = list[a + 1];
                scores[a] = scores[a + 1];
                list[a + 1] = tempMove;
                scores[a + 1] = tempScore;
                fail = 1;
            }
        }
        
        attempts++;
        
    } while (fail);
    
    
    
}

void sort_root_moves(Move_t *list, int length) {
    Move_t move;
    int fail, score;
    int attempts = 0;
    
    do {
        fail = 0;
        for (int a = 0;a < length - 1 - attempts;a++) {
            if (root_scores[a] < root_scores[a + 1]) {
                fail = 1;
                move = list[a];
                list[a] = list[a + 1];
                list[a + 1] = move;
                
                score = root_scores[a];
                root_scores[a] = root_scores[a + 1];
                root_scores[a + 1] = score;
                
            }
        }
        
        attempts++;
        
    } while (fail);
}

void save_root_move(int score, int num) {
    root_scores[num] = score;
}

void clear_root_scores() {
    for (int a = 0;a < MAX_NUM_MOVES;a++)
        root_scores[a] = 0;
}