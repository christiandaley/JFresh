//
//  TT.h
//  JFresh
//
//  Created by Christian on 1/4/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#ifndef JFresh_TT_h
#define JFresh_TT_h

#include <stdint.h>
#include "Board.h"
#include "Move.h"

#define TTSEED 2014
#define RANDS_LENGTH 793
#define PIECE_RAND(piece, color, square) ((rands[(square) * 12 + ((piece) - 1) + 6 * (color)]))
#define CASTLING_RAND(castling) ((rands[castling + 768]))
#define EN_PASSANT_RAND(epSquare) ((epSquare == NOT_ON_BOARD ? 0 : rands[FILE(epSquare) + 768 + 16]))
#define TURN_RAND(turn) ((turn == WHITE ? 0 : rands[792]))
#define ENTRY_DEPTH(flag) (((flag) >> 2))
#define ENTRY_TYPE(flag) (((flag) & 0x3))
#define HASH_FLAG(depth, type) ((((depth) << 2) | (type)))
#define HASH_INDEX(key, table) ((key % table->length))
#define SCORE_UPPER 1
#define SCORE_LOWER 2
#define SCORE_EXACT 3

typedef uint64_t TTableKey;
typedef uint8_t TTableFlag;
typedef int64_t TTableScore;
typedef uint64_t TTableIndex;

typedef struct {
    uint64_t length;
    uint8_t entrySize;
    uint64_t used;
    
    TTableKey *keys;
    TTableFlag *flags;
    TTableScore *scores;
    Move_t *moves;
    
} TTable_t;

uint64_t random_value       (int seed);
void init_tt                ();
void init_hash              ();
void clear_tt               ();
void dealloc_tt             ();
void add_to_tt              (TTableKey hash, TTableFlag flag, TTableScore score, Move_t bestMove);
int get_from_tt             (TTableKey hash, TTableFlag *flag, TTableScore *score, Move_t *ttMove);
int tt_permill_full         ();
void add_position           (TTableKey key);
void pop_position           ();
void reset_prev_hashes      ();
TTableKey *prev_positions   (int *length);
TTable_t *shared_TT         ();
TTableKey position_hash     (Board_t *board);
TTableKey move_hash         (Board_t *board, Move_t move, TTableKey current);
TTableKey turn_hash         (TTableKey current);

#endif
