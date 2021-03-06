#include "../../include/input.h"
#include "../../include/raw_mode.h"
#include "../../include/output.h"

struct editorConfig E;

/**
 * Refelect the buffer changes on the editoe screen
 */
void refresh_screen() {
    scroll();
    struct abuf ab = ABUF_INIT;

    append(&ab, "\x1b[?25l", 6);
    append(&ab, "\x1b[H", 3);

    display_rows(&ab);
    display_message(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1,
             (E.rx - E.coloff) + 1);
    append(&ab, buf, strlen(buf));

    append(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    free((&ab)->b);
}

/**
 * Append the string to buffer for the editor
 * @param ab Buffer data structure for editor content
 * @param s String to be appended
 * @param len Length of the string to be appended
 */
void append(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void display_rows(struct abuf *ab) {
    int y;
    for (y = 0; y < E.screenrows; y++) {
        int filerow = y + E.rowoff;
        if (filerow >= E.numrows) {
            append(ab, "~", 1);
        } else {
            int len = E.row[filerow].rsize - E.coloff;
            if (len < 0) len = 0;
            if (len > E.screencols) len = E.screencols;
            append(ab, &E.row[filerow].render[E.coloff], len);
        }

        append(ab, "\x1b[K", 3);
        append(ab, "\r\n", 2);
    }
}

/**
 * This function is used to display message
 * at the bottom of the editor window
 * @param ab The message to be displayed
 */
void display_message(struct abuf *ab) {
    append(ab, "\x1b[K", 3);
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screencols) msglen = E.screencols;
    if (msglen && time(NULL) - E.statusmsg_time < 5)
        append(ab, E.statusmsg, msglen);
}

/**
 * This function is called when Enter key is pressed
 * to enter a new line.
 */
void insert_new_line() {
    if (E.cx == 0) {
        insert_row(E.cy, "", 0);
    } else {
        erow *row = &E.row[E.cy];
        insert_row(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.row[E.cy];
        row->size = E.cx;
        row->chars[row->size] = '\0';
        update_row(row);
    }
    E.cy++;
    E.cx = 0;
}

/**
 * @brief This function is a helper function to shift
 * characters on right to move left
 * @param row The row data structure
 * @param at Position of cursor of which right character is to be deleted
 */
void delete_char_helper(erow *row, int at) {
    if (at < 0 || at >= row->size) return;
    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
    update_row(row);
    E.dirty++;
}
/**
 * This function is called when Delete key or Backspace key is pressed to
 * delete a character from the editor
 */
void delete_character() {
    if (E.cy == E.numrows) return;
    if (E.cx == 0 && E.cy == 0) return;

    erow *row = &E.row[E.cy];
    if (E.cx > 0) {
        delete_char_helper(row, E.cx - 1);
        E.cx--;
    } else {
        E.cx = E.row[E.cy - 1].size;
        append_string(&E.row[E.cy - 1], row->chars, row->size);
        delete_row(E.cy);
        E.cy--;
    }
}

/**
 * This function is used to move the cursor
 * in the editor window according to key pressed
 * @param key The arrow key
 */
void move_cursor(int key) {
    erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

    switch (key) {
        case ARROW_LEFT:
            if (E.cx != 0) {
                E.cx--;
            } else if (E.cy > 0) {
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            break;
        case ARROW_RIGHT:
            if (row && E.cx < row->size) {
                E.cx++;
            } else if (row && E.cx == row->size) {
                E.cy++;
                E.cx = 0;
            }
            break;
        case ARROW_UP:
            if (E.cy != 0) {
                E.cy--;
            }
            break;
        case ARROW_DOWN:
            if (E.cy < E.numrows) {
                E.cy++;
            }
            break;
    }

    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen) {
        E.cx = rowlen;
    }
}


void insert_char_helper(erow *row, int at, int c) {
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;
    update_row(row);
    E.dirty++;
}

/**
 * This function is called when a character is inserted using
 * keyboard event
 * @param c The ASCII value of the character to be inserted
 */
void insert_character(int c) {
    if (E.cy == E.numrows) {
        insert_row(E.numrows, "", 0);
    }
    insert_char_helper(&E.row[E.cy], E.cx, c);
    E.cx++;
}

/**
 * This function is used to auto close the brackets and quotes
 * @param c The bracket or quote ASCII value
 */
void auto_complete_bracket(int c){
    if(c=='('){
        insert_character(')');
        E.cx--;
    }
    else if(c=='"'){
        insert_character('"');
        E.cx--;
    }
    else if(c=='\''){
        insert_character('\'');
        E.cx--;
    }
    else if(c=='['){
        insert_character(']');
        E.cx--;
    }
    else if(c=='<'){
        insert_character('>');
        E.cx--;
    }
    else if(c=='{'){
        erow *row = &E.row[E.cy];
        if(strchr(row->chars,'=')==NULL){
            insert_new_line();
            insert_new_line();
            insert_character('}');
            E.cx--;
            E.cy--;
            insert_character(TAB);
        }
        else {
            insert_character('}');
            E.cx--;
        }
    }
}

/**
 * This function is used to read the listen to keyboard
 * press event
 * @return The ASCII value of the key pressed
 */
int read_key() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }

    if (c == '\x1b') {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }

        return '\x1b';
    } else {
        return c;
    }
}

