//
//  Debug.h
//  JFresh
//
//  Created by Christian on 12/5/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_Debug_h
#define JFresh_Debug_h

#include "Undo.h"
#include "Board.h"
#include "Move.h"

void printBoardArray        (Board_t *board); // Print the board array
void printBoardBits         (Board_t *board); // print the bitbords
void printBitboard          (const Bitboard bitboard); // print the bits in the bitboard
void printBoardInfo         (Board_t *board); // prints info about the board
int checkBoardForErrors     (Board_t *board); // checks that the array and bitboard are in sync
void printMove              (Move_t move); // prints info about a move
void printMoveList          (Move_t *list, int length); // prints a list of moves

#endif
