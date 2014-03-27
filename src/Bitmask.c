//
//  Bitmask.c
//  JFresh
//
//  Created by Christian on 12/27/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include "Bitmask.h"
#include "Debug.h"
#include "Bitscan.h"
#include "Piece.h"

Bitboard FileMasks[8];
Bitboard RankMasks[8];

Bitboard ABCDMask;
Bitboard EFGHMask;

Bitboard rookMasks[64];
Bitboard bishopMasks[64];

Bitboard occupancyVariationsRook[64][4096];
Bitboard occupancyVariationsBishop[64][512];

Bitboard bishopAttacks[64][512];
Bitboard rookAttacks[64][4096];

Bitboard knightAttacks[64];
Bitboard pawnAttacks[2][64];
Bitboard kingAttacks[64];

Bitboard moveBoards[64][64];

void init_moveBoards() {
    for (int a = 0;a < 64;a++) {
        for (int b = 0;b < 64;b++) {
            moveBoards[a][b] ^= (Bitboard)1 << a;
            moveBoards[a][b] ^= (Bitboard)1 << b;
        }
    }
}

void init_fileRankMasks() {
    const Bitboard fileA = 0x0101010101010101ULL;
    const Bitboard rank1 = 0xFFULL;
    
    for (int a = 0; a < 8; a++) {
        FileMasks[a] = fileA << a;
        RankMasks[a] = rank1 << (8 * a);
    }
    
    ABCDMask = FileMasks[0] | FileMasks[1] | FileMasks[2] | FileMasks[3];
    EFGHMask = FileMasks[4] | FileMasks[5] | FileMasks[6] | FileMasks[7];
    
}

void generateOccupancyMasks() {
    init_fileRankMasks();
    
    for (int square = 0;square < 64;square++) {
         // rook occupancy masks
        
        rookMasks[square] = FileMasks[FILE(square)] | RankMasks[RANK(square)];
        rookMasks[square] ^= (Bitboard)1 << square;
        if (FILE(square) != 0)
            rookMasks[square] &= ~FileMasks[0];
        if (FILE(square) != 7)
            rookMasks[square] &= ~FileMasks[7];
        if (RANK(square) != 0)
            rookMasks[square] &= ~RankMasks[0];
        if (RANK(square) != 7)
            rookMasks[square] &= ~RankMasks[7];
        
        // bishop occupancy masks
        int x, y;
        getXY(square, &x, &y);
        
        while (x - 1 > 0 && y + 1 < 7) {
            x--; y++;
            bishopMasks[square] |= (Bitboard)1 << (y * 8 + x);
        }
        
        getXY(square, &x, &y);
        
        while (x + 1 < 7 && y + 1 < 7) {
            x++; y++;
            bishopMasks[square] |= (Bitboard)1 << (y * 8 + x);
        }
        
        getXY(square, &x, &y);
        
        while (x + 1 < 7 && y - 1 > 0) {
            x++; y--;
            bishopMasks[square] |= (Bitboard)1 << (y * 8 + x);
        }
        
        getXY(square, &x, &y);
        
        while (x - 1 > 0 && y - 1 > 0) {
            x--; y--;
            bishopMasks[square] |= (Bitboard)1 << (y * 8 + x);
        }
        
        // knights
        getXY(square, &x, &y);
        
        if (isLegalSquare(x - 2, y + 1))
            knightAttacks[square] |= (Bitboard)1 << (square + 6);
        
        if (isLegalSquare(x - 1, y + 2))
            knightAttacks[square] |= (Bitboard)1 << (square + 15);
        
        if (isLegalSquare(x + 1, y + 2))
            knightAttacks[square] |= (Bitboard)1 << (square + 17);
        
        if (isLegalSquare(x + 2, y + 1))
            knightAttacks[square] |= (Bitboard)1 << (square + 10);
        
        if (isLegalSquare(x + 2, y - 1))
            knightAttacks[square] |= (Bitboard)1 << (square - 6);
        
        if (isLegalSquare(x + 1, y - 2))
            knightAttacks[square] |= (Bitboard)1 << (square - 15);
        
        if (isLegalSquare(x - 1, y - 2))
            knightAttacks[square] |= (Bitboard)1 << (square - 17);
        
        if (isLegalSquare(x - 2, y - 1))
            knightAttacks[square] |= (Bitboard)1 << (square - 10);
        
        // king attacks
        if (isLegalSquare(x - 1,  y + 1))
            kingAttacks[square] |= (Bitboard)1 << (square + 7);
        
        if (isLegalSquare(x,  y + 1))
            kingAttacks[square] |= (Bitboard)1 << (square + 8);
        
        if (isLegalSquare(x + 1,  y + 1))
            kingAttacks[square] |= (Bitboard)1 << (square + 9);
        
        if (isLegalSquare(x + 1,  y))
            kingAttacks[square] |= (Bitboard)1 << (square + 1);
        
        if (isLegalSquare(x + 1,  y - 1))
            kingAttacks[square] |= (Bitboard)1 << (square - 7);
        
        if (isLegalSquare(x,  y - 1))
            kingAttacks[square] |= (Bitboard)1 << (square - 8);
        
        if (isLegalSquare(x - 1,  y - 1))
            kingAttacks[square] |= (Bitboard)1 << (square - 9);
        
        if (isLegalSquare(x - 1,  y))
            kingAttacks[square] |= (Bitboard)1 << (square - 1);
        
        // pawn attacks
        if (isLegalSquare(x - 1, y + 1))
            pawnAttacks[WHITE][square] |= (Bitboard)1 << (square + 7);
        if (isLegalSquare(x + 1, y + 1))
            pawnAttacks[WHITE][square] |= (Bitboard)1 << (square + 9);
        
        if (isLegalSquare(x + 1, y - 1))
            pawnAttacks[BLACK][square] |= (Bitboard)1 << (square - 7);
        if (isLegalSquare(x - 1, y - 1))
            pawnAttacks[BLACK][square] |= (Bitboard)1 << (square - 9);
        
    }
    
}

