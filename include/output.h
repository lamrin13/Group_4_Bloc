#ifndef __OUTPUT_H
#define __OUTPUT_H
#include <stdarg.h>
#include <time.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include "./raw_mode.h"

#define _BSD_SOURCE
#define _GNU_SOURC

void show_status_message(const char *fmt, ...);
void insert_row(int at, char *s, size_t len);
void update_row(erow *row);
void append_string(erow *row, char *s, size_t len);
void delete_row(int at);
void scroll();
void save();
#endif