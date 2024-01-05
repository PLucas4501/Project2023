#include <iostream>
#include <cstring>

#include <omp.h>
#include <getopt.h>

#include "data_import.h"
#include "distance.h"
#include "AVL.h"

using namespace std;

//Solve true graph for a given dataset (and optionally for a given k)
//Improvements: output binary format
int main(int argc, char *argv[]) {
    int opt;
    unsigned int k = 0, threads = 8;
    char *path, in_path[100] = { '\0' }, out_path[100] = { '\0' };
    while((opt = getopt(argc, argv, "k:t:")) != -1) {
        switch(opt) {
            case 'k':
                k = atoi(optarg);
                if(k < 0)
                { fprintf(stderr, "Invalid k\n"); exit(EXIT_FAILURE); }
                break;
            case 't':
                threads = atoi(optarg);
                if(threads <= 0)
                { fprintf(stderr, "Invalid num of threads\n"); exit(EXIT_FAILURE); }
                break;
            default:
                fprintf(stderr, "Invalid CMD Line arguements\n");
                exit(EXIT_FAILURE);
        }
    } path = argv[optind];
    //cout << "File: " << path << " | k: " << k << " | Threads: " << threads << endl;
    


    //Create graph from dataset
    vector<struct point> graph;
    strcat(in_path, IMPORT_PATH);
    strcat(in_path, path);
    binary(in_path, graph);

    //Create output file
    unsigned int i = 0, j;
    strcat(out_path, EXPORT_PATH);
    while(path[i] != '\0') 
        if(path[i++] == '.')
            break;
    strncat(out_path, path, i - 1);

    if(freopen(out_path, "a+", stdout) < 0) 
    { cerr << "Error creating file " << out_path << endl; exit(EXIT_FAILURE); } 
    cout << graph.get_size() << endl; //First line of file is # of nodes

    //Configure data structures
    omp_lock_t lock[graph.get_size()];
    minAVL<float, unsigned int> neighbors[graph.get_size()];
    for(i=0; i < graph.get_size(); i++) {
        neighbors[i].set_cap(k);
        omp_init_lock(&(lock[i]));
    }
        

    //Find neighbors
    float dist;
    #pragma omp parallel for private(i, j, dist) num_threads(threads)
    for(i=0; i < graph.get_size(); i++) {
        for(j = i+1; j < graph.get_size(); j++) {
            dist = euclidean_distance(graph[i], graph[j]);
            omp_set_lock(&(lock[i]));
            neighbors[i].insert(dist, j);
            omp_unset_lock(&(lock[i]));

            omp_set_lock(&(lock[j]));
            neighbors[j].insert(dist, i);
            omp_unset_lock(&(lock[j]));
        }
    }

    for(i=0; i<graph.get_size(); i++) {
        neighbors[i].print();
        neighbors[i].clear();
        omp_destroy_lock(&(lock[i]));
    } return 0;
}