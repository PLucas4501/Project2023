#include <iostream>
#include <stdexcept>
#include <random>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <omp.h>

#include "KNN.h"

//Create k unique random neighbors for node@graph[index]
void KNN::krand_neighbors(unsigned int index) {
    if(index >= graph.get_size())
        throw std::out_of_range("graph index out of range");

    unsigned int cindex, i;
    AVL<unsigned int> candidates;
    for(i=0; i < graph.get_size(); i++)
        if(i != index) 
            candidates.insert(i);

    i = this->k;
    std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
    while(i > 0) {
        cindex = gen() % candidates.get_size();
        cindex = candidates.remove_index(cindex);

        //It is possible for a neighbor to not be inserted due to same key (distance)
        if(graph[index].edge.New.insert(this->distance(graph[index].cord, graph[cindex].cord), cindex))
            i -= 1;
        else if(candidates.is_empty()) //This is a problem... though highly unlikely
            throw std::logic_error("unable to initialize knn graph: insufficient neighbors");

        //Reverse neighbor insertion requires lock
        omp_set_lock(&(graph[cindex].lock));
        graph[cindex].Redge.New.insert(index);
        omp_unset_lock(&(graph[cindex].lock));  
    } candidates.clear();
}

//Evaluate and add candidates to each otherz
void KNN::add_candidates(unsigned int nodeA, unsigned int nodeB) {
    if(nodeA == nodeB) return;
    float key = this->distance(graph[nodeA].cord, graph[nodeB].cord);

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
    for(unsigned int i=0; i < point_vector.get_size(); i++)
        this->add_node(point_vector[i]);

    //Create data locks
    #pragma omp parallel for num_threads(threads)
    for(unsigned int i=0; i < graph.get_size(); i++)
        omp_init_lock(&(graph[i].lock));
    omp_set_num_threads(threads);
}

KNN::KNN(char *path, unsigned int threads = 4) {
    vector<struct point> point_vector;
    char dataset_path[100] = { '\0' };

    strcat(dataset_path, IMPORT_PATH);
    strcat(dataset_path, path);
    binary(dataset_path, point_vector);

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
        graph[i].edge.Old.print();
}

//Calculate distance between two points
float KNN::distance(float *nodeA, float *nodeB) {
    struct point p1{ this->dim, nodeA };
    struct point p2{ this->dim, nodeB };
    return this->dist(p1, p2);
}

//We can add an arbitrary amount of nodes
void KNN::add_node(struct point data) {
    if(this->initialized)
        throw std::logic_error("unable to add node to graph: graph already initialized");

    if(data.dim != this->dim) //Dimensions mismatch
        throw std::logic_error("Dataset mismatch: incorrect dimensions");

    struct node new_node;
    new_node.cord = new float[this->dim];
    for(unsigned int i=0; i < this->dim; i++)
        new_node.cord[i] = data.cord[i];
    graph.push(new_node);
}

//Create neighbors for all nodes, and optionally set the distance metric and k
void KNN::initialize(unsigned int k, double sampling = 1, double delta = 0.001) {
    if(this->initialized)
        throw std::logic_error("unable to initialize knn graph: graph already initialized");

   this->k = k;
    if(this->k >= graph.get_size())
        throw std::logic_error("unable to initialize knn graph: k >= graph");

    typedef std::chrono::high_resolution_clock timer;
    timer::time_point start = timer::now();
    std::cout << "Initializing graph... " << std::flush;
    this->delta = delta;
    this->dist = euclidean_distance;
    this->sample_size = sampling * this->k;
    #pragma omp parallel for
    for(unsigned int i=0; i < graph.get_size(); i++) {
        graph[i].Cedge.set_cap(this->k);
        graph[i].edge.Old.set_cap(this->k);
        this->krand_neighbors(i);
    } this->initialized = true;
    timer::time_point finish = timer::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "initialization complete | Time elapsed: " << elapsed.count() << "ms" << std::endl;
}


