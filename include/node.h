//Node prototypes for the KNN graph

#ifndef NODE_H
#define NODE_H

struct node {
    unsigned int dim;
    double *cord;
    struct node **edge;
};

#endif