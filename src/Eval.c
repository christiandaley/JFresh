//
//  Eval.c
//  JFresh
//
//  Created by Christian on 1/3/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//


#include "Eval.h"
#include "Piece.h"
#include "UciOptions.h"
#include "Attack.h"
#include "TT.h"
#include "Bitscan.h"
#include "Bitmask.h"
#include "Debug.h"
#include "MagicNumbers.h"
#include "MagicMoves.h"
#include "Undo.h"
#include "Attack.h"

PawnTable_t ptable;

static Bitboard passed_pawns;
static uint64_t pawnHashRands[128];
static double mg_weight;
static double eg_weight;

static const int isolated_pawn[2] = {18, 21};
static const int pawn_weak[2] = {8, 18};
static const int pawn_doubled[2] = {5, 6};
static const int pawn_duo[2] = {4, 8};
static const int passed_pawn[2][2][8] = {
    {{ 0,   0,   0,  20,  80, 120, 200, 0 }, // middlegame white
        { 0, 200, 120,  80,  20,   0,   0, 0 }}, // middlegame black
    {{ 0,   2,   2,  24,  95, 145, 234, 0 }, // endgame white
        { 0, 234, 145,  95,  24,   2,   2, 0 }} // endgame black
};

static const int supported_passed_pawn[2] = {20, 65};
static const int rook_behind_passed_pawn[2] = {20, 60};
static const int blocked_passed_pawn[2] = {95, 75};
static const int passed_pawn_db[8] = {0, 0, 0, 2, 7, 19, 31, 0};
static const int pawn_shield[2] = {8, 4};
static const int no_pawn_shield[2] = {10, 5};
static const int has_castle[2] = {15, 0};
static const int can_castle[2] = {5, 0};
static const int rook_open_file[2] = {35, 20};
static const int rook_half_open_file[2] = {10, 10};
static const int rook_on_7[2] = {25, 35};
static const int blocked_c_pawn = 12;
static const int no_castle = 20;
//static const int cant_castle = 20;
static const int undeveloped_piece = 12;
//static const int attacker_near_king[2] = {5, 3};
static const int blocked_rook[2] = {8, 4};
static const int double_bishops[2] = {12, 8};
static const int same_color_bishop[2] = {4, 0};

static const int square_colors[64] = {
    0, 1, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 1, 0, 1, 0,
    0, 1, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 1, 0, 1, 0,
    0, 1, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 1, 0, 1, 0,
    0, 1, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 1, 0, 1, 0
};

static const int knight_outpost[2][64] = {
    // white
    {   0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 4, 4, 4, 4, 1, 0,
        0, 2, 6, 8, 8, 6, 2, 0,
        0, 1, 4, 4, 4, 4, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0 },
    // black
    {   0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 4, 4, 4, 4, 1, 0,
        0, 2, 6, 8, 8, 6, 2, 0,
        0, 1, 4, 4, 4, 4, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0 }
    
};

static const int bishop_outpost[2][64] = {
    // white
    { 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 2, 2, 2, 2, 1, 0,
        0, 3, 5, 5, 5, 5, 3, 0,
        0, 1, 3, 3, 3, 3, 1, 0,
        0, 0, 1, 1, 1, 1, 0, 0,
        -1, 0, 0, 0, 0, 0, 0,-1,
        0, 0, 0, 0, 0, 0, 0, 0 },
    
    // black
    { 0, 0, 0, 0, 0, 0, 0, 0,
        -1, 0, 0, 0, 0, 0, 0,-1,
        0, 0, 1, 1, 1, 1, 0, 0,
        0, 1, 3, 3, 3, 3, 1, 0,
        0, 3, 5, 5, 5, 5, 3, 0,
        0, 1, 2, 2, 2, 2, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0 }
};

