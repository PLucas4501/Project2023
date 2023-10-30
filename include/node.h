//Node prototypes for the KNN graph

#ifndef NODE_H
#define NODE_H

#include "binary_heap.h"

struct node {
    unsigned int dim;
    double *cord;
    struct node **edge;
    binary_heap reverse_edge;
};

#endif