//
//  uci.c
//  JFresh
//
//  Created by Christian on 12/5/13.
//  Copyright (c) 2013 Christian Daley. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "Uci.h"
#include "Debug.h"
#include "Move.h"
#include "Undo.h"
#include "MoveGen.h"
#include "Fen.h"
#include "Bitscan.h"
#include "Perft.h"
#include "Bitmask.h"
#include "MagicNumberGen.h"
#include "MagicMoves.h"
#include "Attack.h"
#include "Piece.h"
#include "Search.h"
#include "Thread.h"
#include "SearchOptions.h"
#include "UciOptions.h"
#include "TT.h"
#include "Eval.h"
#include "Log.h"
#include "Sort.h"

static Board_t *board;
static SearchOptions_t *options;

void readLine(char *ptr) { // Reads text from UI
    
    fgets(ptr, MAX_BUFFER_LENGTH, stdin); // get the line of text from the input stream
    fflush(stdin);
    
    if (shared_engine_options()->log)
        write_log(0, ptr);
}

char *parseToken(char *line, char *token) {
    int loc = 0;
    while (line[loc] != ' ' && line[loc] != '\n' && line[loc] != '\0') { // go until a space, or end of line is reached
        token[loc] = line[loc]; // copy the text into the token
        loc++;
    }
    
    token[loc] = '\0';
    
    while (line[loc] == ' ' || line[loc] == '\n')
        loc++;
    
    return line + loc;
}

int uciLoop() { // Main loop
    setDefaults();
    printEngineInfo();
    
    char *line = malloc(sizeof(char) * MAX_BUFFER_LENGTH); // the current line of text
    char *origLine = line;
    char token[MAX_TOKEN_LENGTH]; // The current token
    
    board = malloc(BoardSize); // make the board
    options = shared_search_options();
    resetBoard(board);
    
    init(); // initialize any data needed
        
    while (strcmp(token, "quit") != 0) { // Loop until the quit command is recieved
        readLine(line);
        
        while (strlen(line) != 0) { // Go through each token in the line of text
            line = parseToken(line, token);
            
            if (strcmp(token, "uci") == 0) { // Give engine info
                send("id name JFresh\n");
                send("id author Christian Daley\n");
                printOptions();
                send("uciok\n");
                
            } else if (strcmp(token, "isready") == 0) { // readyok
                send("readyok\n");
                
            } else if (strcmp(token, "position") == 0) { // position command recieved
                reset_undo();
                
                if (options->pondering) {
                    killSearch();
                    usleep(UPDATE_SPEED);
                    undoMove(&board);
                }
                
                line = parsePosition(line);
                
            } else if (strcmp(token, "ucinewgame") == 0) {
                killSearch();
                usleep(SLEEP_TIME);
                resetBoard(board);
                clear_tt();
            } else if (strcmp(token, "undo") == 0) { // undoes the previous move made
                undoMove(&board);
                
            } else if (strcmp(token, "board") == 0) { // prints out the current board
                line = parseToken(line, token);
                if (strcmp(token, "array") == 0)
                    printBoardArray(board);
                else if (strcmp(token, "bits") == 0)
                    printBoardBits(board);
                
            } else if (strcmp(token, "info") == 0) { // prints out board info
                printBoardInfo(board);
                
            } else if (strcmp(token, "perft") == 0) { // perft testing
                if (shared_TT()->length == 0)
                    init_tt();
                
                clear_tt();
                board->key = position_hash(board);
                line = parseToken(line, token);
                int depth = atoi(token);
                double time;
                uint64_t nodes;
                struct timeval start, end;
                
                gettimeofday(&start, NULL);
                
                send("\nPerft depth %d calculating...\n", depth);
                
                nodes = perft(board, depth - 1);
                gettimeofday(&end, NULL);
                
                send("Result: %llu nodes\n", nodes);
                
                time = end.tv_sec - start.tv_sec + (double)(end.tv_usec - start.tv_usec) / (1000000.0);
                
                send("completed in %.2lf ms\n", time * 1000);
                send("%d nodes/second\n", (int)(nodes / time));
                clear_tt();
                
            } else if (strcmp(token, "check") == 0) { // see if the position is check
                send("%s\n", isCheck(board, board->turn) ? "Check" : "Not Check");
                
            } else if (strcmp(token, "list") == 0) {
                line = parseToken(line, token);
                Move_t moves[MAX_NUM_MOVES];
                int n = -1;
                
                if (strcmp(token, "legal") == 0)
                    n = generateLegalMoves(board, moves);
                else if (strcmp(token, "capt") == 0)
                    n = generateCaptures(board, moves);
                
                if (n != -1)
                    printMoveList(moves, n);
                
            } else if (strcmp(token, "go") == 0) {
                line = parseGo(line);
            } else if (strcmp(token, "stop") == 0) {
                killSearch();
                if (options->pondering) {
                    options->pondering = 0;
                    usleep(UPDATE_SPEED);
                    undoMove(&board);
                }
                
            } else if (strcmp(token, "setoption") == 0) {
                line = setOptions(line);
            } else if (strcmp(token, "ponderhit") == 0) {
                options->pondering = 0;
            } else if (strcmp(token, "eval") == 0) {
                send("Eval: %d\n", eval(board));
            } else if (strcmp(token, "see") == 0) {
                line = parseToken(line, token);
                int from = getBoardlocation(token[0], token[1]);
                int to = getBoardlocation(token[2], token[3]);
                
                Move_t capt = getMove(board, from, to, token[4]);
                if (!MOVE_IS_CAPTURE(capt))
                    send("Specified move is not a capture\n");
                else
                    send("SEE value: %d\n", see(board, capt));
                
                
            } else if (strcmp(token, "quies") == 0) {
                
            } else if (strcmp(token, "quit") != 0) {
                send("Unknown Command: %s\n", token);
                line[0] = '\0';
            }
            
        }
        
        line = origLine;
        
        checkBoardForErrors(board);
        
        fflush(stdin);
    }
    
    free(line);
    quit();
    
    return 0;
}

