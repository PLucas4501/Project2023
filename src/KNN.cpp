#include <iostream>
#include <stdexcept>
#include <random>
#include <chrono>
#include <cstdio>
#include <cstring>

#include <omp.h>

#include "KNN.h"

//Create k unique random neighbors for node@graph[index] (super slow)
void KNN::krand_neighbors(unsigned int index) {
    if(index >= graph.get_size())
        throw std::out_of_range("krand_neighbors() failed: graph index out of range");

    unsigned int cindex;
    vector<unsigned int> candidates(graph.get_size());
    for(unsigned i=0; i < graph.get_size(); i++)
        if(i != index) candidates.push(i);

    std::ranlux24_base gen(std::chrono::system_clock::now().time_since_epoch().count());
    while(graph[index].edge.New.get_size() < this->k) {
        if(candidates.is_empty()) //This is a problem... though highly unlikely
            throw std::logic_error("unable to initialize knn graph: insufficient neighbors");
        cindex = gen() % candidates.get_size();
        cindex = candidates.remove(cindex);
        graph[index].edge.New.insert(calc_dist(index, cindex), cindex);

        //Reverse neighbor insertion requires mutex lock
        omp_set_lock(&(graph[cindex].lock));
        graph[cindex].Redge.New.insert(index);
        omp_unset_lock(&(graph[cindex].lock));  
    } candidates.clear();
}

//Evaluate and add candidates to each other
void KNN::add_candidates(unsigned int nodeA, unsigned int nodeB) {
    if(nodeA == nodeB) return;
    float key = calc_dist(nodeA, nodeB);

    //Add candidates to each other
    omp_set_lock(&(graph[nodeA].lock));
    graph[nodeA].Cedge.insert(key, nodeB);
    omp_unset_lock(&(graph[nodeA].lock));  

    omp_set_lock(&(graph[nodeB].lock));
    graph[nodeB].Cedge.insert(key, nodeA);
    omp_unset_lock(&(graph[nodeB].lock));  
}


//------PUBLIC INTERFACE------//
KNN::KNN(vector<struct point> &point_vector, unsigned int threads = 4) {
    this->threads = threads;
    this->dim = point_vector[0].dim;
    #pragma omp parallel for num_threads(threads)
    for(unsigned int i=0; i < point_vector.get_size(); i++)
        this->add_node(point_vector[i]);

    //Create data locks
    #pragma omp parallel for num_threads(threads)
    for(unsigned int i=0; i < graph.get_size(); i++)
        omp_init_lock(&(graph[i].lock));
}

KNN::KNN(char *path, unsigned int threads = 4) {
    vector<struct point> point_vector;
    char dataset_path[100] = { '\0' };
    sprintf(dataset_path, "%s%s", IMPORT_PATH, path);
    
    if(binary(dataset_path, point_vector) < 0)
        std::cerr << "Could not import dataset properly" << std::endl;

    this->threads = threads;
    this->dim = point_vector[0].dim;
    for(unsigned int i=0; i < point_vector.get_size(); i++) {
        this->add_node(point_vector[i]);
        delete [] point_vector[i].cord;
    }

    //Create data locks
    #pragma omp parallel for num_threads(threads)
    for(unsigned int i=0; i < graph.get_size(); i++)
        omp_init_lock(&(graph[i].lock));
    strcat(this->dataset, path);
}


KNN::~KNN() {
    #pragma omp parallel for num_threads(threads)
    for(unsigned int i=0; i < graph.get_size(); i++) {
        delete[] graph[i].cord;
        graph[i].edge.clear();
        graph[i].Redge.clear();
        omp_destroy_lock(&(graph[i].lock));
    } graph.clear();
}

//Simple print
void KNN::print_graph() {
    for(unsigned int i=0; i<graph.get_size(); i++)
        graph[i].edge.Old.full_print();
}

//We can add an arbitrary amount of nodes
void KNN::add_node(struct point data) {
    if(this->initialized) { 
        std::cerr << "add_node() failed: graph already initialized" << std::endl; 
        return; 
    }

    if(data.dim != this->dim) { 
        std::cerr << "add_node() failed: dimensions mismatch" << std::endl; 
        return; 
    }

    struct node new_node;
    new_node.cord = new float[this->dim];
    for(unsigned int i=0; i < this->dim; i++)
        new_node.cord[i] = data.cord[i];

    #pragma omp critical
    graph.push(new_node);
}

