

#ifndef __RAW_MODE_H
#define __RAW_MODE_H

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#define TAB 9

/**
 * Editor row data structure
 */
typedef struct erow {
    int size;
    int rsize;
    char *chars;
    char *render;
    unsigned char *hl;
} erow;

/**
 * Editor configuration variables
 */
struct editorConfig {
    int cx, cy;
    int rx;
    int rowoff;
    int coloff;
    int screenrows;
    int screencols;
    int numrows;
    erow *row;
    int dirty;
    int mode;
    char *filename;
    char statusmsg[200];
    time_t statusmsg_time;
    struct termios orig_termios;
};




void die(const char *message);
void raw_mode();
void disable_raw_mode();

#endif
