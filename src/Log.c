//
//  Log.c
//  JFresh
//
//  Created by Christian on 1/27/14.
//  Copyright (c) 2014 Christian Daley. All rights reserved.
//

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "Log.h"
#include "UciOptions.h"
#include "Uci.h"

void send(const char *format, ...) {
    
    char text[200];
    va_list args;
    
    va_start(args, format);
    vsprintf(text, format, args);
    va_end(args);
    
    
    printf("%s", text);
    fflush(stdout);
    
    if (shared_engine_options()->log == 1)
        write_log(1, text);
    
}

void write_log(int engine, const char *text) {
    static int newLine = 1;
    static int first = 1;
    FILE *f;
    
    if (first) {
        f = fopen("log.txt", "w");
        fprintf(f, "Uci-Engine log for JFresh\n");
    } else
        f = fopen("log.txt", "a");
    
    first = 0;
    
    if (newLine == 1 && engine == 1)
        fprintf(f, "JFresh: ");
    else if (engine == 0)
        fprintf(f, "GUI: ");
    
    fprintf(f, "%s", text);
    
    if (text[strlen(text) - 1] == '\n')
        newLine = 1;
    else
        newLine = 0;
        
        
    
    fclose(f);
}