//
//  MoveGen.c
//  JFresh
//
//  Created by Christian on 12/8/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "Board.h"
#include "Move.h"
#include "MoveGen.h"
#include "Bitscan.h"
#include "Debug.h"
#include "Bitmask.h"
#include "MagicMoves.h"
#include "MagicNumbers.h"
#include "Piece.h"
#include "Attack.h"

int generateLegalMoves(Board_t *board, Move_t *moves) {
        
    int pCount = 0, lCount = 0;
    Move_t pMoves[MAX_NUM_MOVES];
    
    if (isCheck(board, board->turn)) {
        pCount = generateEvasions(board, pMoves);
        for (int a = 0;a < pCount;a++) {
            makeMove(pMoves[a], &board);
            if (!isCheck(board, ENEMY(board->turn))) {
                moves[lCount] = pMoves[a];
                lCount++;
            }
                
            undoMove(&board);
        }
        
    } else {
        
        pCount = generatePsuedoLegalMoves(board, pMoves);
        
        for (int a = 0;a < pCount;a++) {
            if (isLegal(board, pMoves[a])) {
                moves[lCount] = pMoves[a];
                lCount++;
            }
        }
    }
    
    return lCount;
}

int generatePsuedoLegalMoves(Board_t *board, Move_t *moves) {
    
    int n = 0;
    
    n = generatePawnMoves(board, moves);
    n += generatePawnCaptures(board, moves + n);
    n += generateKnightMoves(board, moves + n);
    n += generateSlidingMoves(board, moves + n);
    n += generateKingMoves(board, moves + n);
    n += generateCastles(board, moves + n);
    
    return n;
}

int generateEvasions(Board_t *board, Move_t *moves) {
    // castling not considered
    int n = 0;
    
    n = generatePawnMoves(board, moves);
    n += generatePawnCaptures(board, moves + n);
    n += generateKnightMoves(board, moves + n);
    n += generateSlidingMoves(board, moves + n);
    n += generateKingMoves(board, moves + n);
    
    return n;
}

int generateSlidingMoves(Board_t *board, Move_t *moves) {
    Bitboard rooks, bishops, queens, occupancy, attacks;
    int square, index, n = 0;
    
    for (rooks = BITBOARD(board, board->turn, ROOK); rooks; rooks &= rooks - 1) {
        square = ls1b(rooks);
        
        occupancy = board->occupied & rookMasks[square];
        index = (int)(occupancy * magicNumbersRook[square] >> magicBitshiftsRook[square]);
        
        attacks = magicMovesRook[square][index] & NOT_FRIENDLY(board->turn);
        
        n += generateMovesFromBitboard(board, attacks, square, 0, moves + n);
        
    }
    
    for (bishops = BITBOARD(board, board->turn, BISHOP); bishops; bishops &= bishops - 1) {
        square = ls1b(bishops);
        
        occupancy = board->occupied & bishopMasks[square];
        index = (int)(occupancy * magicNumbersBishop[square] >> magicBitshiftsBishop[square]);
        
        attacks = magicMovesBishop[square][index] & NOT_FRIENDLY(board->turn);
        
        n += generateMovesFromBitboard(board, attacks, square, 0, moves + n);
        
    }
    
    for (queens = BITBOARD(board, board->turn, QUEEN); queens; queens &= queens - 1) {
        square = ls1b(queens);
        
        // bishop pattern
        
        occupancy = board->occupied & bishopMasks[square];
        index = (int)(occupancy * magicNumbersBishop[square] >> magicBitshiftsBishop[square]);
        
        attacks = magicMovesBishop[square][index] & NOT_FRIENDLY(board->turn);
        
        // rook pattern
        occupancy = board->occupied & rookMasks[square];
        index = (int)(occupancy * magicNumbersRook[square] >> magicBitshiftsRook[square]);
        
        attacks |= magicMovesRook[square][index] & NOT_FRIENDLY(board->turn);
        
        n += generateMovesFromBitboard(board, attacks, square, 0, moves + n);
        
    }
    
    return n;
}

