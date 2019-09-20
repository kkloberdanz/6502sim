CFLAGS=-std=iso9899:1990
WARN_FLAGS=-Wall -Wextra -Wpedantic -Werror
CLANG=clang -Wassign-enum -Wenum-conversion
SANITIZE=-fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined

OBJS=6502sim

release: OPTIM_FLAGS=-Os
release: test

debug: OPTIM_FLAGS=-Og -ggdb -DDEBUG $(SANITIZE)
debug: test

valgrind: OPTIM_FLAGS=-Og -ggdb -DDEBUG
valgrind: test

CC=cc $(OPTIM_FLAGS) $(CFLAGS) $(WARN_FLAGS)

loc: clean
	find . -path '*/.*' -prune -o -type f -exec sloccount {} \+

test: 6502sim assembly
	./6502sim a.o65

6502sim:
	$(CC) -o 6502sim 6502sim.c

assembly:
	xa test_xa.s

lint:
	clang-tidy 6502sim.c -- -Wall -Wextra -Wpedantic -Wassign-enum \
		-Wenum-conversion -std=iso9899:1990

clean:
	rm -f *.o
	rm -f *.so
	rm -f 6502sim
	rm -f *.o65
