#include <iostream>
#include <stdexcept>
#include <random>
#include <chrono>
#include <cstdio>
#include <cstring>

#include <omp.h>
#include <fcntl.h>

#include "KNN.h"

//Create k unique random neighbors for node@graph[index] (super slow)
void KNN::krand_neighbors(unsigned int index, unsigned int k) {
    if(index >= graph.get_size())
        throw std::out_of_range("krand_neighbors() failed: graph index out of range");

    unsigned int cindex;
    vector<unsigned int> candidates(graph.get_size());
    for(unsigned i=0; i < graph.get_size(); i++)
        if(i != index) candidates.push(i);

    std::ranlux24_base gen(std::chrono::system_clock::now().time_since_epoch().count());
    while(graph[index].edge.New.get_size() < k) {
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

//Initialize neighbors through random projection trees
void KNN::random_projection() {

    return;
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
void KNN::print(unsigned int index = 0) {
    if(index == 0) {
        for(unsigned int i=0; i < graph.get_size(); i++)
            graph[i].edge.Old.print();
    } else graph[index].edge.Old.print();
}

//Export k neighbors graph to file
bool KNN::export_graph(char *path = nullptr, unsigned int k = 0) {
    typedef std::chrono::high_resolution_clock timer;
    std::cout << "Exporting graph... " << std::flush;
    timer::time_point start = timer::now();

    //Create output file
    char out_path[100] = { '\0' };
    if(path == nullptr)
        path = this->dataset;
    
    if(sprintf(out_path, "%s%s.solved%d", EXPORT_PATH, path, k) < 0) { 
        std::cerr << "export_graph() failed: sprintf() failed" << std::endl; 
        return false;
    }

    FILE *out_file;
    if((out_file = fopen(out_path, "w")) < 0) { 
        std::cerr << "export_graph() failed: fopen() failed" << std::endl; 
        return false; 
    }

    float worst;
    unsigned int n;
    float *neighbors;
    for(unsigned int i=0; i < graph.get_size(); i++) {
        if(k > graph[i].edge.Old.get_size()) {
            std::cerr << "export_graph() failed: k > neighbors" << std::endl;
            fclose(out_file);
            return false;
        } else n = k > 0 ? k : graph[i].edge.Old.get_size();
        
        if((neighbors = new float[n]) == nullptr) {
            std::cerr << "export_graph() failed: new failed" << std::endl;
            fclose(out_file);
            return false;
        }

        //#pragma omp parallel for num_threads(threads)  
        #pragma omp parallel for num_threads(threads) 
        for(unsigned int j = 0; j < n; j++)
            neighbors[j] = graph[i].edge.Old[j].key;

        if(fwrite(neighbors, sizeof(float), n, out_file) < n) { 
            std::cerr << "export_graph() failed: fwrite() failed" << std::endl; 
            delete [] neighbors;
            fclose(out_file);
            return false;
        }

        worst = graph[i].edge.Old.max_key();
        if(fwrite(&worst, sizeof(float), 1, out_file) < 1) {
            std::cerr << "export_graph() failed: fwrite() failed" << std::endl; 
            delete [] neighbors;
            fclose(out_file);
            return false;
        }

        delete [] neighbors;
    } fclose(out_file);

    timer::time_point finish = timer::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "done | Time elapsed: " << elapsed.count() << "ms" << std::endl;
    return true;
}

//We can add an arbitrary amount of nodes
void KNN::add_node(struct point data) {
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

//Set distance function
void KNN::set_metric(unsigned int DF = DEFAULT) {
    switch(DF) {
        case DEFAULT:
            this->distf = &KNN::norm_distance;
            break;
        case NORM:
            this->distf = &KNN::norm_distance;
            break;
        case EUCLID:
            this->distf = &KNN::euclidean_distance;
            break;
        case MANHATTAN:
            this->distf = &KNN::manhattan_distance;
            break;
        default:
            std::cerr << "set_metric() failed: unknown distance function" << std::endl;
            return;
    }

    //Calc norms if necessary
    unsigned int i, j;
    if(DF == NORM) {
        #pragma omp parallel for num_threads(threads) private(i, j)
        for(i=0; i < graph.get_size(); i++)
            for(j=0; j < this->dim; j++)
                graph[i].norm += graph[i].cord[j] * graph[i].cord[j];
    }
}

//----------------Solve the graph: iterate and find the k nearest neighbors for every node----------------//

//Improvements:
//Maybe a little faster in Region A?
bool KNN::solve(unsigned int k, double sampling = 1, double delta = 0) {
    std::cout << "Initializing graph... " << std::flush;
    typedef std::chrono::high_resolution_clock timer;
    timer::time_point start = timer::now();
    if(this->distf == nullptr) {
        std::cerr << "solve() failed: distance metric not set" << std::endl; 
        return false;        
    }

    if(k >= graph.get_size()) { 
        std::cerr << "solve() failed: k >= graph" << std::endl; 
        return false; 
    }

    //Make first neighbor approximation approximation
    unsigned int i;
    double sample_size = sampling * k;
    double threshold = delta * k * graph.get_size();
    #pragma omp parallel for num_threads(threads) private(i)
    for(i=0; i < graph.get_size(); i++) {
        graph[i].Cedge.set_cap(k);
        graph[i].edge.Old.set_cap(k);
        this->krand_neighbors(i, k);
    }

    timer::time_point finish = timer::now();
    std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "initialization complete | Time elapsed: " << elapsed.count() << "ms" << std::endl;
    std::cout << "Solving graph... " << std::flush;
    start = timer::now();

    double change;
    RN_AVL<unsigned int> sampledNR;
    minAVL<float, unsigned int> sampledN;
    unsigned int j, m, tgt_node, iterations = 0;
    struct minAVL<float, unsigned int>::payload rem;
    do {
        timer::time_point regA_S = timer::now();
        #pragma omp parallel for \
            private(i, j, m, tgt_node, rem, sampledN, sampledNR)
        for(i=0; i < graph.get_size(); i++) {
            //Sample neighbors
            for(j=0; j < sample_size; j++) {
                if(!graph[i].edge.New.is_empty()) {
                    rem = graph[i].edge.New.remove_min();
                    sampledN.insert(rem.key, rem.data);
                }

                if(!graph[i].Redge.New.is_empty())
                    sampledNR.New.insert(graph[i].Redge.New.remove_min());

                if(j < graph[i].Redge.Old.get_size())
                    sampledNR.Old.insert(graph[i].Redge.Old[j]);
            }

            //New neighbors X other
            for(j=0; j < sampledN.get_size(); j++) {
                tgt_node = sampledN[j].data;
                for(m=j+1; m < graph[i].edge.New.get_size(); m++)
                    this->add_candidates(tgt_node, sampledN[m].data);
                
                for(m=0; m < graph[i].edge.Old.get_size(); m++)
                    this->add_candidates(tgt_node, graph[i].edge.Old[m].data);
                
                for(m=0; m < sampledNR.get_size(); m++)
                    this->add_candidates(tgt_node, sampledNR[m]);
            }

            //New reverse neighbors X other
            for(j=0; j < sampledNR.New.get_size(); j++) {
                tgt_node = sampledNR.New[j];
                for(m=j+1; m < sampledNR.New.get_size(); m++)
                    this->add_candidates(tgt_node, sampledNR.New[m]);

                for(m=0; m < sampledNR.Old.get_size(); m++)
                    this->add_candidates(tgt_node, sampledNR.Old[m]);

                for(m=0; m < graph[i].edge.Old.get_size(); m++)
                    this->add_candidates(tgt_node, graph[i].edge.Old[m].data);
            }

            //Make new neighbors and new reverse neighbors old
            while(!sampledN.is_empty()) {
                rem = sampledN.remove_min();
                graph[i].edge.Old.insert(rem.key, rem.data);
            }

            while(!sampledNR.New.is_empty())
                graph[i].Redge.Old.insert(sampledNR.New.remove_min());

            sampledNR.Old.clear();
        } timer::time_point regA_F = timer::now();
        auto elapsedA = std::chrono::duration_cast<std::chrono::milliseconds>(regA_F - regA_S);
        //std::cout << "Region A: " << elapsedA.count() << "ms" << std::endl;

        //Parallel region B
        change = 0;
        timer::time_point regB_S = timer::now();
        #pragma omp parallel for private(i, tgt_node, rem) reduction(+:change)
        for(i=0; i < graph.get_size(); i++) {
            while(!graph[i].Cedge.is_empty()) {
                rem = graph[i].Cedge.remove_min();
                if(rem.key < graph[i].edge.max_key()) { //Better key found
                    if(graph[i].edge.find(rem.data) || graph[i].edge.find_key(rem.key)) //Check that neighbor does not already exist
                        continue;
            
                    tgt_node = graph[i].edge.remove_max().data;
                    omp_set_lock(&(graph[tgt_node].lock));
                    graph[tgt_node].Redge.remove(i); //Remove reverse neighbor from old neighbor
                    omp_unset_lock(&(graph[tgt_node].lock));
            
                    omp_set_lock(&(graph[rem.data].lock));
                    graph[rem.data].Redge.New.insert(i); //Add new reverse neighbor to new neighbor
                    omp_unset_lock(&(graph[rem.data].lock));
                    graph[i].edge.New.insert(rem.key, rem.data);

                    change++;
                } else break;
            } graph[i].Cedge.clear();
        } timer::time_point regB_F = timer::now();
        auto elapsedB = std::chrono::duration_cast<std::chrono::milliseconds>(regB_F - regB_S);
        //std::cout << "Region B: " << elapsedB.count() << "ms" << std::endl;
        iterations++;
    } while(change > threshold);

    //Evaluation
    finish = timer::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "graph solved at " << iterations << " iterations";
    std::cout << " | Time elapsed: " << elapsed.count() << "ms" << std::endl;

    //Transfer flagged nodes
    #pragma omp parallel for num_threads(threads) private(i)
    for(i=0; i < graph.get_size(); i++) {
        while(!graph[i].edge.New.is_empty()) {
            rem = graph[i].edge.New.remove_min();
            graph[i].edge.Old.insert(rem.key, rem.data);
        }

        if(graph[i].edge.Old.get_size() != k)
            std::cerr << "Warning: node " << i << " with " << graph[i].edge.Old.get_size() << " neighbors after solve" << std::endl;
    } return true;
}

//True solution
void KNN::true_solve(unsigned int k = 0) {
    typedef std::chrono::high_resolution_clock timer;
    std::cout << "Solving graph... " << std::flush;
    timer::time_point start = timer::now();

    if(this->distf == nullptr) {
        std::cerr << "true_solve() failed: distance metric not set" << std::endl; 
        return;   
    }

    float key;
    unsigned int i, j;
    #pragma omp parallel for num_threads(threads) private(key, i, j)
    for(i=0; i < graph.get_size(); i++) {
        for(j = i+1; j < graph.get_size(); j++) {
            key = calc_dist(i, j);
            omp_set_lock(&(graph[i].lock));
            graph[i].edge.Old.insert(key, j);
            omp_unset_lock(&(graph[i].lock));

            omp_set_lock(&(graph[j].lock));
            graph[j].edge.Old.insert(key, i);
            omp_unset_lock(&(graph[j].lock));
        }
    } timer::time_point finish = timer::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "graph solved | Time elapsed: " << elapsed.count() << "ms" << std::endl;
}

//Evaluate accuracy of KNN graph
bool KNN::accuracy(unsigned int k, char *path = nullptr) {
    typedef std::chrono::high_resolution_clock timer;
    std::cout << "Calculating accuracy... " << std::flush;
    timer::time_point start = timer::now();

    //Attempt to open solved graph
    char in_path[100] = { '\0' };
    if(path == nullptr)
        path = this->dataset;
    
    if(sprintf(in_path, "%s%s.solved%d", EXPORT_PATH, path, k) < 0) { 
        std::cerr << "accuracy() failed: sprintf() failed" << std::endl; 
        return false;
    }

    unsigned int i, j;
    FILE *FDC = nullptr;
    float true_neighbors[k + 1];
    minAVL<float, unsigned int> neighbors;
    double ret_acc_total = 0, true_acc_total = 0;
    double distance, best, worst, ret_acc, true_acc;
    #pragma omp parallel for num_threads(threads) reduction(+:ret_acc_total, true_acc_total) \
        private(i, j, neighbors, true_neighbors, distance, best, worst, ret_acc, true_acc, FDC)
    for(i=0; i < graph.get_size(); i++) {
        //Find true best neighbors
        worst = 0;
        if((FDC = fopen(in_path, "r")) == nullptr) {
            neighbors.set_cap(graph[i].edge.Old.get_size());
            for(j=0; j < graph.get_size(); j++) {
                if(i == j)
                    continue;
                
                distance = calc_dist(i, j);
                neighbors.insert(distance, j);
                if(distance > worst)
                    worst = distance;
            }
        } else {
            if(fseek(FDC, sizeof(float) * (k + 1) * i, SEEK_SET) < 0)
                perror("Warning: fdopen() failed");

            if(fread(true_neighbors, sizeof(float), k + 1, FDC) < k)
                perror("Warning: fdopen() failed");
            worst = true_neighbors[k];
        }

        //Decrease accuracy with each miss
        ret_acc = true_acc = graph[i].edge.Old.get_size();
        for(j=0; j < graph[i].edge.Old.get_size(); j++) {
            best = 0;            
            if(FDC) {
                if(graph[i].edge.Old[j].key != true_neighbors[j])
                    best = true_neighbors[j];
            } else {
                if(graph[i].edge.Old[j].data != neighbors[j].data)
                    best = neighbors[j].key;
            }
        
            if(best) {
                distance = graph[i].edge.Old[j].key;
                distance = (distance - best)/(worst - best);
                true_acc -= distance;
                ret_acc -= (1 - best/worst) * distance; //weighted by best's distance difference with worst
            }
        } neighbors.clear();
        ret_acc_total += ret_acc/graph[i].edge.Old.get_size();
        true_acc_total += true_acc/graph[i].edge.Old.get_size();
        if(FDC) {
            fclose(FDC);
            FDC = nullptr;
        }

    } timer::time_point finish = timer::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "relative accuracy: " << (ret_acc_total/graph.get_size())*100 << "%";
    std::cout << " | Absolute accuracy: " << (true_acc_total/graph.get_size())*100 << "%";
    std::cout << " | Time elapsed: " << elapsed.count() << "ms" << std::endl;
    return true;
}


//Remove all neighbors from every node
void KNN::clear() {
    for(unsigned int i=0; i < graph.get_size(); i++) {
        graph[i].edge.clear();
        graph[i].Redge.clear();
    } 
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