char *parsePosition(char *line) { // parses a given position
    char token[MAX_TOKEN_LENGTH];
    line = parseToken(line, token);
    
    if (strcmp(token, "startpos") == 0) { // startpos command
        resetBoard(board);
        line = parseToken(line, token);
    } else if (strcmp(token, "fen") == 0) { // fen string
        line = parseFen(line, board);
    }
    
    while (line[0] != '\0' && strcmp(token, "moves") != 0) {
        line = parseToken(line, token);
    }
    
    if (strcmp(token, "moves") == 0) // list of moves to be made
        line = parseMoves(line);
    
    setKingLocations(board); // set the location for the kings
    
    return line;
}


char *parseMoves(char *line) {
    
    char token[MAX_TOKEN_LENGTH];
    Move_t move;
    
    while (strlen(line) != 0) { // go until all moves are read
        
        line = parseToken(line, token);
        
        int from = getBoardlocation(token[0], token[1]); // get the to and from spaces
        int to = getBoardlocation(token[2], token[3]);
                
        move = getMove(board, from, to, token[4]); // get the move
        TTableKey temp = position_hash(board);
        temp = move_hash(board, move, temp);
        
        makeMove(move, &board); // make the move
        board->key = position_hash(board);
        
        if (temp != board->key) {
            send("Error: Move hashed incorrectly\n");
            exit(1);
        }
        
    }
        
    return line;
}

char *parseGo(char *line) {
    char token[MAX_TOKEN_LENGTH];
    init_options();
    options->mode = SearchModeTime;
    
    do {
        
        line = parseToken(line, token);
        
        if (strcmp(token, "wtime") == 0) {
            line = parseToken(line, token);
            options->wTime = atoi(token);
        } else if (strcmp(token, "btime") == 0) {
            line = parseToken(line, token);
            options->bTime = atoi(token);
        } else if (strcmp(token, "winc") == 0) {
            line = parseToken(line, token);
            options->wInc = atoi(token);
        } else if (strcmp(token, "binc") == 0) {
            line = parseToken(line, token);
            options->bInc = atoi(token);
        } else if (strcmp(token, "movestogo") == 0) {
            line = parseToken(line, token);
            options->movesToGo = atoi(token);
        } else if (strcmp(token, "depth") == 0) {
            line = parseToken(line, token);
            options->mode = SearchModeDepth;
            
            options->info = atoi(token);
        } else if (strcmp(token, "nodes") == 0) {
            line = parseToken(line, token);
            options->mode = SearchModeNodes;
            
            options->info = atoll(token);
        } else if (strcmp(token, "mate") == 0) {
            line = parseToken(line, token);
            options->mode = SearchModeMate;
            options->info = atoi(token);
        } else if (strcmp(token, "movetime") == 0) {
            line = parseToken(line, token);
            options->info = atoi(token);
        } else if (strcmp(token, "infinite") == 0) {
            options->mode = SearchModeInfinite;
        } else if (strcmp(token, "ponder") == 0) {
            options->pondering = 1;
        } else if (strcmp(token, "searchmoves") == 0) {
            int from, to, n = 0;
            
            while (line[0] != '\0') {
                line = parseToken(line, token);
                
                from = getBoardlocation(token[0], token[1]);
                to = getBoardlocation(token[2], token[3]);
                options->searchMoves[n] = getMove(board, from, to, token[4]);
                n++;
            }
            
            options->searchMoves[n] = NULL_MOVE;
        }
        
        
    } while(line[0] != '\0');
        
    startSearch();
    
    return line;
}

void init() {
    init_bitmasks();
    init_magicMoves();
    init_undo(board);
    init_deltaIncs();
    init_eval();
}

void quit() {
    free_undo();
    dealloc_tt();
    dealloc_eval();
}

Board_t *shared_board() {
    return board;
}
