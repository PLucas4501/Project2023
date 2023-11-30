INCLUDE = ../include
SRC = ../src/
CC = g++
CFLAGS = -Wall -I$(INCLUDE)
OBJ = data_structures.o

all: data_structures clean

data_structures: data_structures.o
	$(CC) $(CFLAGS) data_structures.o -o data_structures


data_structures.o: data_structures.cpp
	$(CC) $(CFLAGS) -c data_structures.cpp -o data_structures.o

clean:
	rm -f *.o

run: all
	./data_structures