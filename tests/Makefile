INCLUDE = ../include
SRC = ../src/
CC = g++
CFLAGS = -Wall -I$(INCLUDE)

all: TESTS

TESTS: TESTS.o
	$(CC) $(CFLAGS) $< -o $@

TESTS.o: TESTS.cpp
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f *.o

run: all
	./TESTS