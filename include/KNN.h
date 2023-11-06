#ifndef KNN_H
#define KNN_H

//A templated class defining the KNN (k-nearest neighbors) problem.

#include <iostream>
#include <stdexcept>
#include "distance.h"
#include "point.h"
#include "vector.h"
#include "AVL.h"
#include "heap.h"

class KNN
{
    //The data points become nodes inside our graph.
    //We define neighbors and reverse neighbors seperately,
    //thus we have both a directed and an undirected graph at will.
    struct node
    {
        double *cord; //Coordinate array
        k_rheap<unsigned int> edge; //Indices to neighbors, sorted by distance - only the k best are kept
        AVL reverse_edge; //Indices to reverse neighbors
    };

    //pair structure to keep neighbor combination (graph indices)
    struct pair { unsigned int a,b; };

    AVL distances; //Look up structure telling us when to calculate distances
    unsigned int k, dim;
    bool initialized{ false }; 
    vector<struct node> graph;
    double (*dist)(struct point, struct point); //Distance metric used, default is euclidian

    //Used internally to create neighbors during initialization
    void krand_neighbors(unsigned int);

    //A key-generation function, that is:
    //1) injective
    //2) symmetric (key_function(a,b) == key_function(b,a))
    //3) discards a==b values (returns 0)
    //Should only be used AFTER (or during) initialization
    unsigned int key_function(struct pair);
    unsigned int key_function(unsigned int, unsigned int);
    
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
    void initialize(double (*)(struct point, struct point), unsigned int);
    void initialize(void);
    void solve();
    void clear();
};

#endif