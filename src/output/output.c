
#include "../../include/raw_mode.h"
#include "../../include/input.h"
#include "../../include/output.h"

extern struct editorConfig E;

/**
 * @brief This function displays message at the bottom of
 * the editor window
 * @param fmt Fixed part of the varadic function argument
 * @param ... Variable part of the varadic function argument
 */
void show_status_message(const char *fmt, ...) {
    va_list append;
    va_start(append, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, append);
    va_end(append);
    E.statusmsg_time = time(NULL);
}

/**
 * @brief This function is used to insert a new row in the
 * editor window
 * @param at The y position of the cursor
 * @param s The content of the file after the cursor
 * @param len The length of the content after the cursor
 */
void insert_row(int at, char *s, size_t len) {
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
 * @brief This function is used when a row is
 * updated after being written once
 * @param row The row data structure
 */
void update_row(erow *row) {
    int tabs = 0;
    int j;
    for (j = 0; j < row->size; j++)
        if (row->chars[j] == '\t') tabs++;

    free(row->render);
    row->render = malloc(row->size + tabs*(TAB - 1) + 1);

    int idx = 0;
    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') {
            row->render[idx++] = ' ';
            while (idx % TAB != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;
}

/**
 * @brief This function is used to appened a string
 * to an existing row
 * @param row The row data structre of the row to be updated
 * @param s The string to append
 * @param len length of the current row
 */
void append_string(erow *row, char *s, size_t len) {
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    update_row(row);
    E.dirty++;
}

/**
 * @brief This function is used to delete a row
 * all the content below the row will be shifted upwards
 * @param at The y position of the row to be deleted
 */
void delete_row(int at) {
    if (at < 0 || at >= E.numrows) return;
    free((&E.row[at])->render);
    free((&E.row[at])->chars);
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
    E.numrows--;
    E.dirty++;
}

/**
 * This function is used to get the position of a row
 * @param row the row data structure
 * @param cx The x position of the cursor
 * @return returns int value which is position of the row
 */
int row_postion(erow *row, int cx) {
    int rx = 0;
    int j;
    for (j = 0; j < cx; j++) {
        if (row->chars[j] == '\t')
            rx += (TAB - 1) - (rx % TAB);
        rx++;
    }
    return rx;
}

/**
 * This function is used to scroll the editor
 * window when the file content overflows x or y
 * dimension of the window
 */
void scroll() {
    E.rx = 0;
    if (E.cy < E.numrows) {
        E.rx = row_postion(&E.row[E.cy], E.cx);
    }

    if (E.cy < E.rowoff) {
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows) {
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if (E.rx < E.coloff) {
        E.coloff = E.rx;
    }
    if (E.rx >= E.coloff + E.screencols) {
        E.coloff = E.rx - E.screencols + 1;
    }
}

/**
 * @brief This function is used to save the file content
 * the data is stored in a single buffer
 * @param buffer_length The length of the buffer to be returned
 * @return Buffer containing the whole file data.
 */
char *data_to_buffer(int *buffer_length) {
    int total_length = 0;
    int j;
    for (j = 0; j < E.numrows; j++)
        total_length += E.row[j].size + 1;
    *buffer_length = total_length;

    char *buffer = malloc(total_length);
    char *p = buffer;
    for (j = 0; j < E.numrows; j++) {
        memcpy(p, E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }

    return buffer;
}
/**
 * This function is called to save the editor content
 * to the file system
 */
void save(){
    if (E.filename == NULL) {
        E.filename = input_prompt("Save as: %s (ESC to cancel)",NULL);
        if (E.filename == NULL) {
            show_status_message("File Name cannot be empty.");
            return;
        }
    }

    int len;
    char *buf = data_to_buffer(&len);

    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        if (ftruncate(fd, len) != -1) {
            if (write(fd, buf, len) == len) {
                close(fd);
                free(buf);
                E.dirty = 0;
                show_status_message("File Saved Successfully. Press Ctrl+Q to quit.");
                return;
            }
        }
        close(fd);
    }

    free(buf);
}

/**
 * This function returns x position of the cursor
 * @param row The row data structure in which cursor is
 * @param rx The row number in which cursor is
 * @return cx cursor's x position
 */
int cusroor_xposition(erow *row, int rx) {
    int cur_rx = 0;
    int cx;
    for (cx = 0; cx < row->size; cx++) {
        if (row->chars[cx] == '\t')
            cur_rx += (TAB - 1) - (cur_rx % TAB);
        cur_rx++;

        if (cur_rx > rx) return cx;
    }
    return cx;
}

/**
 * This is a helper function for search operation
 * it searchs for keyword in the editor window
 * @param query The keyword to search
 * @param key The input from editor prompt (Esacpe, arrow key or Enter)
 */
void search_helper(char *query, int key) {
    printf("Searching for %s\n",query);
    static int last_match = -1;
    static int direction = 1;

    if (key == '\r' || key == '\x1b') {
        last_match = -1;
        direction = 1;
        return;
    } else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
        direction = 1;
    } else if (key == ARROW_LEFT || key == ARROW_UP) {
        direction = -1;
    } else {
        last_match = -1;
        direction = 1;
    }

    if (last_match == -1) direction = 1;
    int current = last_match;
    int i;
    for (i = 0; i < E.numrows; i++) {
        current += direction;
        if (current == -1) current = E.numrows - 1;
        else if (current == E.numrows) current = 0;

        erow *row = &E.row[current];
        char *match = strstr(row->render, query);
        if (match) {
            last_match = current;
            E.cy = current;
            E.cx = cusroor_xposition(row, match - row->render);
            E.rowoff = E.numrows;
            break;
        }
    }
}

/**
 * The search function
 * first stores current cursor position
 * then calls input prompt to take input from user
 * on exit restores cursor position if Escape is pressed
 */
void search() {
    int saved_cx = E.cx;
    int saved_cy = E.cy;
    int saved_coloff = E.coloff;
    int saved_rowoff = E.rowoff;

    char *query = input_prompt("Search: %s (Use ESC/Arrows/Enter)",
                               search_helper);

    if (query) {
        free(query);
    } else {
        E.cx = saved_cx;
        E.cy = saved_cy;
        E.coloff = saved_coloff;
        E.rowoff = saved_rowoff;
    }
}