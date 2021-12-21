#include "../../include/raw_mode.h"

extern struct editorConfig E;
/**
 * @brief This function is used to exit the editor when error occurs
 * @param message The message to display while exiting
 */
void die(const char *message) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(message);
    exit(1);
}

/**
 * This function disables raw mode when exiting the editor
 */
void disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die("Exiting Raw Mode");
}
/**
 * This functions enable raw mode, which ignores all the bash commands
 */
void raw_mode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
        die("Failed to enable raw mode");
    }
    atexit(disable_raw_mode);

    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("Failed to enable raw mode");
    }
}