//Create neighbors for all nodes, and optionally set the distance metric and k
void KNN::initialize(unsigned int k, double sampling = 1, double delta = 0.001) {
    std::cout << "Initializing graph... " << std::flush;

    if(this->initialized)
    { std::cerr << "initialize() failed: graph already initialized" << std::endl; return; }

    if(k >= graph.get_size())
    { std::cerr << "initialize() failed: k >= graph" << std::endl; return; }
    typedef std::chrono::high_resolution_clock timer;
    timer::time_point start = timer::now();
   
    this->k = k;
    distf = &KNN::euclidean_distance;
    this->sample_size = sampling * this->k;
    this->threshold = delta * this->k * graph.get_size();
    if(distf == &KNN::euclidean_distance) {
        #pragma omp parallel for num_threads(threads)
        for(unsigned int i=0; i < graph.get_size(); i++)
            for(unsigned int j=0; j < dim; j++)
                graph[i].norm += graph[i].cord[j] * graph[i].cord[j];
        distf = &KNN::norm_distance;
    } this->initialized = true;

    #pragma omp parallel for num_threads(threads)
    for(unsigned int i=0; i < graph.get_size(); i++) {
        graph[i].Cedge.set_cap(this->k);
        graph[i].edge.Old.set_cap(this->k);
        this->krand_neighbors(i);
    } 

    timer::time_point finish = timer::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "initialization complete | Time elapsed: " << elapsed.count() << "ms" << std::endl;
}


//----------------Solve the graph: iterate and find_key the k nearest neighbors for every node----------------//

//Improvements: 
//Fix whatever the fck makes nodes with less than k neighbors at end
//Maybe a little faster in Region A?
bool KNN::solve() {
    if(!initialized) {
        std::cerr << "solve() failed: KNN graph not initialized" << std::endl;
        return false;
    } std::cout << "Solving Graph... " << std::flush;
    typedef std::chrono::high_resolution_clock timer;
    timer::time_point start = timer::now();

    AVL<unsigned int> sampledNR;
    minAVL<float, unsigned int> sampledN;
    struct minAVL<float, unsigned int>::payload rem;
    unsigned int i, j, k, tgt_node, iterations = 0;
    do {
        timer::time_point regA_S = timer::now();
        #pragma omp parallel for \
            private(i, j, k, tgt_node, rem, sampledN, sampledNR)
        for(i=0; i < graph.get_size(); i++) {
            //Sample neighbors (sampling policy is just getting the min)
            for(j=0; j < this->sample_size; j++) {
                if(!graph[i].edge.New.is_empty()) {
                    rem = graph[i].edge.New.remove_min();
                    sampledN.insert(rem.key, rem.data);
                }

                if(!graph[i].Redge.New.is_empty())
                    sampledNR.insert(graph[i].Redge.New.remove_min());
            }

            //Check new neighbors
            for(j=0; j<sampledN.get_size(); j++) {
                tgt_node = sampledN[j].data;

                for(k=j+1; k<sampledN.get_size(); k++)
                    this->add_candidates(tgt_node, sampledN[k].data);

                for(k=0; k<sampledNR.get_size(); k++)
                    this->add_candidates(tgt_node, sampledNR[k]);

                for(k=0; k<graph[i].edge.Old.get_size(); k++)
                    this->add_candidates(tgt_node, graph[i].edge.Old[k].data);

                for(k=0; k<graph[i].Redge.Old.get_size() && k<this->sample_size; k++)
                    this->add_candidates(tgt_node, graph[i].Redge.Old[k]);
            }

            //Check new reverse neighbors
            for(j=0; j<sampledNR.get_size(); j++) {
                tgt_node = sampledNR[j];

                for(k=j+1; k<sampledNR.get_size(); k++)
                    this->add_candidates(tgt_node, sampledNR[k]);

                for(k=0; k<graph[i].edge.Old.get_size(); k++)
                    this->add_candidates(tgt_node, graph[i].edge.Old[k].data);

                for(k=0; k<graph[i].Redge.Old.get_size() && k<this->sample_size; k++)
                    this->add_candidates(tgt_node, graph[i].Redge.Old[k]);
            }

            //Make new neighbors and reverse neighbors old
            while(!sampledN.is_empty()) {
                rem = sampledN.remove_min();
                graph[i].edge.Old.insert(rem.key, rem.data);
            }

            while(!sampledNR.is_empty())
                graph[i].Redge.Old.insert(sampledNR.remove_min());
        } timer::time_point regA_F = timer::now();
        auto elapsedA = std::chrono::duration_cast<std::chrono::milliseconds>(regA_F - regA_S);
        //std::cout << "Region A: " << elapsedA.count() << "ms" << std::endl;

        //Parallel region B
        this->change = 0;
        timer::time_point regB_S = timer::now();
        #pragma omp parallel for private(i, rem) reduction(+:change)
        for(i=0; i<graph.get_size(); i++) {
            while(!graph[i].Cedge.is_empty()) {
                rem = graph[i].Cedge.remove_min();
                if(rem.key < graph[i].edge.max_key()) { //Better key found
                    if(graph[i].edge.Old.find_key(rem.key)) //Check that neighbor does not already exist
                        continue;
             
                    omp_set_lock(&(graph[rem.data].lock));
                    graph[rem.data].Redge.New.insert(i); //Add new reverse neighbor to new neighbor
                    omp_unset_lock(&(graph[rem.data].lock));

                    omp_set_lock(&(graph[graph[i].edge.max()].lock));
                    graph[graph[i].edge.max()].Redge.remove(i); //Remove reverse neighbor from old neighbor
                    omp_unset_lock(&(graph[graph[i].edge.max()].lock));

                    graph[i].edge.New.insert(rem.key, rem.data);
                    graph[i].edge.remove_max();
                    change++;
                } else graph[i].Cedge.clear();
            }
        } timer::time_point regB_F = timer::now();
        auto elapsedB = std::chrono::duration_cast<std::chrono::milliseconds>(regB_F - regB_S);
        //std::cout << "Region B: " << elapsedB.count() << "ms" << std::endl;
        iterations++;
    } while(this->change > this->threshold);

    //Evaluation
    timer::time_point finish = timer::now();
    std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "graph solved at " << iterations << " iterations";
    std::cout << " | Time elapsed: " << elapsed.count() << "ms" << std::endl;

    //Transfer flagged nodes
    #pragma omp parallel for num_threads(threads)
    for(i=0; i < graph.get_size(); i++) {
        while(!graph[i].edge.New.is_empty()) {
            rem = graph[i].edge.New.remove_min();
            graph[i].edge.Old.insert(rem.key, rem.data);
        }

        if(graph[i].edge.Old.get_size() != this->k)
            std::cerr << "Warning: node " << i << " with less than k neighbors after solve" << std::endl;
    } return true;
}

