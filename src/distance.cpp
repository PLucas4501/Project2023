#include "distance.h"

double euclidean_distance(struct point x1, struct point x2)
{
    double dist = 0;
    for (unsigned int i=0; i < x1.dim; i++)
        dist += pow((x1.cord[i] - x2.cord[i]), 2);

    return sqrt(dist);
}


double manhattan_distance(struct point x1, struct point x2)
{
    double dist = 0;
    for (unsigned int i=0; i < x1.dim; i++)  
        dist += abs((x1.cord[i] - x2.cord[i]));
    
    return dist;
}