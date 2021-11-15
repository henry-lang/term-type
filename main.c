#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "words.h"

#define CTRL_KEY(k) ((k) & 0x1f)

typedef struct buffer {
    char *buf;
    int len;
} buffer;
#define BUFFER_NEW {NULL, 0}

void die(const char *);

void term_size(int *, int *);
void term_no_raw(void);
void term_raw(void);
char term_get_key(void);

void buffer_append(buffer *, const char *);
void buffer_free(buffer *);

void game_refresh(void);

char *words_str(int);

struct termios orig_termios;

void die(const char *msg) {
    perror(msg);
    exit(1);
}

void term_size(int *rows, int *cols) {
    struct winsize ws;

    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        die("ioctl");
    } else {
        *rows = ws.ws_row;
        *cols = ws.ws_col;
    }
}

void term_no_raw(void) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void term_raw(void) {
    if(tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");
    atexit(term_no_raw);

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

char term_get_key(void) {
    int nread;
    char key;
    while ((nread = read(STDIN_FILENO, &key, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return key;
}

void buffer_append(buffer *buf, const char *str) {
    int len = strlen(str); // TODO: pass in size as a parameter???
    char *new = realloc(buf->buf, buf->len + len);

    memcpy(&new[buf->len], str, len);
    buf->buf = new;
    buf->len += len;
}

void buffer_free(buffer *buf) {
    free(buf->buf);
}

void game_refresh(void) {
    buffer buf = BUFFER_NEW;
    // buffer_append(&buf, "\x1b[?25l");   
    buffer_append(&buf, "\x1b[2J");
    buffer_append(&buf, "\x1b[H");

    char *str = words_str(50);
    buffer_append(&buf, str);
    free(str);

    buffer_append(&buf, "\x1b[H");
    // buffer_append(&buf, "\x1b[?25h");

    write(STDOUT_FILENO, buf.buf, buf.len);

    buffer_free(&buf);
}

char *words_str(int count) {
    char *str = malloc(count * MAX_WORD_LENGTH);
    char *current = str;
    for(int i = 0; i < count; i++) {
        const char *rand_str = words[rand() % words_size];
        int len = strlen(rand_str);
        strcpy(current, rand_str);
        current += len + 1;
        if(i != count - 1) {
            *(current - 1) = ' ';
        }
    }
    return str;
}

int main(void) {
    srand(time(NULL));
    term_raw();

    game_refresh();
    // while(term_get_key() != 'q') game_refresh();

    return 0;
}