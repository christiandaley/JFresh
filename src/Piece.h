//
//  Piece.h
//  JFresh
//
//  Created by Christian on 12/30/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_Piece_h
#define JFresh_Piece_h

#define PAWN 1
#define ROOK 2
#define KNIGHT 3
#define BISHOP 4
#define QUEEN 5
#define KING 6

#define WHITE_CASTLE_KINGSIDE 1
#define WHITE_CASTLE_QUEENSIDE 2
#define BLACK_CASTLE_KINGSIDE 4
#define BLACK_CASTLE_QUEENSIDE 8

#define WHITE 0
#define BLACK 1

#define FORWARD(color) (((color) == WHITE ? 8 : -8))
#define PAWN_PROMOTE_RANK(color) (((color) == WHITE ? 6 : 1))
#define PAWN_START_RANK(color) (((color) == WHITE ? 1 : 6))
#define PAWN_CAPTURE_LEFT(color) (((color) == WHITE ? 7 : -9))
#define PAWN_CAPTURE_RIGHT(color) (((color) == WHITE ? 9 : -7))


#endif
