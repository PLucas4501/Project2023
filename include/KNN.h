#ifndef KNN_H
#define KNN_H

#include <omp.h> 
#include <cstring>

#include "point.h"
#include "vector.h"
#include "AVL.h"
#include "data_import.h"

//Distance function codes
#define DEFAULT 0
#define NORM 1
#define EUCLID 2
#define MANHATTAN 3


//A class defining the KNN (k-nearest neighbors) problem.
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

    //Additional parameters
    unsigned int dim, threads;
    float (KNN::*distf)(unsigned int, unsigned int){ nullptr }; //Distance metric
    float calc_dist(unsigned int A, unsigned int B)
    { return (this->*distf)(A, B); }

    //Internal functions
    void krand_neighbors(unsigned int, unsigned int);
    void random_projection();
    void add_candidates(unsigned int, unsigned int);
    
public:
    //Constructors & destructors
    KNN(vector<struct point>&, unsigned int);
    KNN(char *, unsigned int);
    ~KNN();

    //Public interface
    unsigned int const get_size()
    { return graph.get_size(); }
    
    bool export_graph(char *, unsigned int);
    void print(unsigned int);
    void add_node(struct point);
    void set_metric(unsigned int);
    bool solve(unsigned int, double, double);
    void true_solve(unsigned int);
    bool accuracy(unsigned int, char *);
    void clear();

    //Distance calculators
    float norm_distance(unsigned int, unsigned int);
    float euclidean_distance(unsigned int, unsigned int);
    float manhattan_distance(unsigned int, unsigned int);
};

#endif