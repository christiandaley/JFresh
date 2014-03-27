//
//  Thread.h
//  JFresh
//
//  Created by Christian on 1/2/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#ifndef JFresh_Thread_h
#define JFresh_Thread_h

#include "SearchOptions.h"
#include "Search.h"

#define SLEEP_TIME 5000
#define UPDATE_SPEED 500000

// starts the search thread with the given options
void startSearch        ();
void killSearch         ();
void checkStop          (SearchOptions_t *options);
void *monitorSearch     (void *args);
int msec_since          (struct timeval start);

#endif
