INCLUDE = ../include
OBJECT = object/
CC = g++
CFLAGS = -Wall -fopenmp -I$(INCLUDE)
OBJ =  $(OBJECT)main.o $(OBJECT)KNN.o $(OBJECT)data_import.o

all: KNN

KNN: main.o KNN.o data_import.o
	$(CC) $(CFLAGS) $(OBJ) -o $@

#GraphSolve: GraphSolve.o $(OBJ)
#	$(CC) $(CFLAGS) GraphSolve.o $(OBJ) -o $@



#GraphSolve.o: GraphSolve.cpp
#	$(CC) $(CFLAGS) -c $< -o $(OBJECT)$@
 
main.o: main.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)$@

KNN.o: KNN.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)$@

data_import.o: data_import.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)$@

clean:
	rm -f object/*.o KNN GraphSolve

run: KNN
	./KNN