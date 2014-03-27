//
//  Search.c
//  JFresh
//
//  Created by Christian on 1/2/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "Search.h"
#include "Eval.h"
#include "Thread.h"
#include "Uci.h"
#include "Debug.h"
#include "Attack.h"
#include "MoveGen.h"
#include "Sort.h"
#include "Move.h"
#include "Undo.h"
#include "TT.h"
#include "UciOptions.h"
#include "Piece.h"
#include "Killer.h"
#include "History.h"

static SearchResults_t *results;
static SearchOptions_t *s_options;
static EngineOptions_t *e_options;
static TTable_t *ttable;

void *search(void *args) {
    
    int alpha = -CHECKMATE;
    int beta = CHECKMATE;
    int score;
    
    results = shared_results();
    s_options = shared_search_options();
    e_options = shared_engine_options();
    ttable = shared_TT();
    
    setThinkTime();
    clear_root_scores();
    clear_history();
    results->nodesSearched = 0;
    results->mate = 0;
    s_options->stop = 0;
    
    if (ttable->length == 0)
        init_tt();
    
    for (results->currentDepth = 1; s_options->stop == 0;results->currentDepth++) {
        clear_killers();
        clear_history();
        results->selDepth = 0;
        
        score = root_search(shared_board(), results->currentDepth, alpha, beta);
        checkStop(s_options);
        
        if (score <= alpha || score >= beta) {
            if ((IS_WINNING_CHECKMATE(score) || IS_LOSING_CHECKMATE(score)) && s_options->stop == 0) {
                int n_ply = abs(CHECKMATE - abs(score));
                if (IS_WINNING_CHECKMATE(score))
                    results->mate = n_ply / 2 + 1;
                else
                    results->mate = -n_ply / 2;
                
                results->currentDepth++;
                break;
            } else {
                if (score <= alpha)
                    alpha = score - 2 * ASPIRATION_WINDOW;
                else
                    beta = score + 2 * ASPIRATION_WINDOW;
            }
            
            results->currentDepth--;
            
            continue;
        }
        
        alpha = score - ASPIRATION_WINDOW;
        beta = score + ASPIRATION_WINDOW;
    }
    
    s_options->stop = 1;
    results->currentDepth--;
    printSearchInfo();
    printPv();
    printBestMove();
    
    return NULL;
}

int root_search(Board_t *board, int depth, int alpha, int beta) {
    int value = -CHECKMATE;
    Move_t list[MAX_NUM_MOVES];
    Move_t currMove, bestMove;
    int moveCount, score;
    int old_alpha = alpha;
    int doFullSearch;
    int score_type;
    TTableKey currHash, newHash;
    currHash = position_hash(board);
    Line_t pv, local_pv;
    local_pv.count = 0;
    
    moveCount = generateLegalMoves(board, list);
    sort_root_moves(list, moveCount);
    
    for (int a = 0; a < moveCount && s_options->stop == 0;a++) {
        currMove = list[a];
        newHash = move_hash(board, currMove, currHash);
        makeMove(currMove, &board);
        board->key = newHash;
        
        results->currentMove = currMove;
        results->currentMoveNumber = a + 1;
        printCurrMove();
        
        doFullSearch = 0;
        
        if (a == 0)
            doFullSearch = 1;
        else {
            score = -full_search(board, depth - 1, -alpha - 1, -alpha, e_options->useNullMove, NodeTypeNotPV, &local_pv);
            if (score > alpha && score < beta)
                doFullSearch = 1;
        }
        
        if (doFullSearch)
            score = -full_search(board, depth - 1, -beta, -alpha, e_options->useNullMove, NodeTypePV, &local_pv);
        
        undoMove(&board);
        
        if (!MOVE_IS_QUIET(currMove) && score > alpha && score < beta) {
            if (score > 0)
                score += SIMPL_BONUS;
            else if (score < 0)
                score -= SIMPL_BONUS;
        }
        
        save_root_move(score, a);
        
        if (score > value || value == -CHECKMATE) {
            value = score;
            bestMove = currMove;
            
            if (value <= alpha)
                score_type = SCORE_UPPER;
            else if (value >= beta)
                score_type = SCORE_LOWER;
            else // alpha < value < beta
                score_type = SCORE_EXACT;
            
            if (value > alpha) {
                alpha = value;
                pv.moves[0] = bestMove;
                memcpy(pv.moves + 1, local_pv.moves, local_pv.count * sizeof(Move_t));
                pv.count = local_pv.count + 1;
            }
        }
    }
    
    add_to_tt(currHash, HASH_FLAG(depth, score_type), value, bestMove);
    
    if ((value > old_alpha && value < beta && s_options->stop == 0) || (IS_WINNING_CHECKMATE(value) && s_options->stop == 0)) {
        results->score = value;
        memcpy(results->pv, pv.moves, pv.count * sizeof(Move_t));
        results->pv[pv.count] = NULL_MOVE;
        printPv();
    }
    
    return value;
}

