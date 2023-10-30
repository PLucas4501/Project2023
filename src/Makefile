INCLUDE = ../include
CC = g++
CFLAGS = -Wall -I$(INCLUDE)
OBJ = main.o distance.o binary_heap.o
TGT = KNN

all: $(TGT)

$(TGT): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TGT)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o

distance.o: distance.cpp
	$(CC) $(CFLAGS) -c distance.cpp -o distance.o

binary_heap.o: binary_heap.cpp
	$(CC) $(CFLAGS) -c binary_heap.cpp -o binary_heap.o


clean:
	rm -f *.o ./$(TGT)

run: all
	./$(TGT)