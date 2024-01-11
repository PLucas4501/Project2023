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
        float *cord{ nullptr }; //Coordinate array
        float norm{ 0 }; //Euclidean norm
        omp_lock_t lock; //Lock for parallelism
        minAVL<float, unsigned int> Cedge; //Candidates found during solve
        N_AVL<float, unsigned int> edge; //Indices to neighbors, sorted by distance - only the k best are kept
        RN_AVL<unsigned int> Redge; //Indices to reverse neighbors
    };

    //The graph itself + dataset path name
    vector<struct node> graph;
    char dataset[100] = { '\0' }; 
    minAVL<float, unsigned int> *true_graph;

    //Additional parameters (including K)
    double acc{ 0 };
    bool initialized{ false }; 
    unsigned int k, dim, threads, sample_size, threshold, change;

    float (KNN::*distf)(unsigned int, unsigned int); //Distance metric used, default is euclidian
    float calc_dist(unsigned int A, unsigned int B)
    { return (this->*distf)(A, B); }

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

    //Mutators
    void add_node(struct point);
    void initialize(unsigned int, double, double);
    void accuracy();
    bool true_solve();
    bool solve();
    void clear();

    //Distance calculators
    float norm_distance(unsigned int, unsigned int);
    float euclidean_distance(unsigned int, unsigned int);
    float manhattan_distance(unsigned int, unsigned int);
};

#endif