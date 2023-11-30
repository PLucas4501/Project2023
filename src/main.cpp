#include <iostream>
#include "KNN.h"
#include "point.h"
#include "distance.h"
#include "reading_datasets.h"

using namespace std;

int main(int argc, char *argv[])
{  
    srand(time(NULL));
    
    unsigned int k = 5;
    /*const unsigned int arr_size = 50, dim = 100;
    struct point point_array[arr_size];
    for(unsigned int i=0; i < arr_size; i++) {
        point_array[i].dim = dim;
        point_array[i].cord = new float[dim];
        for(unsigned int j=0; j < dim; j++)
            point_array[i].cord[j] = i;
    }

    KNN knn_problem(dim, k, manhattan_distance, point_array, arr_size);*/
    
    const char *path = "../datasets/D1.bin";
    KNN knn_problem(100, k, euclidean_distance, nullptr, 0);
    reading_datasets(path, &knn_problem);


    knn_problem.initialize();
    knn_problem.solve();
    knn_problem.print_graph();

    /*for(unsigned int i=0; i<arr_size; i++)
        delete [] point_array[i].cord;

    return 0;*/
}