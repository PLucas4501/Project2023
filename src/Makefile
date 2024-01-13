INCLUDE = ../include
OBJECT = object/
CC = g++
CFLAGS = -Wall -fopenmp -I$(INCLUDE)
OBJA =  $(OBJECT)main.o $(OBJECT)KNN.o $(OBJECT)data_import.o
OBJB = $(OBJECT)GraphSolve.o $(OBJECT)KNN.o $(OBJECT)data_import.o

all: KNN GraphSolve

KNN: main.o KNN.o data_import.o
	$(CC) $(CFLAGS) $(OBJA) -o $@

GraphSolve: GraphSolve.o KNN.o data_import.o
	$(CC) $(CFLAGS) $(OBJB) -o $@



GraphSolve.o: GraphSolve.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)$@
 
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