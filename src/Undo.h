//
//  Undo.h
//  JFresh
//
//  Created by Christian on 12/30/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_Undo_h
#define JFresh_Undo_h

#include "Board.h"

void init_undo          (Board_t *orig); // initializes the undo stack
void free_undo          ();
void saveBoard          (Board_t **ptr); // saves a board in memory
void undoMove           (Board_t **ptr); // retrieves the last board saved in memory
void reset_undo         ();
Board_t **undo_stack    (int *length);


#endif
