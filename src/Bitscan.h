//
//  Bitscan.h
//  JFresh
//
//  Created by Christian on 12/22/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_Bitscan_h
#define JFresh_Bitscan_h

int ls1b        (uint64_t bits); // returns the location of the ls1b in the given 64 bit integer
int countBits   (uint64_t bits); // counts the number of set bits in the given 64 bit integer
void getSetBits (uint64_t num, int *bits); // fills an array of ints with the location of all the set bits in the given 64 bit integer

#endif
