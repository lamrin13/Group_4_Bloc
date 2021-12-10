#ifndef __INPUT_H
#define __INPUT_H

#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define ESC 27
#define ENTER '\r'
#define ABUF_INIT {NULL, 0}

enum editorKey {
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

struct abuf {
    char *b;
    int len;
};
void refresh_screen();
void append(struct abuf *ab, const char *s, int len);
void display_message(struct abuf *ab);
void display_rows(struct abuf *ab);
int read_key();
void insert_new_line();
void process_keypress(int c);
char *input_prompt(char *prompt);
#endif