//
//  UciOptions.c
//  JFresh
//
//  Created by Christian on 1/4/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "UciOptions.h"
#include "Uci.h"
#include "Log.h"
#include "TT.h"

static EngineOptions_t uci_options;

void setDefaults() {
    uci_options.ttSize = 256;
    uci_options.useNullMove = 1;
    uci_options.lmrAllowed = 1;
    uci_options.ponderAllowed = 0;
    uci_options.log = 1;
}

void printOptions() {
    send("option name Hash type spin default 256 min 8 max 1024\n");
    send("option name NullMove type check default true\n");
    send("option name Ponder type check default false\n");
    send("option name LMR type check default true\n");
    send("option name Log type check default true\n");
    
}

void printEngineInfo() {
    send("JFresh, version %s by Christian Daley\n", ENGINE_VERSION);
}

char *setOptions(char *line) {
    char token[MAX_TOKEN_LENGTH];
    
    while (line[0] != '\0') {
        line = parseToken(line, token);
        
        if (strcmp(token, "name") == 0) {
            if (strcmp(line, "Clear Hash\n") == 0) {
                clear_tt();
                continue;
            }
            
            line = parseToken(line, token);
            if (strcmp(token, "Hash") == 0) {
                line = parseToken(line, token);
                line = parseToken(line, token);
                uci_options.ttSize = atoi(token);
            } else if (strcmp(token, "Ponder") == 0) {
                line = parseToken(line, token);
                line = parseToken(line, token);
                uci_options.ponderAllowed = strcmp(token, "true") == 0;
            } else if (strcmp(token, "NullMove") == 0) {
                line = parseToken(line, token);
                line = parseToken(line, token);
                uci_options.useNullMove = strcmp(token, "true") == 0;
            } else if (strcmp(token, "LMR") == 0) {
                line = parseToken(line, token);
                line = parseToken(line, token);
                uci_options.lmrAllowed = strcmp(token, "true") == 0;
            }
            
        } else {
            
        }
        
    }
    
    return line;
}


EngineOptions_t *shared_engine_options() {
    return &uci_options;
}