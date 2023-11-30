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
        float *cord; //Coordinate array
        minAVL<float, unsigned int> edge; //Indices to neighbors, sorted by distance - only the k best are kept
        AVL<unsigned int> Redge; //Indices to reverse neighbors

        //The following will be empty before and after solve
        minAVL<float, unsigned int> Cedge; //Candidates found during solve
        minAVL<float, unsigned int> Uedge; //New edges added from previous iteration
        vector<unsigned int> URedge; //New reverse added from previous iteration
    };

    //pair structure to keep neighbor combination (graph indices)
    struct pair { unsigned int a,b; };

    bool change; //solve flag
    unsigned int k, dim;
    bool initialized{ false }; 
    vector<struct node> graph;
    //AVL<unsigned int> distpair; //Temporarily keeps pairs of calculated distances as a unique key
    float (*dist)(struct point, struct point); //Distance metric used, default is euclidian

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
    KNN(unsigned int, unsigned int, float (*)(struct point, struct point), struct point*, unsigned int);
    ~KNN();

    //Accessors
    void print_node(unsigned int);
    void print_full_node(unsigned int);
    void print_graph();
    void print_full_graph();

    //Mutators
    void add_node(struct point);
    void initialize(float (*)(struct point, struct point), unsigned int);
    void initialize(void);
    void solve();
    void clear();

    //Special
    void funA(unsigned int);
    void funB(unsigned int);
};

#endif