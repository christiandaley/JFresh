//
//  Attack.c
//  JFresh
//
//  Created by Christian on 12/30/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include "Attack.h"
#include "Piece.h"
#include "Board.h"
#include "Move.h"
#include "Undo.h"
#include "Bitmask.h"
#include "MagicMoves.h"
#include "MagicNumbers.h"
#include "Debug.h"
#include "Bitscan.h"

int DeltaIncs[64][64];

void init_deltaIncs() {
    int temp;
    
    for (int a = 0;a < 64;a++) {
        for (int b = 0;b < 64;b++) {
            DeltaIncs[a][b] = 0;
            
            if (a == b)
                continue;
            
            if (FILE(a) == FILE(b)) {
                if (a < b)
                    DeltaIncs[a][b] = 8;
                else
                    DeltaIncs[a][b] = -8;
            }
            
            if (RANK(a) == RANK(b)) {
                if (a < b)
                    DeltaIncs[a][b] = 1;
                else
                    DeltaIncs[a][b] = -1;
            }
            
            temp = a + 7;
            while (FILE(temp) != 7 && temp < 64) {
                if (temp == b) {
                    DeltaIncs[a][b] = 7;
                    break;
                }
                
                temp += 7;
            }
            
            temp = a + 9;
            while (FILE(temp) != 0 && temp < 64) {
                if (temp == b) {
                    DeltaIncs[a][b] = 9;
                    break;
                }
                
                temp += 9;
            }
            
            temp = a - 7;
            while (FILE(temp) != 0 && temp >= 0) {
                if (temp == b) {
                    DeltaIncs[a][b] = -7;
                    break;
                }
                
                temp -= 7;
            }
            
            temp = a - 9;
            while (FILE(temp) != 7 && temp >= 0) {
                if (temp == b) {
                    DeltaIncs[a][b] = -9;
                    break;
                }
                
                temp -= 9;
            }

            
        }
    }
}

int isCheck(Board_t *board, int color) {
        
    return isAttacked(board, KING_SQUARE(board, color), ENEMY(color));
}

int isAttacked(Board_t *board, int square, int color) {
    return attacks_to_square(board, square, color) != 0;
}

int isLegal(Board_t *board, Move_t move) {
    
    int from, to, legal, piece, turn;
    
    from = MOVE_FROM(move);
    to = MOVE_TO(move);
    turn = board->turn;
    
    piece = board->pieces[from];
        
    if (MOVE_IS_EN_PASSANT(move)) {
        makeMove(move, &board);
        changeTurn(board);
        legal = !isCheck(board, turn);
        changeTurn(board);
        undoMove(&board);
        return legal;
    }
    
    if (piece == KING) {
        legal = !isAttacked(board, to, ENEMY(board->turn));
        return legal;
    }
    
    if (isPinned(board, from, turn))
        return DeltaIncs[KING_SQUARE(board, turn)][from] == DeltaIncs[KING_SQUARE(board, turn)][to];
    
    return 1;
}

int isPinned(Board_t *board, int square, int color) {
    
    int kingLoc = KING_SQUARE(board, color);
    int delta, loc;
    
    delta = DeltaIncs[kingLoc][square];
    
    if (delta == 0)
        return 0;
    
    loc = square;
    
    do {
        loc -= delta;
    } while (board->pieces[loc] == EMPTY_SQUARE && ON_BOARD(loc));
    
    if (loc != kingLoc) // blocker
        return 0;
    
    loc = square;
    
    do {
        loc += delta;
        
        if (loc > 64 || loc < 0) // no piece along the line, cannot be pinned
            return 0;
        
    } while (board->pieces[loc] == EMPTY_SQUARE);
    
    if (delta != DeltaIncs[square][loc])
        return 0;
    if (board->colors[loc] == color)
        return 0;
    
    int blocker = board->pieces[loc];
    
    if ((blocker == BISHOP || blocker == QUEEN) && (delta == 7 || delta == -7 || delta == 9 || delta == -9))
        return 1;
    if ((blocker == ROOK || blocker == QUEEN) && (delta == 8 || delta == 1 || delta == -8 || delta == -1))
        return 1;
    
    
    return 0;
}

