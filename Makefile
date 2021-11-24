CC=gcc

bloc: src/editor.c src/raw/raw_mode.c
	$(CC) src/editor.c src/raw/raw_mode.c -o bloc -Wall -Wextra -pedantic -std=c99