/**
 * This function is used to process the key pressed
 * and call the appropriate functions
 * @param c The ASCII value of key pressed
 */
void process_keypress(int c) {
    static int quit_times = 1;

    if(E.mode==0) {
        if (c == CTRL_KEY('q')){
            if (E.dirty && quit_times > 0){
                show_status_message("WARNING!!! File has unsaved changes. "
                                       "Press Ctrl-S to save chnages or, "
                                       "Press Ctrl-Q %d more time to quit.",quit_times);
                quit_times--;
                return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
        }
        else if(c == CTRL_KEY('s')){
            save();
        }
        else if(c == CTRL_KEY('f')){
            printf("searching for something...");
            search();
        }
        else if(c == 'i' || c=='I'){
            E.mode = 1;
            show_status_message("Entered in insert mode. Strat typing. Happy Coding!!");
        }
        else{
            show_status_message("Please press 'i' or 'I' to enter in insert mode.");
        }
    }
    else {
        if(c == ENTER){
            insert_new_line();
        }
        else if(c == HOME_KEY){
            E.cx = 0;
        }
        else if(c == END_KEY && E.cy < E.numrows){
            E.cx = E.row[E.cy].size;
        }
        else if(c == BACKSPACE){
            delete_character();
        }
        else if(c == DEL_KEY){
            move_cursor(ARROW_RIGHT);
            delete_character();
        }
        else if(
                c == ARROW_RIGHT ||
                c == ARROW_DOWN ||
                c == ARROW_LEFT ||
                c ==ARROW_UP
                ){
            move_cursor(c);
        }
        else if(c == ESC){
            E.mode = 0;
            show_status_message("Entered in command mode. ""Press Ctrl-s to save. ""Press Ctrl-f to search. ""Press Ctrl-q to quit.");
        }
        else if(c == CTRL_KEY('q') || c == CTRL_KEY('s')){
            show_status_message("Press ESC (escape) key to enter in command mode. No action taken.");
        }
        else{
            insert_character(c);
            if(c=='(' || c=='[' || c=='<' || c=='{' || c=='"' || c=='\''){
                auto_complete_bracket(c);
            }
        }
    }
    quit_times = 1;
}
/**
 * This function is used to take input from user
 * in command mode
 * @param prompt The prompt text to display
 * @param search Callback function to be called only used for search
 * @return The input string inserted from user
 */
char *input_prompt(char *prompt, void (*search)(char *,int)) {
    size_t bufsize = 128;
    char *buf = malloc(bufsize);

    size_t buflen = 0;
    buf[0] = '\0';

    while (1) {
        show_status_message(prompt, buf);
        refresh_screen();

        int c = read_key();
        if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
            if (buflen != 0) buf[--buflen] = '\0';
        } else if (c == '\x1b') {
            free(buf);
            if (search){
                search(buf, c);
                show_status_message("");
            }
            return NULL;
        } else if (c == '\r') {
            if (buflen != 0) {
                if(search){
                    search(buf,c);
                    show_status_message("");
                }
                return buf;
            }
        } else if (!iscntrl(c) && c < 128) {
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }
        if (search){
            search(buf, c);
        }
    }
}