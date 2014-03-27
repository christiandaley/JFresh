//
//  TT.c
//  JFresh
//
//  Created by Christian on 1/4/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "TT.h"
#include "UciOptions.h"
#include "SearchOptions.h"
#include "Debug.h"
#include "Piece.h"
#include "MagicNumberGen.h"
#include "SearchResults.h"

TTable_t ttable;
static uint64_t rands[RANDS_LENGTH];

uint64_t random_value(int seed) {
    
    const uint64_t n1 = 247126480216 * seed + 1;
    const uint64_t n2 = 138476281 * seed + 1;
    static uint64_t prev = 1;
    
    
    prev = (prev * n1 + n2);
    
    return prev;
}

void init_tt() {
    EngineOptions_t *options = shared_engine_options();
    
    int bytes = options->ttSize * 1024 * 1024;
    int entrySize = sizeof(TTableKey) + sizeof(TTableFlag) + sizeof(TTableScore) + sizeof(Move_t);
    
    ttable.length = bytes / entrySize;
    
    ttable.keys = calloc(ttable.length, sizeof(TTableKey));
    ttable.flags = calloc(ttable.length, sizeof(TTableFlag));
    ttable.scores = calloc(ttable.length, sizeof(TTableScore));
    ttable.moves = calloc(ttable.length, sizeof(Move_t));
}

void init_hash() {
    int fail;
    do {
        fail = 0;
        for (int a = 0;a < RANDS_LENGTH;a++)
            rands[a] = random_value(TTSEED);
        
        for (int a = 0;a < RANDS_LENGTH - 1;a++) {
            for (int b = a + 1;b < RANDS_LENGTH;b++) {
                if (rands[a] == rands[b] || rands[a] == 0 || rands[b] == 0)
                    fail = 1;
            }
        }
        
        
    } while (fail);
}

void clear_tt() {
        
    for (int a = 0;a < ttable.length;a++) {
        ttable.keys[a] = 0;
        ttable.scores[a] = 0;
        ttable.flags[a] = 0;
    }
    
    ttable.used = 0;
}

void dealloc_tt() {
    free(ttable.keys);
    free(ttable.flags);
    free(ttable.scores);
    free(ttable.moves);
}

void add_to_tt(TTableKey hash, TTableFlag flag, TTableScore score, Move_t bestMove) {
    uint64_t index = HASH_INDEX(hash, shared_TT());
    
    if (ENTRY_DEPTH(ttable.flags[index]) <= ENTRY_DEPTH(flag) || ttable.keys[index] == 0) {
        ttable.keys[index] = hash;
        ttable.flags[index] = flag;
        ttable.scores[index] = score;
        ttable.moves[index] = bestMove;
        ttable.used++;
    }
}

int get_from_tt(TTableKey hash, TTableFlag *flag, TTableScore *score, Move_t *ttMove) {
    uint64_t index = HASH_INDEX(hash, shared_TT());
    
    if (ttable.keys[index] == 0)
        return 0;
    
    if (ttable.keys[index] != hash)
        return 0;
    
    *flag = ttable.flags[index];
    *score = ttable.scores[index];
    *ttMove = ttable.moves[index];
    return 1;
}

int tt_permill_full() {
    return (int)(ttable.used * 1000000 / ttable.length);
}

TTable_t *shared_TT() {
    
    return &ttable;
}

TTableKey position_hash(Board_t *board) {    
    TTableKey hash = 0;
    for (int square = 0;square < 64;square++) {
        if (board->pieces[square]) {
            hash ^= PIECE_RAND(board->pieces[square], board->colors[square], square);
        }
    }
    
    hash ^= CASTLING_RAND(board->castling);
    hash ^= EN_PASSANT_RAND(board->epSquare);
    
    hash ^= TURN_RAND(board->turn);
    
    return hash;
}

TTableKey move_hash(Board_t *board, Move_t move, TTableKey current) {
    
    int from, to;
    uint8_t castling = board->castling;
    
    from = MOVE_FROM(move);
    to = MOVE_TO(move);
        
    current ^= PIECE_RAND(board->pieces[from], board->turn, from);
    current ^= PIECE_RAND(board->pieces[from], board->turn, to);
    
    current ^= EN_PASSANT_RAND(board->epSquare);
    current ^= CASTLING_RAND(board->castling);
    
    if (MOVE_IS_CAPTURE(move)) {
        current ^= PIECE_RAND(board->pieces[to], ENEMY(board->turn), to);
        
    } else if (MOVE_IS_EN_PASSANT(move)) {
        current ^= PIECE_RAND(PAWN, ENEMY(board->turn), board->epSquare + FORWARD(ENEMY(board->turn)));
    } else if (MOVE_IS_CASTLE_KINGSIDE(move)) {
        current ^= PIECE_RAND(ROOK, board->turn, from + 3);
        current ^= PIECE_RAND(ROOK, board->turn, from + 1);
        
    } else if (MOVE_IS_CASTLE_QUEENSIDE(move)) {
        current ^= PIECE_RAND(ROOK, board->turn, from - 4);
        current ^= PIECE_RAND(ROOK, board->turn, from - 1);
    }
    
    if (MOVE_IS_PROMOTION(move)) {
        current ^= PIECE_RAND(PAWN, board->turn, to);
        if (MOVE_IS_PROMOTE_QUEEN(move)) {
            current ^= PIECE_RAND(QUEEN, board->turn, to);
            
        } else if (MOVE_IS_PROMOTE_ROOK(move)) {
            current ^= PIECE_RAND(ROOK, board->turn, to);
            
        } else if (MOVE_IS_PROMOTE_BISHOP(move)) {
            current ^= PIECE_RAND(BISHOP, board->turn, to);
            
        } else if (MOVE_IS_PROMOTE_KNIGHT(move)) {
            current ^= PIECE_RAND(KNIGHT, board->turn, to);
        }
    }
    
    if (board->pieces[from] == PAWN) {
        if (to - from == 16)
            current ^= EN_PASSANT_RAND(to - 8);
        else if (to - from == -16)
            current ^= EN_PASSANT_RAND(to + 8);
    }
    
    if (board->pieces[from] == ROOK || board->pieces[to] == ROOK) {
        if (from == 0 || to == 0)
            castling &= ~WHITE_CASTLE_QUEENSIDE;
        if (from == 7 || to == 7)
            castling &= ~WHITE_CASTLE_KINGSIDE;
        if (from == 56 || to == 56)
            castling &= ~BLACK_CASTLE_QUEENSIDE;
        if (from == 63 || to == 63)
            castling &= ~BLACK_CASTLE_KINGSIDE;
    }
    
    if (board->pieces[from] == KING) {
        if (board->turn == WHITE)
            castling &= 0xC;
        else
            castling &= 0x3;
    }
            
    current ^= CASTLING_RAND(castling);
    
    current ^= TURN_RAND(BLACK);
    
    return current;
}

TTableKey turn_hash(TTableKey current) {
    return current ^ TURN_RAND(BLACK);
}