static const int pawn_square_values[2][2][64] = {
    {
        // white middlegame
        { 0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0, -12, -12,   0,   0,   0,
            1,   1,   1,  10,  10,   1,   1,   1,
            3,   3,   3,  13,  13,   3,   3,   3,
            6,   6,   6,  16,  16,   6,   6,   6,
            10,  10,  10,  30,  30,  10,  10,  10,
            66,  66,  66,  66,  66,  66,  66,  66,
            0,   0,   0,   0,   0,   0,   0,   0 },
        
        // black middlegame
        { 0,   0,   0,   0,   0,   0,   0,   0,
            66,  66,  66,  66,  66,  66,  66,  66,
            10,  10,  10,  30,  30,  10,  10,  10,
            6,   6,   6,  16,  16,   6,   6,   6,
            3,   3,   3,  13,  13,   3,   3,   3,
            1,   1,   1,  10,  10,   1,   1,   1,
            0,   0,   0, -12, -12,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0 }},
    
    {
        // white endgame
        { 0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0, -12, -12,   0,   0,   0,
            1,   1,   1,  10,  10,   1,   1,   1,
            3,   3,   3,  13,  13,   3,   3,   3,
            6,   6,   6,  16,  16,   6,   6,   6,
            10,  10,  10,  30,  30,  10,  10,  10,
            66,  66,  66,  66,  66,  66,  66,  66,
            0,   0,   0,   0,   0,   0,   0,   0 },
        // black endgame
        { 0,   0,   0,   0,   0,   0,   0,   0,
            66,  66,  66,  66,  66,  66,  66,  66,
            10,  10,  10,  30,  30,  10,  10,  10,
            6,   6,   6,  16,  16,   6,   6,   6,
            3,   3,   3,  13,  13,   3,   3,   3,
            1,   1,   1,  10,  10,   1,   1,   1,
            0,   0,   0, -12, -12,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0 }}
};

static const int knight_square_values[2][2][64] = {
    
    {
        // white middlegame
        {-19, -19, -19, -19, -19, -19, -19, -19,
            1,   2,   2,   2,   2,   2,   2,   1,
            1,   2,  19,  17,  17,  19,   2,   1,
            1,  12,  21,  24,  24,  21,  12,   1,
            1,  14,  23,  28,  28,  23,  14,   1,
            1,  14,  23,  27,  27,  23,  14,   1,
            1,  12,  18,  22,  22,  18,  12,   1,
            -29, -19, -19,  -9,  -9, -19, -19, -29 },
        
        // black middlegame
        {-29, -19, -19,  -9,  -9, -19, -19, -29,
            1,  12,  18,  22,  22,  18,  12,   1,
            1,  14,  23,  27,  27,  23,  14,   1,
            1,  14,  23,  28,  28,  23,  14,   1,
            1,  12,  21,  24,  24,  21,  12,   1,
            1,   2,  19,  17,  17,  19,   2,   1,
            1,   2,   2,   2,   2,   2,   2,   1,
            -19, -19, -19, -19, -19, -19, -19, -19 }},
    
    {
        // white endgame
        {-19, -19, -19, -19, -19, -19, -19, -19,
            1,   2,   2,   2,   2,   2,   2,   1,
            1,   2,  19,  17,  17,  19,   2,   1,
            1,  12,  21,  24,  24,  21,  12,   1,
            1,  14,  23,  28,  28,  23,  14,   1,
            1,  14,  23,  27,  27,  23,  14,   1,
            1,  12,  18,  22,  22,  18,  12,   1,
            -29, -19, -19,  -9,  -9, -19, -19, -29 },
        // black endgame
        {-29, -19, -19,  -9,  -9, -19, -19, -29,
            1,  12,  18,  22,  22,  18,  12,   1,
            1,  14,  23,  27,  27,  23,  14,   1,
            1,  14,  23,  28,  28,  23,  14,   1,
            1,  12,  21,  24,  24,  21,  12,   1,
            1,   2,  19,  17,  17,  19,   2,   1,
            1,   2,   2,   2,   2,   2,   2,   1,
            -19, -19, -19, -19, -19, -19, -19, -19 }}
    
};

static const int bishop_square_values[2][2][64] = {
    {
        // white middlegame
        {-10, -10,  -8,  -6,  -6,  -8, -10, -10,
            0,   8,   6,   8,   8,   6,   8,   0,
            2,   6,  12,  10,  10,  12,   6,   2,
            4,   8,  10,  16,  16,  10,   8,   4,
            4,   8,  10,  16,  16,  10,   8,   4,
            2,   6,  12,  10,  10,  12,   6,   2,
            0,   8,   6,   8,   8,   6,   8,   0,
            0,   0,   2,   4,   4,   2,   0,   0 },
        
        // black middlegame
        {  0,   0,   2,   4,   4,   2,   0,   0,
            0,   8,   6,   8,   8,   6,   8,   0,
            2,   6,  12,  10,  10,  12,   6,   2,
            4,   8,  10,  16,  16,  10,   8,   4,
            4,   8,  10,  16,  16,  10,   8,   4,
            2,   6,  12,  10,  10,  12,   6,   2,
            0,   8,   6,   8,   8,   6,   8,   0,
            -10, -10,  -8,  -6,  -6,  -8, -10, -10 }},
    
    {
        // white endgame
        {-10, -10,  -8,  -6,  -6,  -8, -10, -10,
            0,   8,   6,   8,   8,   6,   8,   0,
            2,   6,  12,  10,  10,  12,   6,   2,
            4,   8,  10,  16,  16,  10,   8,   4,
            4,   8,  10,  16,  16,  10,   8,   4,
            2,   6,  12,  10,  10,  12,   6,   2,
            0,   8,   6,   8,   8,   6,   8,   0,
            0,   0,   2,   4,   4,   2,   0,   0 },
        
        // black endgame
        {  0,   0,   2,   4,   4,   2,   0,   0,
            0,   8,   6,   8,   8,   6,   8,   0,
            2,   6,  12,  10,  10,  12,   6,   2,
            4,   8,  10,  16,  16,  10,   8,   4,
            4,   8,  10,  16,  16,  10,   8,   4,
            2,   6,  12,  10,  10,  12,   6,   2,
            0,   8,   6,   8,   8,   6,   8,   0,
            -10, -10,  -8,  -6,  -6,  -8, -10, -10 }}
    
};