Bitboard attack_board(Board_t *board, int color) {
    int piece, square, index;
    Bitboard attacks = EMPTY_BITBOARD;
    Bitboard occupancy;
    
    for (square = 0;square < 64;square++) {
        if (board->pieces[square] == 0 || board->colors[square] != color)
            continue;
        
        piece = board->pieces[square];
        
        switch (piece) {
                
            case PAWN:
                attacks |= pawnAttacks[color][square];
                
                break;
                
            case KNIGHT:
                
                attacks |= knightAttacks[square];
                
                break;
                
            case KING:
                
                attacks |= kingAttacks[square];
                
                break;
                
            case ROOK:
                
                occupancy = board->occupied & rookMasks[square];
                index = (int)(occupancy * magicNumbersRook[square] >> magicBitshiftsRook[square]);
                attacks |= magicMovesRook[square][index];
                
                break;
                
            case BISHOP:
                
                occupancy = board->occupied & bishopMasks[square];
                index = (int)(occupancy * magicNumbersBishop[square] >> magicBitshiftsBishop[square]);
                attacks |= magicMovesBishop[square][index];
                
                break;
                
            case QUEEN:
                // rook pattern
                occupancy = board->occupied & rookMasks[square];
                index = (int)(occupancy * magicNumbersRook[square] >> magicBitshiftsRook[square]);
                attacks |= magicMovesRook[square][index];
                
                // bishop pattern
                occupancy = board->occupied & bishopMasks[square];
                index = (int)(occupancy * magicNumbersBishop[square] >> magicBitshiftsBishop[square]);
                attacks |= magicMovesBishop[square][index];
                
                break;
                
        }
        
    }
    
    return attacks;
}

int attack_count(Board_t *board, int color, int target) {
    Bitboard attacks = attacks_to_square(board, target, color);
    
    return countBits(attacks);
}

Bitboard least_valuable_attacker (Board_t *board, Bitboard attacks, int byColor, int *piece) {
    Bitboard subset;
    const int order[6] = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};
    
    for (int a = 0;a < 6;a++) {
        *piece = order[a];
        subset = attacks & BITBOARD(board, byColor, *piece);
        
        if (subset)
            return subset & -subset;
    }
    
    *piece = 0;
    
    return EMPTY_BITBOARD;
}

Bitboard attacks_to_square(Board_t *board, int square, int byColor) {
    Bitboard attacks = EMPTY_BITBOARD, occupancy;
    int index;
    
    for (int ptype = PAWN; ptype <= KING; ptype++) {
        switch (ptype) {
            case PAWN:
                attacks |= pawnAttacks[ENEMY(byColor)][square] & BITBOARD(board, byColor, PAWN);
                
                break;
                
            case ROOK:
                occupancy = board->occupied & rookMasks[square];
                index = (int)(occupancy * magicNumbersRook[square] >> magicBitshiftsRook[square]);
                attacks |= magicMovesRook[square][index] & BITBOARD(board, byColor, ROOK);
                
                break;
                
            case BISHOP:
                occupancy = board->occupied & bishopMasks[square];
                index = (int)(occupancy * magicNumbersBishop[square] >> magicBitshiftsBishop[square]);
                attacks |= magicMovesBishop[square][index] & BITBOARD(board, byColor, BISHOP);
                
                break;
                
            case QUEEN:
                occupancy = board->occupied & bishopMasks[square];
                index = (int)(occupancy * magicNumbersBishop[square] >> magicBitshiftsBishop[square]);
                attacks |= magicMovesBishop[square][index] & BITBOARD(board, byColor, QUEEN);
                
                occupancy = board->occupied & rookMasks[square];
                index = (int)(occupancy * magicNumbersRook[square] >> magicBitshiftsRook[square]);
                attacks |= magicMovesRook[square][index] & BITBOARD(board, byColor, QUEEN);
                
                break;
                
            case KNIGHT:
                attacks |= knightAttacks[square] & BITBOARD(board, byColor, KNIGHT);
                
                break;
                
            case KING:
                attacks |= kingAttacks[square] & BITBOARD(board, byColor, KING);
                
                break;
        }

    }
    
    return attacks;
}
