bloc: src/editor.c src/raw/raw_mode.c src/input/input.c src/output/output.c
	$(CC) src/editor.c src/raw/raw_mode.c src/input/input.c src/output/output.c -o bloc -Wall -Wextra -pedantic -std=c99