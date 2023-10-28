#include "manhattan_distance.h"
#include <iostream>

double manhattan_distance(node x1, node x2)
{
    double dist = 0;
    for (unsigned int i=0; i<x1.N; i++)  
        dist += abs((x1.cord[i] - x2.cord[i]));
    
    return dist;
}


//