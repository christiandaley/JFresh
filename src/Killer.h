//
//  Killer.h
//  JFresh
//
//  Created by Christian on 2/8/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#ifndef JFresh_Killer_h
#define JFresh_Killer_h

#include "Move.h"

void add_killer        (Move_t move, int depth);
void clear_killers     ();
int is_killer          (Move_t move, int depth);


#endif
