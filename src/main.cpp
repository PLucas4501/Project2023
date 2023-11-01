#include <iostream>
#include "KNN.h"
#include "point.h"
#include "distance.h"

using namespace std;

int main(int argc, char *argv[])
{  
    srand(time(NULL));
    const unsigned int arr_size = 10, dim = 2;
    struct point point_array[arr_size];
    for(unsigned int i=0; i<arr_size; i++)
    {
        point_array[i].dim = dim;
        point_array[i].cord = new double[2];
        point_array[i].cord[0] = point_array[i].cord[1] = i;
    }

    unsigned int k = 2;
    KNN knn_problem(dim, k, manhattan_distance, point_array, arr_size);
    knn_problem.initialize();
    knn_problem.print_full_graph();

    for(unsigned int i=0; i<arr_size; i++)
        delete [] point_array[i].cord;

    return 0;
}