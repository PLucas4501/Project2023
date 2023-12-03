#ifndef KNN_H
#define KNN_H

//A templated class defining the KNN (k-nearest neighbors) problem.

#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "distance.h"
#include "point.h"
#include "vector.h"
#include "AVL.h"
#include "data_import.h"


class KNN
{
    //The data points become nodes inside our graph.
    //We define neighbors and reverse neighbors seperately,
    //thus we have both a directed and an undirected graph at will.
    struct node {
        float *cord; //Coordinate array
        minAVL<float, unsigned int> edge; //Indices to neighbors, sorted by distance - only the k best are kept
        AVL<unsigned int> Redge; //Indices to reverse neighbors

        //The following will be empty before and after solve
        minAVL<float, unsigned int> Cedge; //Candidates found during solve
        minAVL<float, unsigned int> Uedge; //New edges added from previous iteration
        vector<unsigned int> URedge; //New reverse added from previous iteration
    };

    //pair structure to keep neighbor combination (graph indices)
    double delta;
    char *dataset = nullptr;
    bool initialized{ false };
    vector<struct node> graph;
    struct pair { unsigned int a,b; };
    unsigned int k, dim, change, sample_size;
    float (*dist)(struct point, struct point); //Distance metric used, default is euclidian

    //Used internally to create neighbors during initialization
    void krand_neighbors(unsigned int);
    unsigned int key_function(struct pair);
    unsigned int key_function(unsigned int, unsigned int);
    
public:
    KNN(unsigned int, vector<struct point>&, float (*)(struct point, struct point), char *path);
    KNN(unsigned int, unsigned int);
    ~KNN();

    //Accessors
    void print_node(unsigned int);
    void print_full_node(unsigned int);
    void print_graph();
    void print_full_graph();

    //Mutators
    void add_node(struct point);
    void initialize(double, double);
    void solve();
    void clear();

    //Special
    void funA(unsigned int);
    void funB(unsigned int);
};

#endif