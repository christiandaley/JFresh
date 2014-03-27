//
//  MagicMoves.c
//  JFresh
//
//  Created by Christian on 12/27/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include <stdio.h>
#include "MagicMoves.h"
#include "MagicNumbers.h"
#include "Bitscan.h"
#include "Bitmask.h"
#include "Debug.h"
#include "Log.h"

Bitboard magicMovesRook[64][4096];
Bitboard magicMovesBishop[64][512];

void init_bishopMoves() {
    Bitboard valid;
    int vars, bitCount, index, temp;
    
    for (int square = 0;square < 64;square++) {
        bitCount = countBits(bishopMasks[square]);
        vars = 1 << bitCount;
        
        for (int a = 0;a < vars;a++) {
            valid = EMPTY_BITBOARD;
            index = (int)((occupancyVariationsBishop[square][a] * magicNumbersBishop[square]) >> magicBitshiftsBishop[square]);
            
            for (temp = square + 7; temp % 8 != 7 && temp < 64;temp += 7) {
                valid |= (Bitboard)1 << temp;
                if (occupancyVariationsBishop[square][a] & (Bitboard)1 << temp)
                    break;
            }
            
            for (temp = square + 9; temp % 8 != 0 && temp < 64;temp += 9) {
                valid |= (Bitboard)1 << temp;
                if (occupancyVariationsBishop[square][a] & (Bitboard)1 << temp)
                    break;
            }
            
            for (temp = square - 7; temp % 8 != 0 && temp >= 0;temp -= 7) {
                
                valid |= (Bitboard)1 << temp;
                if (occupancyVariationsBishop[square][a] & (Bitboard)1 << temp)
                    break;
            }
            
            for (temp = square - 9; temp % 8 != 7 && temp >= 0;temp -= 9) {
                valid |= (Bitboard)1 << temp;
                
                if (occupancyVariationsBishop[square][a] & (Bitboard)1 << temp)
                    break;
            }
            
            if (magicMovesBishop[square][index] != 0 && magicMovesBishop[square][index] != valid) {
                send("Error: bishop magic move collision at square %d\n", square); // degugging to make sure no magics collide
                exit(1);
            }
            
            magicMovesBishop[square][index] = valid;
            
            
        }
        
    }
}

void init_rookMoves() {
    Bitboard valid;
    int vars, bitCount, index, temp;
    
    for (int square = 0;square < 64;square++) {
        bitCount = countBits(rookMasks[square]);
        vars = 1 << bitCount;
        
        for (int a = 0;a < vars;a++) {
            valid = EMPTY_BITBOARD;
            index = (int)((occupancyVariationsRook[square][a] * magicNumbersRook[square]) >> magicBitshiftsRook[square]);
            
            for (temp = square + 8; temp < 64;temp += 8) {
                valid |= (Bitboard)1 << temp;
                if (occupancyVariationsRook[square][a] & (Bitboard)1 << temp)
                    break;
            }
            
            for (temp = square + 1; temp % 8 != 0 && temp < 64;temp++) {
                valid |= (Bitboard)1 << temp;
                if (occupancyVariationsRook[square][a] & (Bitboard)1 << temp)
                    break;
            }
            
            for (temp = square - 8; temp >= 0;temp -= 8) {
                
                valid |= (Bitboard)1 << temp;
                if (occupancyVariationsRook[square][a] & (Bitboard)1 << temp)
                    break;
            }
            
            for (temp = square - 1; temp % 8 != 7 && temp >= 0;temp--) {
                valid |= (Bitboard)1 << temp;
                
                if (occupancyVariationsRook[square][a] & (Bitboard)1 << temp)
                    break;
            }
            
            if (magicMovesRook[square][index] != 0 && magicMovesRook[square][index] != valid) {
                send("Error: rook magic move collision at square %d\n", square); // degugging to make sure no magics collide
                exit(1);
            }
            
            magicMovesRook[square][index] = valid;
            
        }
        
    }
}

void init_magicMoves() {
    init_bishopMoves();
    init_rookMoves();
}

