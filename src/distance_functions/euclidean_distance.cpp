#include "euclidean_distance.h"
#include <iostream>

double euclidean_distance(node x1, node x2)
{
    double dist = 0;
    for (unsigned int i=0; i<x1.N; i++)
        dist += pow((x1.cord[i] - x2.cord[i]), 2);

    //std::cout << sqrt(dist) << std::endl;
    return sqrt(dist);
}