static const int queen_square_values[2][2][64] = {
    {
        // white middlegame
        { 0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   4,   4,   4,   4,   0,   0,
            0,   4,   4,   6,   6,   4,   4,   0,
            0,   4,   6,   8,   8,   6,   4,   0,
            0,   4,   6,   8,   8,   6,   4,   0,
            0,   4,   4,   6,   6,   4,   4,   0,
            0,   0,   4,   4,   4,   4,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0 },
        // black middlegame
        { 0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   4,   4,   4,   4,   0,   0,
            0,   4,   4,   6,   6,   4,   4,   0,
            0,   4,   6,   8,   8,   6,   4,   0,
            0,   4,   6,   8,   8,   6,   4,   0,
            0,   4,   4,   6,   6,   4,   4,   0,
            0,   0,   4,   4,   4,   4,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0 }},
    
    {
        // white endgame
        { 0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   4,   4,   4,   4,   0,   0,
            0,   4,   4,   6,   6,   4,   4,   0,
            0,   4,   6,   8,   8,   6,   4,   0,
            0,   4,   6,   8,   8,   6,   4,   0,
            0,   4,   4,   6,   6,   4,   4,   0,
            0,   0,   4,   4,   4,   4,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0 },
        
        // black endgame
        { 0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   4,   4,   4,   4,   0,   0,
            0,   4,   4,   6,   6,   4,   4,   0,
            0,   4,   6,   8,   8,   6,   4,   0,
            0,   4,   6,   8,   8,   6,   4,   0,
            0,   4,   4,   6,   6,   4,   4,   0,
            0,   0,   4,   4,   4,   4,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0 }}
};

static const int king_vals_mid[2][64] = {
    { // white
        -40, -40, -40, -40, -40, -40, -40, -40,
        -40, -10, -10, -10, -10, -10, -10, -40,
        -40, -10,  20,  20,  20,  20, -10, -40,
        -40, -10,  40,  40,  40,  40, -10, -40,
        -40, -10,  60,  60,  60,  60, -10, -40,
        -40, -10,  60,  60,  60,  60, -10, -40,
        -40, -10, -10, -10, -10, -10, -10, -40,
        -40, -40, -40, -40, -40, -40, -40, -40 },
    // black
    {-40, -40, -40, -40, -40, -40, -40, -40,
        -40, -10, -10, -10, -10, -10, -10, -40,
        -40, -10,  60,  60,  60,  60, -10, -40,
        -40, -10,  60,  60,  60,  60, -10, -40,
        -40, -10,  40,  40,  40,  40, -10, -40,
        -40, -10,  20,  20,  20,  20, -10, -40,
        -40, -10, -10, -10, -10, -10, -10, -40,
        -40, -40, -40, -40, -40, -40, -40, -40 }
};

static const int king_vals_k[2][64] = {
    // white
    {-60, -40, -20, -20, -20, -20, -20, -20,
        -60, -40, -20,   0,   0,   0,   0,   0,
        -60, -40, -20,  20,  20,  20,  20,  20,
        -60, -40, -20,  20,  40,  40,  40,  40,
        -60, -40, -20,  20,  60,  60,  60,  40,
        -60, -40, -20,  20,  60,  60,  60,  40,
        -60, -40, -20,  20,  40,  40,  40,  40,
        -60, -40, -20, -20, -20, -20, -20, -20 },
    // black
    {-60, -40, -20, -20, -20, -20, -20, -20,
        -60, -40, -20,  20,  40,  40,  40,  40,
        -60, -40, -20,  20,  60,  60,  60,  40,
        -60, -40, -20,  20,  60,  60,  60,  40,
        -60, -40, -20,  20,  40,  40,  40,  40,
        -60, -40, -20,  20,  20,  20,  20,  20,
        -60, -40, -20,   0,   0,   0,   0,   0,
        -60, -40, -20, -20, -20, -20, -20, -20 }
    
};

