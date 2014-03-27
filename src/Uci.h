//
//  Uci.h
//  JFresh
//
//  Created by Christian on 12/5/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#ifndef JFresh_uci_h
#define JFresh_uci_h

#include "Board.h"

#define ENGINE_VERSION "0.1a"
#define MAX_BUFFER_LENGTH 2000 // Maximum length of input buffer
#define MAX_TOKEN_LENGTH 80 // Maximum lenght of a token

void readLine           (char *ptr); // Reads text from the stdin buffer. This will be commands sent from the UI to the engine
char *parseToken        (char *line, char *token); // Reads the next token from a line of text. Returns a pointer to where it stopped reading in the line of text

char *parsePosition     (char *line); // parses the "position" command

char *parseMoves        (char *line); // parse a series of moves
char *parseGo           (char *line);

int uciLoop             (); // Main loop of the program
void init               (); // initialize any data that is necessary for the program
void quit               (); // releases any memory necessary before quitting the program;
Board_t *shared_board   ();

#endif
