//
//  UciOptions.h
//  JFresh
//
//  Created by Christian on 1/4/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#ifndef JFresh_UciOptions_h
#define JFresh_UciOptions_h

#include <stdint.h>

typedef struct {
    
    uint16_t ttSize;
    int useNullMove;
    int ponderAllowed;
    int lmrAllowed;
    int log;
    
} EngineOptions_t;

void setDefaults                        ();
void printOptions                       ();
void printEngineInfo                    ();
char *setOptions                        (char *line);
EngineOptions_t *shared_engine_options  ();

#endif