static const int king_vals_q[2][64] = {
    // white
    {-20, -20, -20, -20, -20, -20, -40, -60,
        0,   0,   0,   0,   0, -20, -40, -60,
        20,  20,  20,  20,  20, -20, -40, -60,
        40,  40,  40,  40,  20, -20, -40, -60,
        40,  60,  60,  60,  20, -20, -40, -60,
        40,  60,  60,  60,  20, -20, -40, -60,
        40,  40,  40,  40,  20, -20, -40, -60,
        -20, -20, -20, -20, -20, -20, -40, -60 },
    // black
    {-20, -20, -20, -20, -20, -20, -40, -60,
        40,  40,  40,  40,  20, -20, -40, -60,
        40,  60,  60,  60,  20, -20, -40, -60,
        40,  60,  60,  60,  20, -20, -40, -60,
        40,  40,  40,  40,  20, -20, -40, -60,
        20,  20,  20,  20,  20, -20, -40, -60,
        0,   0,   0,   0,   0, -20, -40, -60,
        -20, -20, -20, -20, -20, -20, -40, -60 }
};


int eval(Board_t *board) {
    
    if (isDraw(board))
        return CONTEMPT_FACTOR;
    
    int value = 0;
    int mult;
    mg_weight = 0;
    eg_weight = OPENING_MATERIAL;
    
    for (int square = 0;square < 64;square++) {
        if (board->pieces[square] == 0)
            continue;
        
        mult = board->colors[square] == board->turn ? 1 : -1;
        value += mult * pieceValues[board->pieces[square]];
        
        mg_weight += pieceValues[board->pieces[square]];
        eg_weight -= pieceValues[board->pieces[square]];
    }
    
    mg_weight /= OPENING_MATERIAL;
    eg_weight /= OPENING_MATERIAL;
    
    if (mg_weight > 1.0)
        mg_weight = 1.0;
    else if (mg_weight < 0.0)
        mg_weight = 0.0;
    
    if (eg_weight > 1.0)
        eg_weight = 1.0;
    else if (eg_weight < 0.0)
        eg_weight = 0.0;
    
    for (int color = 0; color < 2; color++) {
        mult = color == board->turn ? 1 : -1;
        value += mult * evalKnights(board, color);
        value += mult * evalKings(board, color);
        value += mult * evalBishops(board, color);
        value += mult * evalRooks(board, color);
        value += mult * evalQueens(board, color);
        value += mult * evalDev(board, color);
        
        value += mult * evalPawns(board, color);
        if (passed_pawns)
            value += mult * evalPassedPawns(board, color);
    }
    
    
    return value;
    
}

