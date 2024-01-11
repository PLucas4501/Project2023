#include <iostream>
#include <cstring>

#include <omp.h>
#include <getopt.h>
#include <unistd.h>

#include "data_import.h"
#include "distance.h"
#include "AVL.h"

using namespace std;

//Solve true graph for a given dataset (and optionally for a given k)
//Improvements: output binary format
int main(int argc, char *argv[]) {
    int opt;
    unsigned int k = 0, threads = 8;
    char *path, in_path[100] = { '\0' }, out_path[110] = { '\0' };
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
    


    //Create graph from imported dataset
    vector<struct point> graph;
    if(sprintf(in_path, "%s%s", IMPORT_PATH, path) < 0)
    { cerr << "Error importing dataset " << out_path << endl; exit(EXIT_FAILURE); }

    if(binary(in_path, graph) < 0)
    { cerr << "Error importing dataset " << out_path << endl; exit(EXIT_FAILURE); }

    //Create output file
    unsigned int i = 0, j;
    if(sprintf(out_path, "%s%s.solved", EXPORT_PATH, path) < 0)
    { cerr << "Error creating file " << out_path << endl; exit(EXIT_FAILURE); }

    FILE *out_file;
    if((out_file = fopen(out_path, "w")) < 0) 
    { cerr << "Error creating file " << out_path << endl; exit(EXIT_FAILURE); } 

    //Configure data structures
    omp_lock_t lock[graph.get_size()];
    unsigned int graph_size = graph.get_size();
    minAVL<float, unsigned int> neighbors[graph_size];
    for(i=0; i < graph_size; i++) {
        neighbors[i].set_cap(k);
        omp_init_lock(&(lock[i]));
    }

    //Write size of dataset first
    if(fwrite(&graph_size, sizeof(unsigned int), 1, out_file) < 0)
    { cerr << "Error writing to file" << endl; exit(EXIT_FAILURE); }        

    //Find neighbors
    float dist;
    #pragma omp parallel for private(i, j, dist) num_threads(threads)
    for(i=0; i < graph_size; i++) {
        for(j = i+1; j < graph_size; j++) {
            dist = euclidean_distance(graph[i], graph[j]);
            omp_set_lock(&(lock[i]));
            neighbors[i].insert(dist, j);
            omp_unset_lock(&(lock[i]));

            omp_set_lock(&(lock[j]));
            neighbors[j].insert(dist, i);
            omp_unset_lock(&(lock[j]));
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
    return 0;
}