#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "../include/raw_mode.h"
#include "../include/input.h"
#include "../include/output.h"
#include <sys/ioctl.h>


extern struct editorConfig E;


/**
 * @brief This function is used to insert new line
 * in the editor
 * @param at The y position of the cursor
 * @param s The content of the file after the cursor position
 * @param len Size of the content after the cursor
 */
void insert_newline(int at, char *s, size_t len) {
    if (at < 0 || at > E.numrows) return;

    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));

    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    update_row(&E.row[at]);

    E.numrows++;
    E.dirty++;
}

/**
 * @brief This function is used to open a file in
 * the editor
 * @param filename Name of the file to open
 */
void editor_open(char *filename) {
    free(E.filename);
    E.filename = strdup(filename);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        E.filename = filename;
        return;
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                               line[linelen - 1] == '\r')) {
            linelen--;
        }
        insert_newline(E.numrows, line, linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
}

/**
 * @brief This function is used retrieve cursor's position
 * @param rows The x position of the cursor
 * @param cols The y position of the cursor
 * @return 0 on success
 */
int get_cursor(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}

/**
 * @brief This function is used to get the size of the editor window
 * @param rows Number of rows in editor without using scroll
 * @param cols Number of columns in editor without using scroll
 * @return 0 on success
 */
int get_size(int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return get_cursor(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/**
 * Initialize the editor structure
 */
void initalize_editor(){
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.mode = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;

    if (get_size(&E.screenrows, &E.screencols) == -1){
        die("getWindowSize");
    }
    E.screenrows -= 2;
}

/**
 * @mainpage Main function which seats in infinite loop
 * listining for the keyboard event
 * @param argc Number of command line argument
 * @param argv String array of command line arguments
 * @return Never returns from main has to exit using exit() call
 */
int main(int argc, char *argv[]){
    raw_mode();
    initalize_editor();
    if (argc >= 2) {
        editor_open(argv[1]);
    }
    show_status_message("Guide: Ctrl-S = save | Ctrl-f = search | Ctrl-Q = quit");
    while(1){
        refresh_screen();
        int c = read_key();
        process_keypress(c);
    }

    return 0;
}