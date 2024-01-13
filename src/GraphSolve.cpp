#include <iostream>
#include <cstring>

#include <getopt.h>
#include <unistd.h>

#include "KNN.h"

using namespace std;

//Solve true graph for a given dataset (and optionally for a given k)
//Improvements: output binary format
int main(int argc, char *argv[]) {
    int opt;
    char *path;
    unsigned int k = 0, threads = 8;
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
 
    KNN knn_problem(path, threads);
    knn_problem.set_metric(NORM);
    knn_problem.true_solve(k);
    knn_problem.export_graph(path, k);

    return 0;
}