int evalPawns(Board_t *board, int color) {
    passed_pawns = EMPTY_BITBOARD;
    PTKey key = pawn_hash(board, color);
    int index = PAWN_TABLE_INDEX(key, ptable.length);
    
    if (ptable.keys[index] == key) {
        passed_pawns = ptable.passed[index];
        return ptable.scores[index];
    }
    
    int value = 0;
    Bitboard pawns;
    int attacker_count = 0, defender_count = 0;
    int isolated, passed;
    int file, rank;
    int inc_forward;
    int mg_val = 0, eg_val = 0, temp;
    int square;
    int enemy = ENEMY(color);
    inc_forward = FORWARD(color);
    
    for (pawns = BITBOARD(board, color, PAWN); pawns; pawns &= pawns - 1) {
        square = ls1b(pawns);
        file = FILE(square);
        rank = RANK(square);
        
        mg_val += pawn_square_values[MG][color][square];
        eg_val += pawn_square_values[EG][color][square];
        
        isolated = 1;
        if (file != 0 && (BITBOARD(board, color, PAWN) & FileMasks[file - 1]))
            isolated = 0;
        if (file != 7 && (BITBOARD(board, color, PAWN) & FileMasks[file + 1]))
            isolated = 0;
        
        if (isolated) {
            mg_val -= isolated_pawn[MG];
            eg_val -= isolated_pawn[EG];
            if (file == 3 || file == 4) {
                mg_val -= isolated_pawn[MG] / 2;
                eg_val -= isolated_pawn[EG] / 2;
            }
        } else {
            
            attacker_count = attack_count(board, enemy, square);
            defender_count = attack_count(board, color, square);
            
            if (attacker_count > defender_count || defender_count == 0) {
                mg_val -= pawn_weak[MG];
                eg_val -= pawn_weak[EG];
                if (file == 3 || file == 4) {
                    mg_val -= pawn_weak[MG] / 2;
                    eg_val -= pawn_weak[EG] / 2;
                }
            }
        }
        
        passed = 1;
        temp = square;
        for (square = square + inc_forward; ON_BOARD(square); square += inc_forward) {

            if (board->pieces[square] == PAWN)
                passed = 0;
            else if (ON_BOARD(square - 1) && file != 0 && board->pieces[square - 1] == PAWN)
                passed = 0;
            else if (ON_BOARD(square + 1) && file != 7 && board->pieces[square + 1] == PAWN)
                passed = 0;
            
            if (passed == 0)
                break;
        }
        
        if (passed) {
            mg_val += passed_pawn[MG][color][rank];
            eg_val += passed_pawn[EG][color][rank];
            passed_pawns |= (Bitboard)1 << temp;
        }
        
    }
    
    for (int a = 0; a < 8;a++) {
        pawns = BITBOARD(board, color, PAWN) & FileMasks[a];
        if (pawns & pawns - 1) { // doubled
            mg_val -= pawn_doubled[MG];
            eg_val -= pawn_doubled[EG];
        }
        
        pawns = BITBOARD(board, color, PAWN) & RankMasks[a];
        if (pawns & pawns - 1) { // side by side
            mg_val += pawn_duo[MG];
            eg_val += pawn_duo[EG];
        }
    }
    
    value = (int)(mg_val * mg_weight + eg_val * eg_weight);
    
    ptable.keys[index] = key;
    ptable.scores[index] = value;
    ptable.passed[index] = passed_pawns;
    
    return value;
}

int evalKings(Board_t *board, int color) {
    
    int value;
    int mg_val = 0, eg_val = 0;
    int myKing = KING_SQUARE(board, color);
    Bitboard myPawns = BITBOARD(board, color, PAWN);
    int file = FILE(myKing);
    int forward = FORWARD(color);
    
    if ((ALL_PAWNS(board) & ABCDMask) && (ALL_PAWNS(board) & EFGHMask))
        eg_val += king_vals_mid[color][myKing];
    else if (ALL_PAWNS(board) & ABCDMask)
        eg_val += king_vals_q[color][myKing];
    else if (ALL_PAWNS(board) & EFGHMask)
        eg_val += king_vals_k[color][myKing];
    
    if (ON_BOARD(myKing + forward)) {
        if (board->pieces[myKing + forward] == PAWN && board->colors[myKing + forward] == color) {
            mg_val += pawn_shield[MG];
            eg_val += pawn_shield[EG];
        }
        
        if (file != 0 && board->pieces[myKing + forward - 1] == PAWN && board->colors[myKing + forward - 1] == color) {
            mg_val += pawn_shield[MG];
            eg_val += pawn_shield[EG];
        }
        
        if (file != 7 && board->pieces[myKing + forward + 1] == PAWN && board->colors[myKing + forward + 1] == color) {
            mg_val += pawn_shield[MG];
            eg_val += pawn_shield[EG];
        }
    }
    
    if ((board->didCastle) & (1 << color))
    {
        mg_val += has_castle[MG];
        eg_val += has_castle[EG];
    } else {
        if (color == WHITE) {
            if (board->castling & WHITE_CASTLE_KINGSIDE) {
                mg_val += can_castle[MG];
                eg_val += can_castle[EG];
                if (board->pieces[F2] == PAWN && board->pieces[G2] == PAWN && board->pieces[H2] == PAWN) {
                    mg_val += pawn_shield[MG];
                    eg_val += pawn_shield[EG];
                }
            }
            
            if (board->castling & WHITE_CASTLE_QUEENSIDE) {
                mg_val += can_castle[MG];
                eg_val += can_castle[EG];
                if (board->pieces[A2] == PAWN && board->pieces[B2] == PAWN && board->pieces[C2] == PAWN) {
                    mg_val += pawn_shield[MG];
                    eg_val += pawn_shield[EG];
                }
            }
            
            if ((board->pieces[F1] == KING || board->pieces[G1] == KING) && board->pieces[H1] == ROOK) {
                mg_val -= blocked_rook[MG];
                eg_val -= blocked_rook[EG];
            }
            
        } else {
            if (board->castling & BLACK_CASTLE_KINGSIDE) {
                mg_val += can_castle[MG];
                eg_val += can_castle[EG];
                if (board->pieces[F7] == PAWN && board->pieces[G7] == PAWN && board->pieces[H7] == PAWN) {
                    mg_val += pawn_shield[MG];
                    eg_val += pawn_shield[EG];
                }
            }
            
            if (board->castling & BLACK_CASTLE_QUEENSIDE) {
                mg_val += can_castle[MG];
                eg_val += can_castle[EG];
                if (board->pieces[C7] == PAWN && board->pieces[B7] == PAWN && board->pieces[A7] == PAWN) {
                    mg_val += pawn_shield[MG];
                    eg_val += pawn_shield[EG];
                }
            }
            
            if ((board->pieces[F8] == KING || board->pieces[G8] == KING) && board->pieces[H8] == ROOK) {
                mg_val -= blocked_rook[MG];
                eg_val -= blocked_rook[EG];
            }
        }
    }
    
    if ((myPawns & FileMasks[file]) == 0) {
        mg_val -= no_pawn_shield[MG];
        eg_val -= no_pawn_shield[EG];
    }
    
    if (file != 0 && (myPawns & FileMasks[file - 1]) == 0) {
        mg_val -= no_pawn_shield[MG];
        eg_val -= no_pawn_shield[EG];
    }
    
    if (file != 7 && (myPawns & FileMasks[file + 1]) == 0) {
        mg_val -= no_pawn_shield[MG];
        eg_val -= no_pawn_shield[EG];
    }
    
    value = (int)(mg_val * mg_weight + eg_val * eg_weight);
    
    return value;
}

