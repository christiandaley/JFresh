//
//  Board.h
//  JFresh
//
//  Created by Christian on 12/5/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_Board_h
#define JFresh_Board_h

#include "Bitboard.h"
#include <stdint.h>
#include <stdlib.h>

#define NOT_ON_BOARD 255
#define EMPTY_SQUARE 0

#define FILE(square) ((square) % 8)
#define RANK(square) ((square) / 8)
#define KING_SQUARE(board, color) (((color) == WHITE ? board->whiteKing : board->blackKing))
#define ENEMY(color) ((1 - (color)))
#define BITBOARD(board, color, type) (((board)->bitboards[(color)][(type) - 1]))
#define FRIENDLY(color) (((color) == WHITE ? board->white : board->black))
#define NOT_FRIENDLY(color) ((~FRIENDLY(color)))
#define ENEMY_BITBOARD(color) (((color) == WHITE ? board->black : board->white))
#define ON_BOARD(square) (((square) >= 0 && (square) <= 63))
#define M_DIST(sq1, sq2) ((abs((sq1) / 8 - (sq2) / 2) + abs(((sq1) % 8) - ((sq2) % 8))))
#define ALL_PAWNS(board) ((BITBOARD(board, WHITE, PAWN) | BITBOARD(board, BLACK, PAWN)))

extern size_t BoardSize;

extern const uint8_t A1, A2, A3, A4, A5, A6, A7, A8;
extern const uint8_t B1, B2, B3, B4, B5, B6, B7, B8;
extern const uint8_t C1, C2, C3, C4, C5, C6, C7, C8;
extern const uint8_t D1, D2, D3, D4, D5, D6, D7, D8;
extern const uint8_t E1, E2, E3, E4, E5, E6, E7, E8;
extern const uint8_t F1, F2, F3, F4, F5, F6, F7, F8;
extern const uint8_t G1, G2, G3, G4, G5, G6, G7, G8;
extern const uint8_t H1, H2, H3, H4, H5, H6, H7, H8;

typedef struct {
    uint8_t pieces[64];
    uint8_t colors[64];
    
    Bitboard bitboards[2][6];
    Bitboard occupied;
    Bitboard black;
    Bitboard white;
    
    uint8_t castling;
    uint8_t didCastle;
    
    uint8_t whiteKing;
    uint8_t blackKing;
    uint8_t turn;
    uint8_t numberOfMoves;
    uint8_t movesSinceCaptureOrPawn;
    uint8_t epSquare;
    uint64_t key;
    
} Board_t;

void clearBoard         (Board_t *board); // clears the board
void resetBoard         (Board_t *board); // sets the board to the starting position
void setupBitboards     (Board_t *board); // sets up the bitboards to reflect the board array
int getBoardlocation    (char file, char rank); // get a board location from a string position e.g e4
void setKingLocations   (Board_t *board); // sets the loction of the kings
int isLegalSquare       (int file, int rank); // determines if the given square is on the board or not
void getXY              (int square, int *x, int *y); // gets x and y cordinates from a single location on the board
void changeTurn         (Board_t *board); // switches the side ot move on the board
void setSquare          (Board_t *board, int square, int piece, int color); // sets the piece at the requested square
int areEqual            (Board_t *b1, Board_t *b2);
int board_is_ok         (Board_t *board);
void set_bitboard       (Board_t *board, Bitboard bb, int color, int ptype);

#endif