int generateKingMoves(Board_t *board, Move_t *moves) {
    int king = KING_SQUARE(board, board->turn);
    
    Bitboard attacks = kingAttacks[king] & NOT_FRIENDLY(board->turn);
    
    return generateMovesFromBitboard(board, attacks, king, 0, moves);
}

int generateCastles(Board_t *board, Move_t *moves) {
    int king = KING_SQUARE(board, board->turn);
    int turn = board->turn;
    int n = 0;
    
    if (turn == WHITE) {
        if (king == E1 && board->pieces[F1] == 0 && board->pieces[G1] == 0 && (board->castling & WHITE_CASTLE_KINGSIDE)) {
            if (!isAttacked(board, F1, BLACK) && !isCheck(board, WHITE)) {
                moves[n] = getMove(board, E1, G1, 0);
                n++;
            }
        }
        
        if (king == E1 && board->pieces[D1] == 0 && board->pieces[C1] == 0 && board->pieces[B1] == 0 && (board->castling & WHITE_CASTLE_QUEENSIDE)) {
            if (!isAttacked(board, 3, BLACK) && !isCheck(board, WHITE)) {
                moves[n] = getMove(board, E1, C1, 0);
                n++;
            }
        }
        
    } else {
        if (king == E8 && board->pieces[F8] == 0 && board->pieces[G8] == 0 && (board->castling & BLACK_CASTLE_KINGSIDE)) {
            if (!isAttacked(board, 61, WHITE) && !isCheck(board, BLACK)) {
                moves[n] = getMove(board, E8, G8, 0);
                n++;
            }
        }
        
        if (king == E8 && board->pieces[D8] == 0 && board->pieces[C8] == 0 && board->pieces[B8] == 0 && (board->castling & BLACK_CASTLE_QUEENSIDE)) {
            if (!isAttacked(board, D8, WHITE) && !isCheck(board, BLACK)) {
                moves[n] = getMove(board, E8, C8, 0);
                n++;
            }
        }
    }
    
    return n;
}

int generatePawnMoves(Board_t *board, Move_t *moves) {
    
    int square, turn, pawn_inc;
    int n = 0;
    Bitboard pawns;
    
    turn = board->turn;
    pawn_inc = FORWARD(turn);
    
    for (pawns = BITBOARD(board, turn, PAWN); pawns; pawns &= pawns - 1) {
        square = ls1b(pawns);
        
        if (board->pieces[square + pawn_inc] == 0) {
            if (RANK(square) != PAWN_PROMOTE_RANK(turn)) {
                moves[n] = getMove(board, square, square + pawn_inc, '\0');
                
            } else {
                moves[n] = getMove(board, square, square + pawn_inc, 'q');
                n++;
                moves[n] = getMove(board, square, square + pawn_inc, 'r');
                n++;
                moves[n] = getMove(board, square, square + pawn_inc, 'b');
                n++;
                moves[n] = getMove(board, square, square + pawn_inc, 'n');
            }
            
            n++;
            
            if (RANK(square) == PAWN_START_RANK(turn) && board->pieces[square + 2 * pawn_inc] == 0) {
                moves[n] = getMove(board, square, square + 2 * pawn_inc, '\0');
                n++;
            }
            
        }
        
    }
    
    return n;
}

int generatePawnCaptures(Board_t *board, Move_t *moves) {
    Bitboard pawns, attacks;
    int square, n = 0;
    
    for (pawns = BITBOARD(board, board->turn, PAWN); pawns; pawns &= pawns - 1) {
        square = ls1b(pawns);
        attacks = pawnAttacks[board->turn][square] & ENEMY_BITBOARD(board->turn);
        
        n += generateMovesFromBitboard(board, attacks, square, RANK(square) == PAWN_PROMOTE_RANK(board->turn), moves + n);
        
        // en passant capture
        if ((square + PAWN_CAPTURE_LEFT(board->turn) == board->epSquare && FILE(square) != 0) || (square + PAWN_CAPTURE_RIGHT(board->turn) == board->epSquare && FILE(square) != 7)) {
            moves[n] = getMove(board, square, board->epSquare, '\0');
            n++;
        }
    }
    
    return n;
}

