INCLUDE = ../include
CC = g++
CFLAGS = -Wall -I$(INCLUDE)
OBJ = main.o distance.o heap.o
TGT = KNN

all: $(TGT)

$(TGT): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TGT)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o

distance.o: distance.cpp
	$(CC) $(CFLAGS) -c distance.cpp -o distance.o

heap.o: heap.cpp
	$(CC) $(CFLAGS) -c heap.cpp -o heap.o


clean:
	rm -f *.o ./$(TGT)

run: all
	./$(TGT)