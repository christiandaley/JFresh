//
//  Bitscan.c
//  JFresh
//
//  Created by Christian on 12/22/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include <stdint.h>
#include "Bitscan.h"

const static int ls1b_64_table[64] =
{
    63, 30,  3, 32, 59, 14, 11, 33,
    60, 24, 50,  9, 55, 19, 21, 34,
    61, 29,  2, 53, 51, 23, 41, 18,
    56, 28,  1, 43, 46, 27,  0, 35,
    62, 31, 58,  4,  5, 49, 54,  6,
    15, 52, 12, 40,  7, 42, 45, 16,
    25, 57, 48, 13, 10, 39,  8, 44,
    20, 47, 38, 22, 17, 37, 36, 26
};

int ls1b(uint64_t bits) { // forward bitscan, using the algorithm created by Matt Taylor
    unsigned int folded;
    bits ^= (bits - 1);
    folded = (int)(bits ^ (bits >> 32));
    
    return ls1b_64_table[folded * 0x78291ACF >> 26];
}

int countBits(uint64_t bits) {
    int n = 0;
    
    while (bits) {
        bits &= bits - 1;
        n++;
    }
    
    return n;
}

void getSetBits(uint64_t num, int *bits) {
    int n = 0;
    while (num) {
        bits[n] = ls1b(num);
        num &= num - 1;
        n++;
    }
    
    bits[n] = -1;
}