#ifndef KNN_H
#define KNN_H

#include <omp.h> 
#include <cstring>

//A templated class defining the KNN (k-nearest neighbors) problem.
#include "distance.h"
#include "point.h"
#include "vector.h"
#include "AVL.h"
#include "data_import.h"

class KNN {
    //The data points become nodes inside our graph.
    //We define neighbors and reverse neighbors seperately,
    //thus we have both a directed and an undirected graph at will.
    struct node {
        float *cord; //Coordinate array
        omp_lock_t lock; //Lock for parallelism
        minAVL<float, unsigned int> Cedge; //Candidates found during solve
        N_AVL<float, unsigned int> edge; //Indices to neighbors, sorted by distance - only the k best are kept
        RN_AVL<unsigned int> Redge; //Indices to reverse neighbors
    };

    //The graph itself + dataset path name
    vector<struct node> graph;
    char dataset[100] = { '\0' };   

    //Additional parameters(including K)
    double delta;
    bool initialized{ false }; 
    unsigned int k, dim, threads, sample_size, change;
    float (*dist)(struct point, struct point); //Distance metric used, default is euclidian

    //Internal functions
    void krand_neighbors(unsigned int);
    void random_projection(unsigned int);
    void add_candidates(unsigned int, unsigned int);
    
public:
    //Constructors
    KNN(vector<struct point>&, unsigned int);
    KNN(char *path, unsigned int);
    ~KNN();

    //Accessors
    void print_graph();
    float distance(float *, float *);

    //Mutators
    void add_node(struct point);
    void initialize(unsigned int, double, double);
    void solve();
    void clear();
};

#endif