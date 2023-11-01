#ifndef KNN_H
#define KNN_H

//A templated class defining the KNN (k-nearest neighbors) problem.

#include <iostream>
#include <stdecxept>
#include "point.h"
#include "vector.h"
#include "heap.h"

class KNN()
{
    //Now the points become nodes inside our graph.
    //We define neighbors and reverse neighbors seperately,
    //thus we have both a directed and an undirected graph at will.
    struct node
    {
        double *cord; //Coordinate array
        heap<unsigned int> edge; //Indices to neighbors, sorted by distance
        vector<unsigned int> reverse_edge; //Indices to reverse neighbors
    };

    unsigned int k, dim; //k and the dimensions of our points
    vector<struct node> graph; //The graph of the problem and its nodes 
    double (*dist)(sturct point, struct point); //A distance metric fucntion
    
public:
    KNN(unsigned int, unsigned int, double (*)(struct point, struct point), struct point*, unsigned int);
    ~KNN();

    //Accessors

    //Mutators
    void add_node(struct point);
    void krand_neighbors(unsigned int);
    void initialize();
    void clear();
}

#endif