int full_search(Board_t *board, int depth, int alpha, int beta, int useNullMove, NodeType ntype, Line_t *pv) {
    int value = -CHECKMATE;
    int check, moveCount, score;
    int old_alpha = alpha;
    int doFullSearch;
    Move_t list[MAX_NUM_MOVES];
    Move_t currMove, bestMove = NULL_MOVE, tt_move = NULL_MOVE;
    Line_t local_pv;
    TTableKey newHash;
    TTableFlag tt_flags;
    TTableScore tt_score;
    TTableKey currHash = board->key;
    
    if (depth <= 0) {
        //return eval(board);
        return quiescenceSearch(board, alpha, beta, 0);
    }
    
    check = isCheck(board, board->turn);
    moveCount = generateLegalMoves(board, list);
    local_pv.count = 0;
    
    if (moveCount == 0)
        return check ? -CHECKMATE + (results->currentDepth - depth) : DRAW;
    
        
    if (get_from_tt(currHash, &tt_flags, &tt_score, &tt_move)) {
        if (ENTRY_DEPTH(tt_flags) >= depth) {
            if (ENTRY_TYPE(tt_flags) == SCORE_EXACT)
                return (int)tt_score;
            else if (ENTRY_TYPE(tt_flags) == SCORE_LOWER) {
                if (tt_score >= beta && ntype != NodeTypePV)
                        return (int)tt_score;
            } else if (ENTRY_TYPE(tt_flags) == SCORE_UPPER) {
                if (tt_score <= alpha && ntype != NodeTypePV)
                    return (int)tt_score;
            }
        }
    }
    
    if (results->mate != 0) {
        // mate distance pruning
        // upper bound
        int mating_value = CHECKMATE - (results->currentDepth - depth);
        if (mating_value < beta) {
            beta = mating_value;
            if (alpha >= beta)
                return beta;
        }
        
        // lower bound
        mating_value = -CHECKMATE + (results->currentDepth - depth);
        if (mating_value > alpha) {
            alpha = mating_value;
            if (alpha >= beta)
                return beta;
        }
        
    }
    
    if (useNullMove && !check && ntype != NodeTypePV && depth > NULL_MOVE_REDUCTION) {
        make_null_move(&board);
        newHash = position_hash(board);
        board->key = newHash;
        score = -full_search(board, depth - 1 - NULL_MOVE_REDUCTION, -beta, -beta + 1, 0, NodeTypeNotPV, &local_pv);
        undoMove(&board);
        
        if (verify_null(board) && depth >= NULL_VER_DEPTH && score >= beta) {
            e_options->useNullMove = 0; // prevent use of null move pruning further in the game tree
            score = full_search(board, depth - 1 - NULL_VER_REDUCTION, alpha, beta, 0, ntype, &local_pv);
            e_options->useNullMove = 1;
        }
        
        if (score >= beta && !IS_WINNING_CHECKMATE(score))
            return beta;
    }
    
    if (depth >= IID_DEPTH && tt_move == NULL_MOVE && ntype == NodeTypePV) {
        internal_iterative(board, depth - IID_REDUCTION, alpha, beta, 0, list, moveCount);
    } else {
        sort_moves(board, list, tt_move, moveCount, depth);
    }
    
    for (int a = 0;a < moveCount && s_options->stop == 0;a++) {
        currMove = list[a];
        
        if (depth <= 3 && !check && results->mate == 0 && ntype != NodeTypePV) {
            // futility pruning
            if (MOVE_IS_QUIET(currMove) && eval(board) + FUTILITY_MARGINS[results->currentDepth - depth] <= alpha)
                continue;
            else if (!MOVE_IS_QUIET(currMove) && see(board, currMove) + FUTILITY_MARGINS[results->currentDepth - depth] <= alpha)
                continue;
        }
        
        newHash = move_hash(board, currMove, currHash);
        makeMove(currMove, &board);
        board->key = newHash;
        
        /*if (ntype != NodeTypePV && a > 0) {
            int reduc;
            if (e_options->lmrAllowed && !check && results->currentDepth - depth >= LMR_DEPTH && a >= LMR_COUNT)
                reduc = LMR_REDUCTION;
            else
                reduc = 0;
            
            score = -full_search(board, depth - 1 - reduc, -alpha - 1, -alpha, e_options->useNullMove, NodeTypeNotPV, &local_pv);
            if (score > alpha)
                doFullSearch = 1;
        } else
            doFullSearch = 1;*/
        
        doFullSearch = 0;
        
        if (ntype == NodeTypePV || a == 0) {
            doFullSearch = 1;
        } else {
            if (e_options->lmrAllowed && !check && results->currentDepth - depth >= LMR_DEPTH && a >= LMR_COUNT)
                score = -full_search(board, depth - 1 - LMR_REDUCTION, -alpha - 1, -alpha, e_options->useNullMove, NodeTypeNotPV, &local_pv);
            else
                score = -full_search(board, depth - 1, -alpha - 1, -alpha, e_options->useNullMove, NodeTypeNotPV, &local_pv);
            
            if (score > alpha && score < beta)
                doFullSearch = 1;
        }
        
        if (doFullSearch)
            score = -full_search(board, depth - 1, -beta, -alpha, e_options->useNullMove, NodeTypePV, &local_pv);
        
        undoMove(&board);
        
        if (!MOVE_IS_QUIET(currMove) && score > alpha && score < beta) { // encourage trades when up in material, discourage when down
            if (score > 0)
                score += SIMPL_BONUS;
            else if (score < 0)
                score -= SIMPL_BONUS;
        }
        
        if (score > value) {
            value = score;
            if (value > alpha) {
                bestMove = currMove;
                alpha = value;
                
                if (value >= beta) {
                    add_killer(bestMove, results->currentDepth - depth);
                    add_history(bestMove, depth);
                    break;
                }
                
                pv->moves[0] = bestMove;
                memcpy(pv->moves + 1, local_pv.moves, local_pv.count * sizeof(Move_t));
                
                pv->count = local_pv.count + 1;
            }
        }
    }
    
    if (value <= old_alpha)
        tt_flags = HASH_FLAG(depth, SCORE_UPPER);
    else if (value >= beta)
        tt_flags = HASH_FLAG(depth, SCORE_LOWER);
    else
        tt_flags = HASH_FLAG(depth, SCORE_EXACT);
    
    add_to_tt(currHash, tt_flags, value, bestMove);
    
    
    return value;
}