int evalBishops(Board_t *board, int color) {
    int value, square, outpost, attacked, file, index, mobility;
    int mg_val = 0, eg_val = 0;
    int enemy = ENEMY(color);
    int right = FORWARD(color) + PAWN_CAPTURE_RIGHT(color);
    int left = FORWARD(color) + PAWN_CAPTURE_LEFT(color);
    Bitboard bishops, occupancy, attacks;
    
    for (bishops = BITBOARD(board, color, BISHOP); bishops; bishops &= bishops - 1) {
        square = ls1b(bishops);
        file = FILE(square);
        
        mg_val += bishop_square_values[MG][color][square];
        eg_val += bishop_square_values[EG][color][square];
        if (square_colors[square] == color) {
            mg_val += same_color_bishop[MG];
            eg_val += same_color_bishop[EG];
        }
        
        
        outpost = bishop_outpost[color][square];
        
        if (outpost) {
            attacked = 0;
            if (ON_BOARD(square + right) && file != 7 && board->pieces[square + right] == PAWN && board->colors[square + right] == enemy)
                attacked = 1;
            if (ON_BOARD(square + left) && file != 0 && board->pieces[square + left] == PAWN && board->colors[square + left] == enemy)
                attacked = 1;
            
            if (!attacked)
                outpost += bishop_outpost[color][square] / 2;
            
            if ((knightAttacks[square] & BITBOARD(board, enemy, KNIGHT)) == 0)
                outpost += bishop_outpost[color][square];
        }
        
        mg_val += outpost;
        eg_val += outpost;
        
        occupancy = board->occupied & bishopMasks[square];
        index = (int)(occupancy * magicNumbersBishop[square] >> magicBitshiftsBishop[square]);
        attacks = magicMovesBishop[square][index];
        
        
        mobility = countBits(attacks);
        mg_val += mobility;
        eg_val += mobility;
    }
    
    bishops = BITBOARD(board, color, BISHOP);
    if (bishops & bishops - 1) {
        mg_val += double_bishops[MG];
        eg_val += double_bishops[EG];
    }
    
    value = (int)(mg_val * mg_weight + eg_val * eg_weight);
    
    return value;
}

int evalKnights(Board_t *board, int color) {
    
    int value;
    int mg_val = 0, eg_val = 0;
    int outpost, attacked;
    int square, file, mobility;
    int enemy = ENEMY(color);
    int right = PAWN_CAPTURE_RIGHT(color) + FORWARD(color);
    int left = PAWN_CAPTURE_LEFT(color) + FORWARD(color);
    
    Bitboard knights, attacks;
    
    for (knights = BITBOARD(board, color, KNIGHT); knights; knights &= knights - 1) {
        square = ls1b(knights);
        file = FILE(square);
        
        mg_val += knight_square_values[MG][color][square];
        eg_val += knight_square_values[EG][color][square];
        
        attacks = knightAttacks[square];
        mobility = countBits(attacks);
        
        mg_val += mobility;
        eg_val += mobility;
        
        outpost = knight_outpost[color][square];
        if (outpost) {
            attacked = 0;
            if (ON_BOARD(square + right) && file != 7 && board->pieces[square + right] == PAWN && board->colors[square + right] == enemy)
                attacked = 1;
            else if (ON_BOARD(square + left) && file != 0 && board->pieces[square + left] == PAWN && board->colors[square + left] == enemy)
                attacked = 1;
            
            if (!attacked)
                outpost += knight_outpost[color][square] / 2;
            if ((attacks & BITBOARD(board, enemy, KNIGHT)) == 0)
                outpost += knight_outpost[color][square];
            
            mg_val += outpost;
            eg_val += outpost;
        }
        
    }
    
    value = (int)(mg_val * mg_weight + eg_val * eg_weight);
    
    return value;
}

