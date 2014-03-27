//
//  MagicNumberGen.h
//  JFresh
//
//  Created by Christian on 12/27/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_MagicNumberGen_h
#define JFresh_MagicNumberGen_h

#include "MagicNumbers.h"

uint64_t rand64(); // generates a random 64 bit integer
MagicNumber randomMagic(); // generates a random magic number

void generateAllMagics      (int pieceType); // generates all needed magic numbers for the given piece type
void generateMagicNumber    (int square, int pieceType); // generates the magic number for the square and piece type, and prints that magic number to stdout
void generateBitShifts      (int pieceType); // generates all the appropriate bit shifts for the given piece type, and prints them to stdout

#endif