int generateKnightMoves(Board_t *board, Move_t *moves) {
    Bitboard knights, attacks;
    
    int square, n = 0;
    
    
    for (knights = BITBOARD(board, board->turn, KNIGHT); knights; knights &= knights - 1) {
        square = ls1b(knights);
        
        attacks = knightAttacks[square] & NOT_FRIENDLY(board->turn);
        
        n += generateMovesFromBitboard(board, attacks, square, 0, moves + n);
        
    }
    
    return n;
}

int generateMovesFromBitboard(Board_t *board, Bitboard bitboard, int from, int isPromotion, Move_t *moves) {
    
    int to;
    int n = 0;
    
    while (bitboard) {
        to = ls1b(bitboard);
        
        if (isPromotion) {
            moves[n] = getMove(board, from, to, 'q');
            n++;
            moves[n] = getMove(board, from, to, 'r');
            n++;
            moves[n] = getMove(board, from, to, 'b');
            n++;
            moves[n] = getMove(board, from, to, 'n');
        } else {
            moves[n] = getMove(board, from, to, '\0');
        }
        
        n++;
        
        bitboard &= bitboard - 1;
    }
    
    return n;
}

int generateCaptures(Board_t *board, Move_t *moves) {
    int n, index, check, square, lcount = 0;
    Bitboard pieces, captures, occupancy;
    Move_t pmoves[MAX_NUM_MOVES];
    
    n = generatePawnCaptures(board, pmoves);
    
    for (int pType = ROOK;pType <= KING;pType++) {
        
        captures = EMPTY_BITBOARD;
        
        for (pieces = BITBOARD(board, board->turn, pType); pieces; pieces &= pieces - 1) {
            square = ls1b(pieces);
            
            switch (pType) {
                case KNIGHT:
                    captures = knightAttacks[square] & ENEMY_BITBOARD(board->turn);
                    
                    break;
                    
                case KING:
                    captures = kingAttacks[square] & ENEMY_BITBOARD(board->turn);
                    break;
                    
                case ROOK:
                    occupancy = board->occupied & rookMasks[square];
                    index = (int)(occupancy * magicNumbersRook[square] >> magicBitshiftsRook[square]);
                    captures = magicMovesRook[square][index] & ENEMY_BITBOARD(board->turn);
                    
                    break;
                    
                case BISHOP:
                    occupancy = board->occupied & bishopMasks[square];
                    index = (int)(occupancy * magicNumbersBishop[square] >> magicBitshiftsBishop[square]);
                    captures = magicMovesBishop[square][index] & ENEMY_BITBOARD(board->turn);
                    
                    break;
                    
                case QUEEN:
                    occupancy = board->occupied & rookMasks[square];
                    index = (int)(occupancy * magicNumbersRook[square] >> magicBitshiftsRook[square]);
                    captures = magicMovesRook[square][index] & ENEMY_BITBOARD(board->turn);
                    
                    occupancy = board->occupied & bishopMasks[square];
                    index = (int)(occupancy * magicNumbersBishop[square] >> magicBitshiftsBishop[square]);
                    captures |= magicMovesBishop[square][index] & ENEMY_BITBOARD(board->turn);
                    break;
                    
            }
            
            n += generateMovesFromBitboard(board, captures, square, 0, pmoves + n);
        }
        
        
    }
    
    check = isCheck(board, board->turn);
    
    if (check) {
        for (int a = 0;a < n;a++) {
            makeMove(pmoves[a], &board);
            if (!isCheck(board, ENEMY(board->turn))) {
                moves[lcount] = pmoves[a];
                lcount++;
            }
            
            undoMove(&board);
        }
    } else {
        for (int a = 0;a < n;a++) {
            if (isLegal(board, pmoves[a])) {
                moves[lcount] = pmoves[a];
                lcount++;
            }
        }
    }
    
    return lcount;
}