

bloc: src/editor.c src/raw/raw_mode.c src/input/input.c src/output/output.c
	$(CC) src/editor.c src/raw/raw_mode.c src/input/input.c src/output/output.c -o bin/bloc -D_XOPEN_SOURCE=500 -Wall -Wextra -pedantic -std=c99

clean :
	rm -rf bin/*