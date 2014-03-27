//
//  Move.c
//  JFresh
//
//  Created by Christian on 12/6/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include "Board.h"
#include "Move.h"
#include "Undo.h"
#include "Debug.h"
#include "Bitmask.h"
#include "Piece.h"
#include "Attack.h"
#include "MoveGen.h"
#include "TT.h"
#include <stdlib.h>
#include <stdio.h>

const uint8_t FlagNone = 0;
const uint8_t FlagPawn = 1;
const uint8_t FlagCastleKingside = 2;
const uint8_t FlagCastleQueenside = 3;
const uint8_t FlagCapture = 4;
const uint8_t FlagEnPassant = 5;

const uint8_t FlagPromoteKnight = 8;
const uint8_t FlagPromoteBishop = 9;
const uint8_t FlagPromoteRook = 10;
const uint8_t FlagPromoteQueen = 11;
const uint8_t FlagCheck = 15;

void makeMove(Move_t move, Board_t **ptr) {
    
    saveBoard(ptr);
    
    Board_t *board = *ptr;
    
    int to = MOVE_TO(move);
    int from = MOVE_FROM(move);
    
    int turn = board->turn;
    
    board->epSquare = NOT_ON_BOARD;
    
    set_bitboard(board, BITBOARD(board, turn, board->pieces[from]) ^ moveBoards[from][to], turn, board->pieces[from]);
    
    
    if (turn == WHITE)
        board->white ^= moveBoards[from][to];
    else
        board->black ^= moveBoards[from][to];
    
    
    if (MOVE_IS_CASTLE_KINGSIDE(move)) {
        BITBOARD(board, turn, ROOK) ^= moveBoards[from + 1][from + 3];
        
        setSquare(board, from + 3, 0, 0);
        setSquare(board, from + 1, ROOK, turn);
        
        board->didCastle |= 1 << turn;
        if (turn == WHITE)
            board->white ^= moveBoards[from + 1][from + 3];
        else
            board->black ^= moveBoards[from + 1][from + 3];
        
    } else if (MOVE_IS_CASTLE_QUEENSIDE(move)) {
        BITBOARD(board, turn, ROOK) ^= moveBoards[from - 1][from - 4];
        
        setSquare(board, from - 4, 0, 0);
        setSquare(board, from - 1, ROOK, turn);
        
        board->didCastle |= 1 << turn;
        if (turn == WHITE)
            board->white ^= moveBoards[from - 1][from - 4];
        else
            board->black ^= moveBoards[from - 1][from - 4];
        
    } else if (MOVE_IS_EN_PASSANT(move)) {
        if (turn == WHITE) {
            BITBOARD(board, BLACK, PAWN) ^= (Bitboard)1 << (to - 8);
            
            setSquare(board, to - 8, 0, 0);
            
            board->black ^= (Bitboard)1 << (to - 8);
        } else {
            BITBOARD(board, WHITE, PAWN) ^= (Bitboard)1 << (to + 8);

            setSquare(board, to + 8, 0, 0);
            
            board->white ^= (Bitboard)1 << (to + 8);
        }
        
        board->movesSinceCaptureOrPawn = 0;
    } else if (MOVE_IS_CAPTURE(move)) {
        
        BITBOARD(board, ENEMY(turn), board->pieces[to]) ^= (Bitboard)1 << to;
        board->movesSinceCaptureOrPawn = 0;
        
        if (turn == WHITE)
            board->black ^= (Bitboard)1 << to;
        else
            board->white ^= (Bitboard)1 << to;
    }
    
    setSquare(board, to, board->pieces[from], board->colors[from]);
    setSquare(board, from, 0, 0);
    
    if (board->pieces[to] == PAWN) {
        board->movesSinceCaptureOrPawn = 0;
        if (from + 16 == to)
            board->epSquare = from + 8;
        else if (from - 16 == to)
            board->epSquare = from - 8;
        
        if (MOVE_IS_PROMOTION(move)) { // promotion
            
            BITBOARD(board, turn, PAWN) ^= (Bitboard)1 << to;
            
            if (MOVE_IS_PROMOTE_QUEEN(move)) {
                
                BITBOARD(board, turn, QUEEN) ^= (Bitboard)1 << to;
                setSquare(board, to, QUEEN, turn);
                
            } else if (MOVE_IS_PROMOTE_ROOK(move)) {
                BITBOARD(board, turn, ROOK) ^= (Bitboard)1 << to;
                setSquare(board, to, ROOK, turn);
                
            } else if (MOVE_IS_PROMOTE_BISHOP(move)) {
                BITBOARD(board, turn, BISHOP) ^= (Bitboard)1 << to;
                setSquare(board, to, BISHOP, turn);
                
            } else if (MOVE_IS_PROMOTE_KNIGHT(move)) {
                BITBOARD(board, turn, KNIGHT) ^= (Bitboard)1 << to;
                setSquare(board, to, KNIGHT, turn);
            }
        }
    }
    
    board->occupied = board->white | board->black;
    
    if ((board->white & 1) == 0)
        board->castling &= ~WHITE_CASTLE_QUEENSIDE;
    
    if ((board->white & (1 << 7)) == 0)
        board->castling &= ~WHITE_CASTLE_KINGSIDE;
    
    if ((board->black & (Bitboard)1 << 56) == 0)
        board->castling &= ~BLACK_CASTLE_QUEENSIDE;
    
    if ((board->black & (Bitboard)1 << 63) == 0)
        board->castling &= ~BLACK_CASTLE_KINGSIDE;
        
    if (board->pieces[to] == KING) {
        if (board->turn == WHITE) {
            board->castling &= 0xC;
            board->whiteKing = to;
        } else {
            board->castling &= 0x3;
            board->blackKing = to;
        }
    }
    
    changeTurn(board); // change turn
    if (board->turn == WHITE)
        board->numberOfMoves++;
    
}

void make_null_move(Board_t **ptr) {
    saveBoard(ptr);
    Board_t *board = *ptr;
    board->epSquare = NOT_ON_BOARD;
    changeTurn(board);
}

Move_t getMove(Board_t *board, int from, int to, char promote) {
    
    Move_t move;
    Flag_t flags = FlagNone;
    
    if (board->pieces[from] == KING && to == from + 2)
        flags = FlagCastleKingside;
    else if (board->pieces[from] == KING && to == from - 2)
        flags = FlagCastleQueenside;
    else if (board->pieces[to] != 0)
        flags |= FlagCapture;
    
    else if (to == board->epSquare && board->pieces[from] == PAWN)
        flags |= FlagEnPassant;
    
    switch (promote) {
            case 'q':
            flags |= FlagPromoteQueen;
            break;
            
            case 'r':
            flags |= FlagPromoteRook;
            
            break;
            
            case 'b':
            flags |= FlagPromoteBishop;
            break;
            
            case 'n':
            flags |= FlagPromoteKnight;
            
            break;
    }
    
    move = (flags << 12) | (from << 6) | to;
    
    return move;
}