//
//  Board.c
//  JFresh
//
//  Created by Christian on 12/5/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include "Board.h"
#include "Debug.h"
#include "MoveGen.h"
#include "Piece.h"
#include "TT.h"

size_t BoardSize = sizeof(Board_t);

const uint8_t A1 = 0;
const uint8_t A2 = 8;
const uint8_t A3 = 16;
const uint8_t A4 = 24;
const uint8_t A5 = 32;
const uint8_t A6 = 40;
const uint8_t A7 = 48;
const uint8_t A8 = 56;

const uint8_t B1 = 1;
const uint8_t B2 = 9;
const uint8_t B3 = 17;
const uint8_t B4 = 25;
const uint8_t B5 = 33;
const uint8_t B6 = 41;
const uint8_t B7 = 49;
const uint8_t B8 = 57;

const uint8_t C1 = 2;
const uint8_t C2 = 10;
const uint8_t C3 = 18;
const uint8_t C4 = 26;
const uint8_t C5 = 34;
const uint8_t C6 = 42;
const uint8_t C7 = 50;
const uint8_t C8 = 58;

const uint8_t D1 = 3;
const uint8_t D2 = 11;
const uint8_t D3 = 19;
const uint8_t D4 = 27;
const uint8_t D5 = 35;
const uint8_t D6 = 43;
const uint8_t D7 = 51;
const uint8_t D8 = 59;

const uint8_t E1 = 4;
const uint8_t E2 = 12;
const uint8_t E3 = 20;
const uint8_t E4 = 28;
const uint8_t E5 = 36;
const uint8_t E6 = 44;
const uint8_t E7 = 52;
const uint8_t E8 = 60;

const uint8_t F1 = 5;
const uint8_t F2 = 13;
const uint8_t F3 = 21;
const uint8_t F4 = 29;
const uint8_t F5 = 37;
const uint8_t F6 = 45;
const uint8_t F7 = 53;
const uint8_t F8 = 61;

const uint8_t G1 = 6;
const uint8_t G2 = 14;
const uint8_t G3 = 22;
const uint8_t G4 = 30;
const uint8_t G5 = 38;
const uint8_t G6 = 46;
const uint8_t G7 = 54;
const uint8_t G8 = 62;

const uint8_t H1 = 7;
const uint8_t H2 = 15;
const uint8_t H3 = 23;
const uint8_t H4 = 31;
const uint8_t H5 = 39;
const uint8_t H6 = 47;
const uint8_t H7 = 55;
const uint8_t H8 = 63;

void clearBoard(Board_t *board) {
    for (int a = 0;a < 64;a++) { // clear the array
        board->pieces[a] = EMPTY_SQUARE;
    }
    
    for (int a = 0;a < 6;a++) { // clear the bitboards
        board->bitboards[0][a] = EMPTY_BITBOARD;
        board->bitboards[1][a] = EMPTY_BITBOARD;
    }
    
    board->castling = 0;
    
    board->whiteKing = NOT_ON_BOARD;
    board->blackKing = NOT_ON_BOARD;
    board->turn = WHITE;
    board->numberOfMoves = 0;
    board->movesSinceCaptureOrPawn = 0;
    board->didCastle = 0;
    board->epSquare = NOT_ON_BOARD;
    board->white = EMPTY_BITBOARD;
    board->black = EMPTY_BITBOARD;
    board->key = 0;
}

void resetBoard(Board_t *board) { // set the board to starting position
    
    clearBoard(board); // clear the board first
    
    board->whiteKing = 5; // set the kings to their proper spot
    board->blackKing = 61;
    for (int a=0;a<8;a++) { // Put the pawns in place
        setSquare(board, a + 8, PAWN, WHITE);
        setSquare(board, a + 48, PAWN, BLACK);
    }
    
    for (int a=0;a<2;a++) { // Put the other pieces in
        
        setSquare(board, a * 56, ROOK, a);
        setSquare(board, a * 56 + 7, ROOK, a);
        
        setSquare(board, a * 56 + 1, KNIGHT, a);
        setSquare(board, a * 56 + 6, KNIGHT, a);
        
        setSquare(board, a * 56 + 2, BISHOP, a);
        setSquare(board, a * 56 + 5, BISHOP, a);
        
        setSquare(board, a * 56 + 3, QUEEN, a);
        
        setSquare(board, a * 56 + 4, KING, a);
    }
    
    board->castling = 0xF; // Castling rights
    
    setupBitboards(board);
    setKingLocations(board);
    
    init_hash();
    board->key = position_hash(board);
}

void setupBitboards(Board_t *board) { // setup the bitboards
    
    for (int a = 0;a < 64;a++) { // go through each square on the board
        
        if (board->pieces[a] != 0) { // if the square if occupied
            
            if (board->colors[a] == WHITE)
                board->white |= (Bitboard)1 << a;
            else
                board->black |= (Bitboard)1 << a;
                
            board->bitboards[board->colors[a]][board->pieces[a] - 1] |= (Bitboard)1 << a;
        }

    }
    
    board->occupied = board->white | board->black;
}

int getBoardlocation(char file, char rank) {
    int numRank = rank - '1';
    int numFile = file - 'a';
    
    return numRank * 8 + numFile;
}

void setKingLocations(Board_t *board) {
    for (int a = 0;a < 64;a++) {
        if (board->pieces[a] == KING) {
            if (board->colors[a] == WHITE)
                board->whiteKing = a;
            else
                board->blackKing = a;
        }
    }
}

int isLegalSquare(int file, int rank) {
    if (file < 0 || file > 7 || rank < 0 || rank > 7)
        return 0;
    
    return 1;
}

void getXY(int square, int *x, int *y) {
    *x = FILE(square);
    *y = RANK(square);
}

void changeTurn(Board_t *board) {
    board->turn = !board->turn;
}

void setSquare(Board_t *board, int square, int piece, int color) {
    board->pieces[square] = piece;
    board->colors[square] = color;
}

int areEqual(Board_t *b1, Board_t *b2) {
    for (int a = 0;a < 64;a++) {
        if (b1->pieces[a] != b2->pieces[a])
            return 0;
    }
    
    return 1;
}

int board_is_ok(Board_t *board) {
    for (int square = 0;square < 64;square++) {
        if (board->colors[square] > 1 || board->colors[square] < 0 || board->pieces[square] < 0 || board->pieces[square] > 6)
            return 0;
    }
    
    return 1;
}

void set_bitboard(Board_t *board, Bitboard bb, int color, int ptype) {
    board->bitboards[color][ptype - 1] = bb;
}
