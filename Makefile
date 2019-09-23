CFLAGS=-std=iso9899:1990
WARN_FLAGS=-Wall -Wextra -Wpedantic -Werror -fstrict-aliasing -Wstrict-aliasing
CLANG=clang -Wassign-enum -Wenum-conversion
GCC=gcc
SANITIZE=-fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined

OBJS=6502sim

release: OPTIM_FLAGS=-Os
release: test

debug: OPTIM_FLAGS=-O0 -ggdb -DDEBUG $(SANITIZE)
debug: test

valgrind: OPTIM_FLAGS=-O0 -ggdb -DDEBUG
valgrind: test
valgrind: VG=valgrind

CC=$(CLANG) $(OPTIM_FLAGS) $(CFLAGS) $(WARN_FLAGS)

loc: clean
	find . -path '*/.*' -prune -o -type f -exec sloccount {} \+

test: all
	$(VG) ./6502sim a.o65

all: main.o 6502sim.o assembly
	$(CC) -o 6502sim main.o 6502sim.o

main.o: clean
	$(CC) -c main.c

6502sim.o: clean
	$(CC) -c 6502sim.c

assembly: clean
	xa snake.s

lint:
	clang-tidy 6502sim.c -- -Wall -Wextra -Wpedantic -Wassign-enum \
		-Wenum-conversion -std=iso9899:1990

clean:
	rm -f *.o
	rm -f *.so
	rm -f 6502sim
	rm -f *.o65
	rm -f memory.dump