//Evaluate accuracy of KNN graph
void KNN::accuracy() {
    typedef std::chrono::high_resolution_clock timer;
    std::cout << "Calculating accuracy... " << std::flush;
    timer::time_point start = timer::now();

    //Secondary method
    unsigned int i, j;
    minAVL<float, unsigned int> true_neighbors;
    double ret_acc_total = 0, true_acc_total = 0;
    double distance, best, worst, ret_acc, true_acc;
    #pragma omp parallel for num_threads(threads) reduction(+:ret_acc_total, true_acc_total) \
        private(i, j, true_neighbors, distance, best, worst, ret_acc, true_acc)
    for(i=0; i < graph.get_size(); i++) {
        //Find true best neighbors
        worst = 0;
        true_neighbors.set_cap(graph[i].edge.Old.get_size());
        for(j=0; j < graph.get_size(); j++) {
            if(i == j)
                continue;
            
            distance = calc_dist(i, j);
            true_neighbors.insert(distance, j);
            if(distance > worst)
                worst = distance;
        }

        //Decrease accuracy with each miss
        ret_acc = true_acc = graph[i].edge.Old.get_size();
        for(j=0; j < graph[i].edge.Old.get_size(); j++) {
            if(graph[i].edge.Old[j].data != true_neighbors[j].data) {
                best = true_neighbors[j].key;
                distance = graph[i].edge.Old[j].key;
                distance = (distance - best)/(worst - best);
                true_acc -= distance;
                ret_acc -= (1 - best/worst) * distance; //weighted by best's distance difference with worst
            }
        } true_neighbors.clear();
        ret_acc_total += ret_acc/graph[i].edge.Old.get_size();
        true_acc_total += true_acc/graph[i].edge.Old.get_size();
    } timer::time_point finish = timer::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "done, relative accuracy: " << (ret_acc_total/graph.get_size())*100 << "%";
    std::cout << " | Absolute accuracy: " << (true_acc_total/graph.get_size())*100 << "%";
    std::cout << " | Time elapsed: " << elapsed.count() << "ms" << std::endl;
}


