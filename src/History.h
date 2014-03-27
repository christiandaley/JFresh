//
//  History.h
//  JFresh
//
//  Created by Christian on 2/9/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#ifndef JFresh_History_h
#define JFresh_History_h

#include "Move.h"

void add_history    (Move_t move, int depth);
void clear_history  ();
int history_score   (Move_t move);

#endif