//----------------Solve the graph: iterate and find the k nearest neighbors for every node----------------//

//Improvements: 
//Fix whatever the fck makes nodes with less than k neighbors at end
//Maybe a little faster in Region A?
void KNN::solve() {
    if(!initialized)
        throw std::logic_error("unable to solve knn graph: knn graph not initialized");
    typedef std::chrono::high_resolution_clock timer;
    std::cout << "Solving Graph... " << std::flush;

    timer::time_point start = timer::now();
    AVL<unsigned int> sampledNR;
    minAVL<float, unsigned int> sampledN;
    unsigned int i, j, k, tgt_node, iterations = 0;
    struct minAVL<float, unsigned int>::payload rem;
    unsigned int threshold = this->delta * this->k * graph.get_size();
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
                    if(graph[i].edge.find(rem.key)) //Check that neighbor does not already exist
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
    } while(this->change > threshold);
    timer::time_point finish = timer::now();
    std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "graph solved at " << iterations << " iterations";
    std::cout << " | Time elapsed: " << elapsed.count() << "ms" << std::endl;

    //Transfer flagged nodes
    for(i=0; i<graph.get_size(); i++) {
        while(!graph[i].edge.New.is_empty()) {
            rem = graph[i].edge.New.remove_min();
            graph[i].edge.Old.insert(rem.key, rem.data);
        }

        if(graph[i].edge.Old.get_size() != this->k)
            std::cerr << "Warning: node " << i << " with less than k neighbors after solve" << std::endl;
    } std::cout << "Calculating accuracy... " << std::flush;
    start = timer::now();

    //Open true graph
    i = 0;
    FILE *true_graph;
    unsigned int neighbor;
    char solved[150] = { '\0' };
    vector<unsigned int> true_neighbors;
    double best, difference, accuracy, total_accuracy = 0;
    strcat(solved, EXPORT_PATH);
    while(this->dataset[i] != '\0') 
        if(this->dataset[i++] == '.')
            break;
    strncat(solved, this->dataset, i - 1);

    //Open true graph
    if(!(true_graph = fopen(solved, "r"))) {
        char command[200] = { '\0' };
        sprintf(command, "GraphSolve %s -k %u -t %u", this->dataset, this->k, this->threads);
        system(command);
        if(!(true_graph = fopen(solved, "r"))) {
            std::cerr << "unable to check for accuracy: could not open true graph file" << std::endl;
            return;
        }
    }

    //Read lines consecutively and check for accuracy
    fscanf(true_graph, "%*[^\n]\n");
    for(i=0; i < graph.get_size(); i++) {
        for(j=0; j < this->k; j++) {
            if(fscanf(true_graph, "%u", &neighbor) < 0) {
                std::cerr << "Error reading file (Node " << i << ", neighbor " << j << ")" << std::endl;
                fclose(true_graph);
                return;                
            } true_neighbors.push(neighbor);
        }

        //Match found neighbors with real ones
        accuracy = 0;
        for(j=0; j < graph[i].edge.Old.get_size(); j++) {
            if(graph[i].edge.Old[j].data != true_neighbors[j]) {
                best = this->distance(graph[i].cord, graph[true_neighbors[j]].cord);
                difference = std::abs(best - graph[i].edge.Old[j].key);
                if(difference > 1)
                    std::cout << difference << ", " << best << std::endl;
                accuracy += 1 - difference/best;
            } else accuracy += 1;
        }
        total_accuracy += accuracy/graph[i].edge.Old.get_size();
        true_neighbors.clear();
    } fclose(true_graph);
    finish = timer::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << (total_accuracy/graph.get_size())*100 << "% | Time elasped: " << elapsed.count() << "ms" << std::endl;
}

//Remove all neighbors from every node
void KNN::clear() {
    for(unsigned int i=0; i < graph.get_size(); i++) {
        graph[i].edge.clear();
        graph[i].Redge.clear();
    } initialized = false; //Can re-initialize graph
}