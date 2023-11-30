INCLUDE = ../include
CC = g++
CFLAGS = -Wall -I$(INCLUDE)
OBJ = main.o distance.o KNN.o rd.o
TGT = KNN

all: $(TGT) clean

$(TGT): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TGT)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o

distance.o: distance.cpp
	$(CC) $(CFLAGS) -c distance.cpp -o distance.o

KNN.o: KNN.cpp
	$(CC) $(CFLAGS) -c KNN.cpp -o KNN.o

rd.o: reading_datasets.cpp
	$(CC) $(CFLAGS) -c reading_datasets.cpp -o rd.o

clean:
	rm -f *.o

run: all
	./$(TGT)