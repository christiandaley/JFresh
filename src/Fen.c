//
//  Fen.c
//  JFresh
//
//  Created by Christian on 12/24/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include <string.h>
#include <stdlib.h>
#include "Fen.h"
#include "Piece.h"

void nextToken(char **line) { // increments the string pointer until it is no longer pointing to whitespace
    
    do {
        (*line)++;
    } while (**line == ' ');
}

char *parseFen(char *line, Board_t *board) { // parse a FEN string
    clearBoard(board);
    
    
    int rank = 7;
    int file = 0;
    const char *letters = "PRNBQKprnbqk";
    
    while (line[0] != ' ') { // parse the piece positions
        int square = (int)(strchr(letters, line[0]) - letters);
        
        if (square < 0) { // current character in the fen string is not a piece character
            
            if (line[0] == '/') { // end of rank reached
                line++;
                continue;
            } else { // a number of empty spaces on the board
                file += line[0] - '1';
            }
            
        } else { // put the piece on the board
            
            setSquare(board, rank * 8 + file, square % 6 + 1, square / 6);
        }
        
        file++; // increment the file/rank
        if (file >= 8) {
            rank--;
            file = 0;
        }
        
        line++;
    }

    nextToken(&line);
    
    if (line[0] == 'b')
        board->turn = BLACK;
    else
        board->turn = WHITE;
    
    
    nextToken(&line);
    
    while (line[0] != ' ') { // caslting rights
        switch (line[0]) {
            case 'K':
                board->castling |= WHITE_CASTLE_KINGSIDE;
                break;
            case 'Q':
                board->castling |= WHITE_CASTLE_QUEENSIDE;
                break;
            case 'k':
                board->castling |= BLACK_CASTLE_KINGSIDE;
                break;
                
            case 'q':
                board->castling |= BLACK_CASTLE_QUEENSIDE;
                break;
        }
        
        line++;
    }
    
    nextToken(&line);
    
    if (line[0] != '-') { // get the ep square, if there is one
        int epSquare = getBoardlocation(line[0], line[1]);
        board->epSquare = epSquare;
    } else {
        board->epSquare = NOT_ON_BOARD;
    }
        
    nextToken(&line);
    
    
    if (line[0] == '\n' || line[0] == '\0' || line[0] - '0' < 0 || line[0] - '0' > 9) {
        board->movesSinceCaptureOrPawn = 0;
        board->numberOfMoves = 0;
        setupBitboards(board);
        return line;
    }
    
    board->movesSinceCaptureOrPawn = atoi(line);
    
    nextToken(&line);
    
    board->numberOfMoves = atoi(line);
    
    nextToken(&line);
    
    setupBitboards(board); // update the bitboards accordingly
    
    return line;
}