void init_attackVariations() {
    
    Bitboard mask;
    int varCount;
    int maskBits[64];
    int indexBits[64];
    int bitCount;
    int temp;
    
    
    for (int square = 0;square < 64;square++) { // rook occupancy/attack variations
        mask = rookMasks[square];
        
        getSetBits(mask, maskBits);
        bitCount = countBits(mask);
        varCount = 1 << bitCount;
        
        for (int a = 0;a < varCount;a++) {
            
            occupancyVariationsRook[square][a] = EMPTY_BITBOARD;
            getSetBits((uint64_t)a, indexBits);
            
            for (int b = 0;indexBits[b] != -1;b++) {
                occupancyVariationsRook[square][a] |= (Bitboard)1 << maskBits[indexBits[b]];
            }
            
            for (temp = square + 8;temp < 56 && (occupancyVariationsRook[square][a] & (Bitboard)1 << temp) == 0; temp += 8)
                rookAttacks[square][a] |= (Bitboard)1 << temp;
                    
            for (temp = square - 8;temp > 7 && (occupancyVariationsRook[square][a] & (Bitboard)1 << temp) == 0; temp -= 8)
                rookAttacks[square][a] |= (Bitboard)1 << temp;
                    
            for (temp = square + 1;temp % 8 != 7 && temp % 8 != 0 && (occupancyVariationsRook[square][a] & (Bitboard)1 << temp) == 0; temp++)
                rookAttacks[square][a] |= (Bitboard)1 << temp;
                    
            for (temp = square - 1; temp > 0 && temp % 8 != 0 && temp % 8 != 7 && (occupancyVariationsRook[square][a] & (Bitboard)1 << temp) == 0; temp--)
                rookAttacks[square][a] |= (Bitboard)1 << temp;
            
            
        }
        
    }
    
    
    for (int square = 0;square < 64;square++) { // bishop occupancy/attack variations
        mask = bishopMasks[square];
        getSetBits(mask, maskBits);
        bitCount = countBits(mask);
        varCount = 1 << bitCount;
        
        for (int a = 0;a < varCount;a++) {
            occupancyVariationsBishop[square][a] = EMPTY_BITBOARD;
            getSetBits((uint64_t)a, indexBits);
            
            for (int b = 0;indexBits[b] != -1;b++) {
                occupancyVariationsBishop[square][a] |= (Bitboard)1 << maskBits[indexBits[b]];
            }
            
            for (temp = square + 7; temp % 8 != 0 && temp % 8 != 7 && temp < 56 && (occupancyVariationsBishop[square][a] & (Bitboard)1 << temp) == 0; temp += 7)
                bishopAttacks[square][a] |= (Bitboard)1 << temp;
            
            for (temp = square + 9; temp % 8 != 0 && temp % 8 != 7 && temp < 56 && (occupancyVariationsBishop[square][a] & (Bitboard)1 << temp) == 0; temp += 9)
                bishopAttacks[square][a] |= (Bitboard)1 << temp;
            
            for (temp = square - 7; temp % 8 != 0 && temp % 8 != 7 && temp > 7 && (occupancyVariationsBishop[square][a] & (Bitboard)1 << temp) == 0; temp -= 7)
                bishopAttacks[square][a] |= (Bitboard)1 << temp;
            
            for (temp = square - 9; temp % 8 != 0 && temp % 8 != 7 && temp > 7 && (occupancyVariationsBishop[square][a] & (Bitboard)1 << temp) == 0; temp -= 9)
                bishopAttacks[square][a] |= (Bitboard)1 << temp;
            
        }
        
    }
        
}

void init_bitmasks() {
    generateOccupancyMasks();
    init_attackVariations();
    init_moveBoards();

}