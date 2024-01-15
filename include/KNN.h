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

//RPT epsilon
#define E 1e-8

//A class defining the KNN (k-nearest neighbors) problem.
class KNN {
    //The data points become nodes inside our graph.
    //We define neighbors and reverse neighbors seperately,
    //thus we have both a directed and an undirected graph at will.
    struct node {
        float norm{ 0 }; //Euclidean norm
        float *cord{ nullptr }; //Coordinate array
    
        omp_lock_t lock; //Lock for parallelism
        minAVL<float, unsigned int> Cedge; //Candidates found during solve
        N_AVL<float, unsigned int> edge; //Indices to neighbors, sorted by distance - only the k best are kept
        RN_AVL<unsigned int> Redge; //Indices to reverse neighbors
    };

    //Random projection tree nodes
    struct RPTnode {
        float offset;
        unsigned int L;
        unsigned int R;
        vector<float> hyperplane;
        vector<unsigned int> points; //For leaves only
    };

    //Random projection tree
    struct RPTree {
        unsigned int leaf_size;
        unsigned int leaves{ 0 };
        vector<struct RPTnode *> nodes;

        RPTree(unsigned int leaf_size): leaf_size(leaf_size) {}
        ~RPTree() {
            for(unsigned int i=0; i < nodes.get_size(); i++)
                delete nodes[i];
        }

        void add_leaf(vector<unsigned int>&);
        void add_node(float, unsigned int, unsigned int, vector<float>&);
    };

    //The graph itself
    vector<struct node> graph;

    //Additional parameters
    float *worst{ nullptr }; //Used on true solve
    char dataset[100] = { '\0' };
    unsigned int dim{ 0 }, threads{ 4 };
    float (KNN::*distf)(unsigned int, unsigned int){ nullptr }; //Distance function

    //Internal functions
    void kfirst_neighbors(unsigned int, unsigned int);
    void krand_neighbors(unsigned int, unsigned int);
    void RPT_split(vector<unsigned int>&, vector<unsigned int>&, vector<unsigned int>&, vector<float>&, float&);
    void RPT_create(struct RPTree&, vector<unsigned int>, int);
    void add_candidates(unsigned int, unsigned int);

    //Using distf to calculate distance
    float calc_dist(unsigned int A, unsigned int B)
    { return (this->*distf)(A, B); }
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
    bool solve(unsigned int, double, bool, double, unsigned int, unsigned int);
    void true_solve(unsigned int);
    bool accuracy(unsigned int, char *);
    void clear();

    //Distance calculators
    float norm_distance(unsigned int, unsigned int);
    float euclidean_distance(unsigned int, unsigned int);
    float manhattan_distance(unsigned int, unsigned int);
};

#endif