int evalRooks(Board_t *board, int color) {
    int value, square, rank, file, index, mobility;
    int enemy = ENEMY(color);
    int mg_val = 0, eg_val = 0;
    
    Bitboard rooks, attacks, occupancy, myPawns, enemyPawns;
    
    for (rooks = BITBOARD(board, color, ROOK); rooks; rooks &= rooks - 1) {
        square = ls1b(rooks);
        file = FILE(square);
        rank = RANK(square);
        myPawns = BITBOARD(board, color, PAWN) & FileMasks[file];
        enemyPawns = BITBOARD(board, enemy, PAWN) & FileMasks[file];
        
        if (myPawns == 0 && enemyPawns == 0)
        {
            mg_val += rook_open_file[MG];
            eg_val += rook_open_file[EG];
        } else if (myPawns == 0) {
            mg_val += rook_half_open_file[MG];
            eg_val += rook_half_open_file[EG];
        }
        
        if (color == WHITE && rank == 6) { // 7th rank
            mg_val += rook_on_7[MG];
            eg_val += rook_on_7[EG];
        } else if (color == BLACK && rank == 1) {
            mg_val += rook_on_7[MG];
            eg_val += rook_on_7[EG];
        }
        
        occupancy = board->occupied & rookMasks[square];
        index = (int)(occupancy * magicNumbersRook[square] >> magicBitshiftsRook[square]);
        attacks = magicMovesRook[square][index];
        
        mobility = countBits(attacks);
        eg_val += mobility;
        mg_val += mobility;
    }
    
    value = (int)(mg_val * mg_weight + eg_val * eg_weight);
    
    return value;
}

int evalQueens(Board_t *board, int color) {
    int value, square;
    int mg_val = 0, eg_val = 0;
    
    Bitboard queens;
    
    for (queens = BITBOARD(board, color, QUEEN); queens; queens &= queens - 1) {
        square = ls1b(queens);
        
        mg_val += queen_square_values[MG][color][square];
        eg_val += queen_square_values[EG][color][square];
        
    }
    
    value = (int)(mg_val * mg_weight + eg_val * eg_weight);
    
    return value;
}

int evalDev(Board_t *board, int color) {
    int value;
    int mg_val = 0;
    int q;
    
    if (color == WHITE) {
        q = BITBOARD(board, BLACK, QUEEN) != 0 ? 3 : 1;
        
        
        if (board->pieces[D4] == PAWN && board->colors[D4] == WHITE && board->pieces[C2] == PAWN && board->colors[C2] == WHITE) {
            if (board->pieces[C3])
                mg_val -= blocked_c_pawn;
        }
        
        if (board->pieces[E1] != KING && (board->didCastle & (1 << WHITE)) == 0) {
            mg_val -= q * no_castle;
        }
        
        if ((board->didCastle & (1 << WHITE)) == 0)
            mg_val -= (q * no_castle) / 2;
        
        if (board->pieces[A1] == ROOK && board->pieces[B1] == KNIGHT)
            mg_val -= q * undeveloped_piece;
        
        if (board->pieces[G1] == KNIGHT && board->pieces[H1] == ROOK)
            mg_val -= q * undeveloped_piece;
        
    } else {
        q = BITBOARD(board, WHITE, QUEEN) != 0 ? 3 : 1;
        
        if (board->pieces[D5] == PAWN && board->colors[D5] == BLACK && board->pieces[C7] == PAWN && board->colors[C7] == BLACK) {
            if (board->pieces[C6])
                mg_val -= blocked_c_pawn;
        }
        
        if (board->pieces[E8] != KING && (board->didCastle & (1 << BLACK)) == 0) {
            mg_val -= q * no_castle;
        }
        
        if ((board->didCastle & (1 << BLACK)) == 0)
            mg_val -= (q * no_castle) / 2;
        
        if (board->pieces[A8] == ROOK && board->pieces[B8] == KNIGHT)
            mg_val -= q * undeveloped_piece;
        
        if (board->pieces[G8] == KNIGHT && board->pieces[H8] == ROOK)
            mg_val -= q * undeveloped_piece;
    }
    
    value = mg_val * mg_weight;
    
    return value;
}

