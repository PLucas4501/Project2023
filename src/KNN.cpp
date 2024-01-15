#include <iostream>
#include <stdexcept>
#include <random>
#include <chrono>
#include <cstdio>
#include <cstring>

#include <omp.h>
#include <fcntl.h>

#include "KNN.h"

//Quickly add the first viable neighbors to node@graph[index]
void KNN::kfirst_neighbors(unsigned int index, unsigned int k) {
    if(index >= graph.get_size())
        throw std::out_of_range("krand_neighbors() failed: graph index out of range");

    if(graph[index].edge.New.get_size() >= k)
        return;

    unsigned int cindex=0;
    while(graph[index].edge.New.get_size() < k && cindex < graph.get_size()) {
        if(cindex == index) {
            cindex++;
            continue;
        }

        if(graph[index].edge.New.insert(calc_dist(index, cindex), cindex)) {
            omp_set_lock(&(graph[cindex].lock));
            graph[cindex].Redge.New.insert(index);
            omp_unset_lock(&(graph[cindex].lock));      
        } cindex++;
     }
}

//Create k unique random neighbors for node@graph[index] (super slow)
void KNN::krand_neighbors(unsigned int index, unsigned int k) {
    if(index >= graph.get_size())
        throw std::out_of_range("krand_neighbors() failed: graph index out of range");

    if(graph[index].edge.New.get_size() >= k)
        return;

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
        if(graph[index].edge.New.insert(calc_dist(index, cindex), cindex)) {
            omp_set_lock(&(graph[cindex].lock));
            graph[cindex].Redge.New.insert(index);
            omp_unset_lock(&(graph[cindex].lock));      
        }
    } candidates.clear();
}

//Random projection split
void KNN::RPT_split(
    vector<unsigned int> &range,
    vector<unsigned int> &rangeL,
    vector<unsigned int> &rangeR,
    vector <float> &hyperplane,
    float &offset) 
{
    //Random generator used
    std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
    unsigned int x1 = gen() % range.get_size();
    unsigned int x2 = gen() % range.get_size();
    if(x1 == x2)
        x2 = (x1 + 1) % range.get_size();

    //Extract indeces
    x1 = range[x1];
    x2 = range[x2];

    unsigned int i;
    float mid[this->dim];
    for(i=0; i < this->dim; i++) {
        mid[i] = (graph[x1].cord[i] + graph[x2].cord[i])/2;
        hyperplane[i] = graph[x1].cord[i] - graph[x2].cord[i];
    }

    offset = 0;
    for(i=0; i < this->dim; i++)
        offset += hyperplane[i] * mid[i];  

    float margin;
    unsigned int j;
    for(i=0; i < range.get_size(); i++) {
        margin = -offset;
        for(j=0; j < this->dim; j++)
            margin += hyperplane[j] * graph[range[i]].cord[j];

        if(margin < -E)
            rangeL.push(range[i]);
        else if(margin > E)
            rangeR.push(range[i]);
        else if(gen() % 2 == 0)
            rangeL.push(range[i]);
        else rangeR.push(range[i]);
    } 
    
    if(rangeL.get_size() ==  0 || rangeR.get_size() == 0)
        std::cerr << "One-sided split" << std::endl;

    return;
}

