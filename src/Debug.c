//
//  Debug.c
//  JFresh
//
//  Created by Christian on 12/5/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "Debug.h"
#include "Move.h"
#include "Piece.h"
#include "Log.h"

void printBoardArray(Board_t *board) {
    const char *p = " PRNBQK";
    const char *c = "WB";
    for (int b = 7;b >= 0;b--) {
        send("|");
        for (int a = 0;a < 8;a++) {
            if (board->pieces[b * 8 + a] != 0) {
                send("%c%c|", c[board->colors[b * 8 + a]], p[board->pieces[b * 8 + a]]);
            } else {
                send("  |");
            }
        }
        
        send("\n");
        send("_________________________\n");
    }
    
    
}

void printBoardBits(Board_t *board) {
    const char *p = " PRNBQK";
    const char *cols = "WB";
    for (int b = 7; b >= 0;b--) {
        send("|");
        for (int a = 0;a < 8;a++) {
            int piece = 0;
            for (int c = 0;c < 2;c++) {
                for (int d = 0;d < 6;d++) {
                    if (board->bitboards[c][d] & ((Bitboard)1 << (b * 8 + a))) {
                        send("%c%c|", cols[c], p[d + 1]);
                        piece = 1;
                        break;
                    }
                }
                
                if (piece)
                    break;
            }
            
            if (!piece)
                send("  |");
        }
        
        send("\n");
        send("_________________________\n");
    }
    
    send("\n");
    
}

void printBitboard(Bitboard bitboard) {
    send("\n");
    for (int b = 7; b >= 0;b--) {
        for (int a = 0;a < 8;a++) {
            send("%d ", (bitboard & (uint64_t)1 << (b * 8 + a)) != 0);
        }
        
        send("\n");
    }
    
    send("\n");

}

void printBoardInfo(Board_t *board) {
    send("\n--------------\n");
    send("Castling Rights (KQkq): %d, %d, %d, %d\n", (board->castling & WHITE_CASTLE_KINGSIDE) != 0, (board->castling & WHITE_CASTLE_QUEENSIDE) != 0, (board->castling & BLACK_CASTLE_KINGSIDE) != 0, (board->castling & BLACK_CASTLE_QUEENSIDE) != 0);
    
    send("White/Black king squares: %d, %d\n", board->whiteKing, board->blackKing);
    send("Turn: %s\n", board->turn ? "Black" : "White");
    send("#Full moves: %d\n", board->numberOfMoves);
    send("#moves since capt/pawn: %d\n", board->movesSinceCaptureOrPawn);
    send("E.P Square: %d (%c%c)", board->epSquare, board->epSquare % 8 + 'a', board->epSquare / 8 + '1');
    send("\n--------------\n");
}

int checkBoardForErrors(Board_t *board) {
    
    for (int a = 0;a < 64;a++) {
        int piece = board->pieces[a];
        int color = board->colors[a];
        
        if (piece) {
            if ((board->bitboards[color][piece - 1] & (Bitboard)1 << a) == 0) {
                send("Error: Inconsistent value at: %d (%c%c)\n", a, a % 8 + 'a', a / 8 + '1');
                printBoardArray(board);
                send("\n");
                printBoardBits(board);
                return 1;
            }
        }
        
        if (((board->occupied & (Bitboard)1 << a) != 0) != (board->pieces[a] != 0)) {
            send("Error: Bitboard occupied and array are not in sync at: %d (%c%c)\n", a, a % 8 + 'a', a / 8 + '1');
            return 1;
        }
        
    }
    
    return 0;
    
}

void printMove(Move_t move) {
    if (move == NULL_MOVE) {
        send("NULL");
        return;
    }
    
    Flag_t flags = MOVE_FLAGS(move);
    
    send("%c%c", MOVE_FROM(move) % 8 + 'a', MOVE_FROM(move) / 8 + '1');
    send("%c%c", MOVE_TO(move) % 8 + 'a', MOVE_TO(move) / 8 + '1');
    
    if ((flags & FlagPromoteQueen) == FlagPromoteQueen)
        send("q");
    else if ((flags & FlagPromoteRook) == FlagPromoteRook)
        send("r");
    else if ((flags & FlagPromoteBishop) == FlagPromoteBishop)
        send("b");
    else if ((flags & FlagPromoteKnight) == FlagPromoteKnight)
        send("n");
}

void printMoveList(Move_t *list, int length) {
    
    send("\n-------------\n");
    
    for (int a = 0;a < length;a++) {
        printMove(list[a]);
        send(", ");
    }
    
    send("%d moves", length);
    
    send("\n-------------\n");
}


