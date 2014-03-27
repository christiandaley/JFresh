//
//  MagicNumberGen.c
//  JFresh
//
//  Created by Christian on 12/27/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>

#include "MagicNumberGen.h"
#include "Bitmask.h"
#include "Bitboard.h"
#include "Bitscan.h"
#include "Board.h"
#include "Piece.h"
#include "Debug.h"
#include "Log.h"

uint64_t rand64() {
    
    uint64_t rand1, rand2;
    
    rand1 = (uint64_t)rand();
    rand2 = (uint64_t)rand();
    
    return rand1 | (rand2 << 32);
}

MagicNumber randomMagic() {
    
    
    return rand64() & rand64() & rand64(); // sparse bitboards are more likely to be magic numbers
}

void generateAllMagics(int pieceType) {
    for (int square = 0;square < 64;square++)
        generateMagicNumber(square, pieceType);
}

void generateMagicNumber(int square, int pieceType) {
    
    int varCount, index, bitCount, fail;
    Bitboard *used;
    
    MagicNumber magic = 0;
    
    Bitboard occupancy, attacks, mask;
    
    mask = MASK(pieceType, square);
        
    bitCount = countBits(mask);
    varCount = 1 << bitCount;
    
    do {
        magic = randomMagic();
        
        fail = 0;
        used = calloc(varCount, sizeof(Bitboard));

        for (int a = 0;a < varCount;a++) {
            occupancy = OCCUPANCY(pieceType, square, a);
            attacks = ATTACKS(pieceType, square, a);
            
            index = (int)(occupancy * magic >> (64 - bitCount));
            
            if (used[index] != 0 && used[index] != attacks) {
                fail = 1;
                break;
            }
            
            used[index] = attacks;
            
            
        }
        
        free(used);
        
    } while (fail);
    
    send("0x%016llxULL, ", magic);
    
}

void generateBitShifts(int pieceType) {
    Bitboard mask;
    int bitCount;
    
    for (int square = 0; square < 64;square++) {
        mask = pieceType == ROOK ? rookMasks[square] : bishopMasks[square];
        bitCount = countBits(mask);
        send("%d, ", 64 - bitCount);
    }
}