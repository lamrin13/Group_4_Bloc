DIRS=build bin
$(shell mkdir -p $(DIRS))

bloc: src/editor.c src/raw/raw_mode.c src/input/input.c src/output/output.c
	$(CC) src/editor.c src/raw/raw_mode.c src/input/input.c src/output/output.c -o bin/bloc -D_XOPEN_SOURCE=500 -Wall -Wextra -pedantic -std=c99

test: src/raw/raw_mode.c src/input/input.c src/output/output.c test/src/test.c
	$(CC) src/raw/raw_mode.c src/input/input.c src/output/output.c test/src/test.c -o bin/tests -D_XOPEN_SOURCE=500 -Wall -Wextra -pedantic -std=c99

clean :
	rm -rf bin build