void KNN::RPT_create(struct RPTree &tree, vector<unsigned int> range, int depth = 100) {
    if(range.get_size() <= tree.leaf_size) {
        tree.add_leaf(range);
        return;
    }

    if(depth <= 0) {
        if(tree.leaf_size < range.get_size())
            while(range.get_size() > tree.leaf_size)
                range.pop();  
        tree.add_leaf(range);
        return;
    } 
    
    float offset;
    vector<float> hyperplane(this->dim);
    vector<unsigned int> rangeL, rangeR;
    RPT_split(range, rangeL, rangeR, hyperplane, offset);
    if(rangeL.get_size() ==  0 || rangeR.get_size() == 0)
        std::cerr << "One-sided split" << std::endl;

    RPT_create(tree, rangeL, depth - 1);
    unsigned int L = tree.nodes.get_size() - 1;

    RPT_create(tree, rangeR, depth - 1);
    unsigned int R = tree.nodes.get_size() - 1;

    tree.add_node(offset, L, R, hyperplane);
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

    if(this->worst)
        delete [] this->worst;
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

        #pragma omp parallel for num_threads(threads) 
        for(unsigned int j = 0; j < n; j++) {
            neighbors[j] = graph[i].edge.Old[j].key;
        }
    

        if(fwrite(neighbors, sizeof(float), n, out_file) < n) { 
            std::cerr << "export_graph() failed: fwrite() failed" << std::endl; 
            delete [] neighbors;
            fclose(out_file);
            return false;
        }

        if(this->worst)
            worst = this->worst[i];
        else worst = graph[i].edge.Old.max_key();
        
        
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
//Maybe a little faster in Region A with the use of distributions?
bool KNN::solve(unsigned int k, double sampling, bool fast_sampling, double delta, unsigned int trees, unsigned int leaf_size) {
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

    //Configure data
    unsigned int i, j, m;
    vector<unsigned int> range(graph.get_size());
    for(i=0; i < graph.get_size(); i++) {
        graph[i].Cedge.set_cap(k);
        graph[i].edge.Old.set_cap(k);
        graph[i].edge.New.set_cap(k);
        //this->krand_neighbors(i, k);
        range.push(i);
    }

    //Create RPT forest
    unsigned int *array;
    vector<unsigned int> sizes;
    vector<unsigned int *> candidates(graph.get_size()/leaf_size);
    #pragma omp parallel for num_threads(threads) private(i, j, m, array)
    for(i=0; i < trees; i++) {
        struct RPTree tree(leaf_size);
        this->RPT_create(tree, range);
        for(j=0; j < tree.nodes.get_size(); j++) {
            if(tree.nodes[j]->points.get_size() > 0) {
                if(!(array = new unsigned int[tree.nodes[j]->points.get_size()])) {
                    std::cerr << "KNN failed: mem allo failure" << std::endl;
                    exit(EXIT_FAILURE);
                }

                for(m=0; m < tree.nodes[j]->points.get_size(); m++)
                    array[m] = tree.nodes[j]->points[m];

                #pragma omp critical
                {
                    candidates.push(array);
                    sizes.push(tree.nodes[j]->points.get_size());
                } tree.nodes[j]->points.clear();
            }
        }
    } range.clear();

    //Initialize neighbors with RPTs
    float key = 0;
    unsigned int nodeA, nodeB;
    #pragma omp parallel for num_threads(threads) private(i, j, m, nodeA, nodeB, key)
    for(i=0; i < candidates.get_size(); i++) {
        for(j=0; j < sizes[i]; j++) {
            nodeA = candidates[i][j];
            for(m=j+1; m < sizes[i]; m++) {
                nodeB = candidates[i][m];
                if(nodeA == nodeB)
                    continue;

                key = 0;
                if(!(graph[nodeB].Redge.New.find(nodeA))) {
                    key = calc_dist(nodeA, nodeB);
                    omp_set_lock(&(graph[nodeA].lock));
                    if(graph[nodeA].edge.New.insert(key, nodeB)) {
                        omp_unset_lock(&(graph[nodeA].lock));

                        //Add reverse neighbor
                        omp_set_lock(&(graph[nodeB].lock));
                        graph[nodeB].Redge.New.insert(nodeA);
                        omp_unset_lock(&(graph[nodeB].lock));     
                    } else omp_unset_lock(&(graph[nodeA].lock)); 
                }    

                if(!(graph[nodeA].Redge.New.find(nodeB))) {
                    if(key == 0) { key = calc_dist(nodeA, nodeB); }
                    
                    omp_set_lock(&(graph[nodeB].lock));
                    if(graph[nodeB].edge.New.insert(key, nodeA)) {
                        omp_unset_lock(&(graph[nodeB].lock)); 

                        //Add reverse neighbor
                        omp_set_lock(&(graph[nodeA].lock));
                        graph[nodeA].Redge.New.insert(nodeB);
                        omp_unset_lock(&(graph[nodeA].lock));     
                    } else omp_unset_lock(&(graph[nodeB].lock));
                }    
            }
        } delete [] candidates[i];
    } sizes.clear();
    candidates.clear();

    //In case any node was left with less than k, quickly add to reach k
    #pragma omp parallel for num_threads(threads) private(i)
    for(i=0; i < graph.get_size(); i++)
        this->kfirst_neighbors(i, k);

    timer::time_point finish = timer::now();
    std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "initialization complete | Time elapsed: " << elapsed.count() << "ms" << std::endl;
    std::cout << "Solving graph... " << std::flush;
    start = timer::now();

    RN_AVL<unsigned int> sampledNR;
    minAVL<float, unsigned int> sampledN;
    unsigned int change, tgt_node, iterations = 0;
    struct minAVL<float, unsigned int>::payload rem;
    double sample_size = sampling * k, threshold = delta * k * graph.get_size();
    do {
        timer::time_point regA_S = timer::now();
        #pragma omp parallel for private(i, j, m, tgt_node, rem, sampledN, sampledNR)
        for(i=0; i < graph.get_size(); i++) {
            //Sample neighbors
            for(j=0; j < sample_size; j++) {
                if(!(graph[i].edge.New.is_empty())) {
                    if(fast_sampling)
                        rem = graph[i].edge.New.remove_min();
                    else if(j < graph[i].edge.New.get_size())
                        rem = graph[i].edge.New.remove_index(j);
                    else rem = graph[i].edge.New.remove_index(j % graph[i].edge.New.get_size());
                    sampledN.insert(rem.key, rem.data); 
                }

                if(!(graph[i].Redge.New.is_empty())) {
                    if(fast_sampling)
                        sampledNR.New.insert(graph[i].Redge.New.remove_min());
                    else if(j < graph[i].Redge.New.get_size())
                        sampledNR.New.insert(graph[i].Redge.New.remove_index(j));
                    else sampledNR.New.insert(graph[i].Redge.New.remove_index(j % graph[i].Redge.New.get_size()));
                }

                if(!(graph[i].Redge.Old.is_empty())) {
                    if(fast_sampling)
                        sampledNR.Old.insert(graph[i].Redge.Old.remove_min());
                    else if(j < graph[i].Redge.Old.get_size())
                        sampledNR.Old.insert(graph[i].Redge.Old.remove(j));
                    else sampledNR.Old.insert(graph[i].Redge.Old.remove(j % graph[i].Redge.Old.get_size()));
                }
            }

            //New neighbors X other
            for(j=0; j < sampledN.get_size(); j++) {
                tgt_node = sampledN[j].data;
                for(m=j+1; m < sampledN.get_size(); m++)
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
            while(!(sampledN.is_empty())) {
                rem = sampledN.remove_min();
                graph[i].edge.Old.insert(rem.key, rem.data);
            }

            while(!(sampledNR.is_empty()))
                graph[i].Redge.Old.insert(sampledNR.remove_max());

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
                //Better key found
                if(rem.key < graph[i].edge.max_key()) { 
                    //Check that neighbor does not already exist
                    if(graph[i].edge.find_key(rem.key))
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

    //Transfer any flagged nodes remaining
    #pragma omp parallel for num_threads(threads) private(i)
    for(i=0; i < graph.get_size(); i++) {
        while(!graph[i].edge.New.is_empty()) {
            rem = graph[i].edge.New.remove_min();
            graph[i].edge.Old.insert(rem.key, rem.data);
        } graph[i].Redge.clear(); //No longer needed

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

    unsigned int i, j;
    if(k > 0) {
        if(!(this->worst = new float[graph.get_size()])) {
            std::cerr << "true_solve() failed: new failed" << std::endl; 
            return;   
        }

        #pragma omp parallel for num_threads(threads) private(i)
        for(i=0; i < graph.get_size(); i++) {
            graph[i].edge.Old.set_cap(k);
            this->worst[i] = 0;
        }
    }

    float key;
    #pragma omp parallel for num_threads(threads) private(i, j, key)
    for(i=0; i < graph.get_size(); i++) {
        for(j = 0; j < graph.get_size(); j++) {
            if(i != j) {
                key = calc_dist(i, j);
                graph[i].edge.Old.insert(key, j);
    
                if(worst[i] == 0)
                { worst[i] = key; }

                else if(key > worst[i])
                { worst[i] = key; }
            }
        }
    } timer::time_point finish = timer::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "graph solved | Time elapsed: " << elapsed.count() << "ms" << std::endl;
}

//Evaluate accuracy of KNN graph
//Improvements: open any file that has k >= than requested
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
        
            if(best > 0) {
                distance = graph[i].edge.Old[j].key;
                distance = (distance - best)/(worst - best);
                true_acc -= distance;
                ret_acc -= (1 - best/worst) * distance; //weighted by best's distance difference with worst
            }
        } neighbors.clear();
        if(FDC) { fclose(FDC); }
        ret_acc_total += ret_acc/graph[i].edge.Old.get_size();
        true_acc_total += true_acc/graph[i].edge.Old.get_size();
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

//RPT functions
void KNN::RPTree::add_leaf(vector<unsigned int> &points) {
    struct RPTnode *new_node = new struct RPTnode;
    new_node->offset = INFINITY;
    new_node->L = new_node->R = 0;
    while(!(points.is_empty()))
        new_node->points.push(points.pop());
    nodes.push(new_node);
    leaves++;
}

void KNN::RPTree::add_node(float offset, unsigned int L, unsigned int R, vector<float> &hyperplane) {
    struct RPTnode *new_node = new struct RPTnode;
    new_node->offset = offset;
    new_node->L = L;
    new_node->R = R;
    while(!(hyperplane.is_empty()))
        new_node->hyperplane.push(hyperplane.pop());
    nodes.push(new_node);
}