/*True solution
bool true_solve() {
    //Create output file
    unsigned int i = 0, j;
    char out_path[100] = { '\0' };
    if(sprintf(out_path, "%s%s.solved%d", EXPORT_PATH, this->binary, this->k) < 0) { 
        std::cerr << "true_solve() failed: sprintf() failed" << std::endl; 
        return false;
    }

    FILE *out_file;
    if((out_file = fopen(out_path, "w")) < 0) { 
        std::cerr << "true_solve() failed: fopen() failed" << endl; 
        return false; 
    }

    if((true_graph = new minAVL<float, unsigned int>[graph.get_size()]) == nullptr) {
        std::cerr << "true_solve() failed: newfailed" << endl; 
        return false;     
    }

    #pragma omp parallel for num_threads private(i)
    for(i=0; i < graph.get_size(); i++)
        true_graph[i].set_cap(this->k);

    //Solve graph
    float key;
    #pragma omp parallel for num_threads(threads) private(i, j, key)
    for(i=0; i < graph.get_size(); i++) {
        for(j = i+1; j < graph.get_size(); j++) {
            key = calc_dist(i, j);
            omp_set_lock(&(graph[i].lock));
            true_graph[i].insert(key, j);
            omp_unset_lock(&(graph[i].lock));

            omp_set_lock(&(graph[j].lock));
            true_graph[j].insert(key, i);
            omp_unset_lock(&(graph[j].lock));
        }
    }

    //Output graph to file
    for(i=0; i < graph_size; i++) {
        unsigned int k_neighbors[neighbors[i].get_size()];
        for(j=0; j < neighbors[i].get_size(); j++)
            k_neighbors[j] = neighbors[i][j].data;

        if(fwrite(k_neighbors, sizeof(unsigned int), neighbors[i].get_size(), out_file) < 0)
        { cerr << "Error writing to file" << endl; exit(EXIT_FAILURE); }

        neighbors[i].clear();
        omp_destroy_lock(&(lock[i]));
    } fclose(out_file);
}*/

//Remove all neighbors from every node
void KNN::clear() {
    for(unsigned int i=0; i < graph.get_size(); i++) {
        graph[i].edge.clear();
        graph[i].Redge.clear();
    } initialized = false; //Can re-initialize graph
}


//Distance functions
float KNN::norm_distance(unsigned int nodeA, unsigned int nodeB) {
    if(nodeA >= graph.get_size() || nodeB >= graph.get_size())
        throw std::out_of_range("norm_distance() failed: graph index out of range");
    
    float distance = 0;
    float *x1 = graph[nodeA].cord;
    float *x2 = graph[nodeB].cord;
    #pragma omp parallel for num_threads(threads) reduction(+:distance)
    for(unsigned int i=0; i < dim; i++)
        distance += x1[i] * x2[i];
    return graph[nodeA].norm + graph[nodeB].norm - 2 * distance;
}


float KNN::euclidean_distance(unsigned int nodeA, unsigned int nodeB) {
    if(nodeA >= graph.get_size() || nodeB >= graph.get_size())
        throw std::out_of_range("euclidean_distance() failed: graph index out of range");
    
    float distance = 0;
    float *x1 = graph[nodeA].cord;
    float *x2 = graph[nodeB].cord;
    #pragma omp parallel for num_threads(threads) reduction(+:distance)
    for(unsigned int i=0; i < dim; i++)
        distance += pow((x1[i] - x2[i]), 2);
    return sqrt(distance);
}

float KNN::manhattan_distance(unsigned int nodeA, unsigned int nodeB) {
    if(nodeA >= graph.get_size() || nodeB >= graph.get_size())
        throw std::out_of_range("manhattan_distance() failed: graph index out of range");
    
    float distance = 0;
    float *x1 = graph[nodeA].cord;
    float *x2 = graph[nodeB].cord;
    #pragma omp parallel for num_threads(threads) reduction(+:distance)
    for(unsigned int i=0; i < dim; i++)  
        distance += abs(x1[i] - x2[i]);
    return distance;
}