int evalPassedPawns(Board_t *board, int color) {
    int value;
    int mg_val = 0, eg_val = 0;
    int rank, file;
    int square;
    int enemy = ENEMY(color);
    int myKing = KING_SQUARE(board, color);
    int enemyKing = KING_SQUARE(board, enemy);
    
    for (; passed_pawns; passed_pawns &= passed_pawns - 1) {
        square = ls1b(passed_pawns);
        file = FILE(square);
        rank = RANK(square);
        
        mg_val = 200;
        eg_val = 220;
        
        if (pawnAttacks[ENEMY(color)][square] & BITBOARD(board, color, PAWN)) {
            mg_val += supported_passed_pawn[MG];
            eg_val += supported_passed_pawn[EG];
        }
        
        if (BITBOARD(board, color, ROOK) & FileMasks[file]) {
            mg_val += rook_behind_passed_pawn[MG];
            eg_val += rook_behind_passed_pawn[EG];
        }
        
        if (BITBOARD(board, enemy, ROOK) & FileMasks[file]) {
            mg_val -= rook_behind_passed_pawn[MG];
            eg_val -= rook_behind_passed_pawn[EG];
        }
        
        if (ENEMY_BITBOARD(color) & FileMasks[file]) {
            mg_val -= blocked_passed_pawn[MG];
            eg_val -= blocked_passed_pawn[EG];
        }
        
        if (FRIENDLY(color) & FileMasks[file]) {
            mg_val -= blocked_passed_pawn[MG] / 2;
            eg_val -= blocked_passed_pawn[EG] / 2;
        }
        
        mg_val -= (M_DIST(myKing, square) - M_DIST(enemyKing, square)) * passed_pawn_db[rank];
        
    }
    

    value = (mg_val * mg_weight + eg_val * eg_weight);
    
    return value;
}

int evalKingTropism(Board_t *board, int color) {
    int value;
    int mg_val = 0, eg_val = 0;
    
    
    
    
    value = (int)(mg_val * mg_weight + eg_val * eg_weight);
    
    return value;
}

int isDraw(Board_t *board) {
    
    Bitboard white, black;
    Board_t **stack;
    int stack_length;
    white = board->white;
    black = board->black;
    //int pieceCounts[2][7];
    
    if (board->movesSinceCaptureOrPawn >= 100) // 50 move rule
        return 1;
    
    //Kk
    if ((white & (white - 1)) == 0 && (black & (black - 1)) == 0)
        return 1;
    
    // 3 fold repetition
    stack = undo_stack(&stack_length);
    for (int a = 0;a < stack_length;a++) {
        if (stack[a]->key == board->key)
            return CONTEMPT_FACTOR;
    }
    
    //for (int square = 0;square < 64;square++)
      //  pieceCounts[board->colors[square]][board->pieces[square]]++;
    
    //if (pieceCounts[WHITE][ROOK] == 0 && pieceCounts[BLACK][ROOK] == 0 && pieceCounts[WHITE][QUEEN] == 0 && pieceCounts[BLACK][QUEEN] == 0) {
        
    //}
    
    return 0;
}

PTKey pawn_hash(Board_t *board, int color) {
    Bitboard pawns;
    PTKey hash = 0;
    int square;
    
    for (pawns = BITBOARD(board, color, PAWN); pawns; pawns &= pawns - 1) {
        square = ls1b(pawns);
        hash ^= pawnHashRands[square + 64 * color];
    }
    
    return hash;
}

void init_eval() {
    
    int fail;
    do {
        fail = 0;
        for (int a = 0;a < 128;a++)
            pawnHashRands[a] = random_value(TTSEED);
        
        for (int a = 0;a < 127;a++) {
            for (int b = a + 1;b < 128;b++)
                if (pawnHashRands[a] == pawnHashRands[b] || pawnHashRands[a] == 0 || pawnHashRands[b] == 0)
                    fail = 1;
        }
        
    } while (fail);
    
    ptable.length = PAWN_TABLE_LENGTH;
    ptable.keys = calloc(PAWN_TABLE_LENGTH, sizeof(PTKey));
    ptable.scores = calloc(PAWN_TABLE_LENGTH, sizeof(int));
    ptable.passed = calloc(PAWN_TABLE_LENGTH, sizeof(Bitboard));
    
}

void dealloc_eval() {
    free(ptable.keys);
    free(ptable.scores);
}