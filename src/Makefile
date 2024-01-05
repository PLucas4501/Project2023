INCLUDE = ../include
CC = g++
CFLAGS = -Wall -fopenmp -I$(INCLUDE)
OBJ = distance.o data_import.o

all: KNN GraphSolve

KNN: main.o KNN.o $(OBJ)
	$(CC) $(CFLAGS) main.o KNN.o $(OBJ) -o KNN

GraphSolve: GraphSolve.o $(OBJ)
	$(CC) $(CFLAGS) GraphSolve.o $(OBJ) -o GraphSolve



GraphSolve.o: GraphSolve.cpp
	$(CC) $(CFLAGS) -c GraphSolve.cpp -o GraphSolve.o
 
main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o

KNN.o: KNN.cpp
	$(CC) $(CFLAGS) -c KNN.cpp -o KNN.o

distance.o: distance.cpp
	$(CC) $(CFLAGS) -c distance.cpp -o distance.o

data_import.o: data_import.cpp
	$(CC) $(CFLAGS) -c data_import.cpp -o data_import.o

clean:
	rm -f *.o KNN GraphSolve

run: KNN
	./KNN