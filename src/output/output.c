
#include "../../include/raw_mode.h"
#include "../../include/input.h"
#include "../../include/output.h"

extern struct editorConfig E;
void show_status_message(const char *fmt, ...) {
    va_list append;
    va_start(append, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, append);
    va_end(append);
    E.statusmsg_time = time(NULL);
}

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

void append_string(erow *row, char *s, size_t len) {
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    update_row(row);
    E.dirty++;
}
void delete_row(int at) {
    if (at < 0 || at >= E.numrows) return;
    free((&E.row[at])->render);
    free((&E.row[at])->chars);
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
    E.numrows--;
    E.dirty++;
}

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
void save(){
    if (E.filename == NULL) {
        E.filename = input_prompt("Save as: %s (ESC to cancel)");
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