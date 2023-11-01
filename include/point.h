#ifndef POINT_H
#define POINT_H

//Prototypes for the problem, and subsequently the KNN graph
//Only coordinates here, these exist in a plane.

struct point
{
    unsigned int dim; //# of dimensions
    double *cord; //Coordinates array
};

#endif