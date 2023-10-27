CC = g++
CFLAGS = -c
DIR = src/

ALL: KNN

KNN: main.o euclidean_distance.o
	$(CC) main.o euclidean_distance.o -o KNN

main.o: src/main.cpp
	$(CC) $(CFLAGS) src/main.cpp

euclidean_distance.o: src/distance_functions/euclidean_distance.cpp
	$(CC) $(CFLAGS) src/distance_functions/euclidean_distance.cpp

clean:
	rm -rf *.o