int quiescenceSearch(Board_t *board, int alpha, int beta, int depth) {
    Move_t list[MAX_NUM_MOVES];
    Move_t move;
    int moveCount, check, static_eval, score;
    TTableKey newHash;

    moveCount = generateLegalMoves(board, list);
    check = isCheck(board, board->turn);
    
    if (results->selDepth < results->currentDepth - depth)
        results->selDepth = results->currentDepth - depth;
    
    if (moveCount == 0)
        return check ? -CHECKMATE + results->currentDepth - depth : DRAW;
    
    static_eval = eval(board);
    results->nodesSearched++;
    
    if (static_eval >= beta && check == 0)
        return beta;
    if (static_eval > alpha && check == 0)
        alpha = static_eval;
    
    sort_moves(board, list, NULL_MOVE, moveCount, results->currentDepth - depth);
    
    for (int a = 0;a < moveCount;a++) {
        move = list[a];
        if (MOVE_IS_QUIET(move) && check == 0)
            continue;
        if (!MOVE_IS_PROMOTION(move) && check == 0 && see(board, move) + DELTA_PRUNING_MARGINS < alpha) {
            continue; // delta pruning
        }
            
        newHash = move_hash(board, move, board->key);
        makeMove(move, &board);
        board->key = newHash;
        
        score = -quiescenceSearch(board, -beta, -alpha, depth - 1);
        undoMove(&board);
        
        if (score > alpha) {
            alpha = score;
            if (alpha >= beta)
                break;
        }
    }
    
    
    return alpha;
    
}

int internal_iterative(Board_t *board, int depth, int alpha, int beta, int useNullMove, Move_t *list, int moveCount) {
    int value = -CHECKMATE;
    int scores[moveCount];
    int score;
    int check = isCheck(board, board->turn);
    
    if (moveCount == 0)
        return check ? -CHECKMATE : DRAW;
    
    if (depth <= 0) {
        results->nodesSearched++;
        return eval(board);
    }
    
    Move_t legal[MAX_NUM_MOVES];
    Move_t move;
    int legalCount;
    
    if (check == 0 && useNullMove) {
        make_null_move(&board);
        board->key = position_hash(board);
        legalCount = generateLegalMoves(board, legal);
        score = -internal_iterative(board, depth - 1 - NULL_MOVE_REDUCTION, -beta, -beta + 1, 0, legal, legalCount);
        
        undoMove(&board);
        
        if (score >= beta)
            return beta;
    }
    
    for (int a = 0;a < moveCount && s_options->stop == 0;a++) {
        move = list[a];
        makeMove(move, &board);
        board->key = position_hash(board);
        legalCount = generateLegalMoves(board, legal);
        
        score = -internal_iterative(board, depth - 1, -beta, -alpha, e_options->useNullMove, legal, legalCount);
        undoMove(&board);
        
        scores[a] = score;
        
        if (score > value)
            value = score;
        if (value > alpha)
            alpha = value;
        if (alpha >= beta)
            break;
    }
    
    if (depth == IID_DEPTH)
        sort_scores(list, scores, moveCount);
    
    return value;
}

int verify_null(Board_t *board) {
    Bitboard pieces = FRIENDLY(board->turn);
    
    pieces &= pieces - 1;
    
    if (pieces == 0)
        return 1;
    
    pieces &= pieces - 1;
    
    return pieces == 0;
}