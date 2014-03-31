//
//  Thread.c
//  JFresh
//
//  Created by Christian on 1/2/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "Thread.h"
#include "Search.h"
#include "Debug.h"
#include "SearchResults.h"
#include "Uci.h"
#include "Log.h"
 
static uint64_t nodes;

static pthread_t searchThread;
static pthread_t monitorThread;

void startSearch() {
        
    gettimeofday(&shared_results()->startTime, NULL);
    
    int fail;
    
    fail = pthread_create(&searchThread, NULL, &search, NULL);
    
    if (fail) {
        send("Failed to create thread for search algorithm\n");
        exit(fail);
    }
    
    fail = pthread_create(&monitorThread, NULL, &monitorSearch, NULL);
    
    if (fail) {
        send("Failed to create thread for monitor algorithm\n");
        exit(fail);
    }
    
}

void *monitorSearch(void *args) {
    nodes = 0;
    int time = 0;
    
    
    while (shared_search_options()->stop == 0) {
        
        usleep(SLEEP_TIME);
        time += SLEEP_TIME;
        
        if (time >= UPDATE_SPEED) {
            printSearchInfo();
            time = 0;
        }
        
        checkStop(shared_search_options());
    }
        
    return NULL;
}

void checkStop(SearchOptions_t *options) {
    
    SearchResults_t *results = shared_results();
    
    if (options->stop == 1)
        return;
    
    if (options->mode == SearchModeDepth) {
        options->stop = results->currentDepth > options->info;
    }
    
    if (options->mode == SearchModeTime) {
        options->stop = msec_since(results->startTime) >= options->info;
        options->terminated = options->stop;
    }
    
    if (options->mode == SearchModeNodes) {
        options->stop = results->nodesSearched >= options->info;
        options->terminated = options->stop;
    }
    
}

void killSearch() {
    shared_search_options()->stop = 1;
    shared_search_options()->terminated = 1;
}

int msec_since(struct timeval start) {
    struct timeval now;
    int elapased;
    
    gettimeofday(&now, NULL);
    
    elapased = (int)(now.tv_sec - start.tv_sec) * 1000;
    elapased += (now.tv_usec - start.tv_usec) / 1000;
    
    return elapased;
}