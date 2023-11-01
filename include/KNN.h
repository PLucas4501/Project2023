#ifndef KNN_H
#define KNN_H

//A templated class defining the KNN (k-nearest neighbors) problem.

#include <iostream>
#include <stdexcept>
#include "distance.h"
#include "point.h"
#include "vector.h"
#include "heap.h"

class KNN
{
    //The data points become nodes inside our graph.
    //We define neighbors and reverse neighbors seperately,
    //thus we have both a directed and an undirected graph at will.
    struct node
    {
        double *cord; //Coordinate array
        heap<unsigned int> edge; //Indices to neighbors, sorted by distance
        vector<unsigned int> reverse_edge; //Indices to reverse neighbors
    };

    bool initialized{ false };
    unsigned int k, dim; //k and the dimensions of our points
    vector<struct node> graph; //The graph of the problem and its nodes 
    double (*dist)(struct point, struct point); //Distance metric used, default is euclidian

    void krand_neighbors(unsigned int); //Used internally to create neighbors
    
public:
    KNN(unsigned int, unsigned int, double (*)(struct point, struct point), struct point*, unsigned int);
    ~KNN();

    //Accessors
    void print_node(unsigned int);
    void print_full_node(unsigned int);
    void print_graph();
    void print_full_graph();

    //Mutators
    void add_node(struct point);
    void initialize(void);
    void initialize(double (*)(struct point, struct point));
    void solve();
    void clear();
};

#endif