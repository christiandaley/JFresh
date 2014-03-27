JFresh
======

UCI chess engine
version 0.1a by Christian Daley

JFresh is a free, open source, uci compatible chess engine written in C. JFresh can be run from either the command 
line or through a uci compatible chess interface. Currently, JFresh can only run on UNIX systems. To compile, simply
navigate to the src directory in terminal and run "make". An already compiled version is available for 64 bit mac
users.

JFresh does not play very strong (yet) and I have estimated its elo at 1800-2000.

Current Features:
* magic bitboards
* alpha-beta pruning
* iterative deepening
* principle variation search
* quiescence search
* transposition table
* mate-distance pruning
* null move pruning
* internal iterative deepening
* futility pruning
* late move reductions
* killer moves
* history heuristic
* static exchange evaluation
* delta pruning
* pawn structure hash table
* perft
* logging

Currently working on:
* king tropism in the eval function
* better draw detection
* getting pondering to work properly

The log file for JFresh will be written to the current working directory of the executable.
