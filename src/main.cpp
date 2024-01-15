#include <getopt.h>

#include "KNN.h"

using namespace std;

int main(int argc, char *argv[])
{ 
    //Default parameters
    bool fast_sampling = false;
    int opt, k = 20, threads = 4;
    char *path, metric[100] = { '\0' };
    double sampling = 0.8, delta = 0.001;
    unsigned int trees = 0, leaf_size = 0;

    //Very basic cmdl arg parsing, without testing input
    while((opt = getopt(argc, argv, "k:s:m:d:t:r:l:f")) != -1) {
        switch(opt) {
            case 'k':
                k = atoi(optarg);
                if(k < 0)
                { fprintf(stderr, "Invalid k\n"); exit(EXIT_FAILURE); }
                break;
            case 's':
                sampling = atof(optarg);
                if(sampling > 1 || sampling <= 0)
                { fprintf(stderr, "Invalid sampling rate\n"); exit(EXIT_FAILURE); }
                break;
            case 'm':
                strcat(metric, optarg);
                break;
            case 'd':
                delta = atof(optarg);
                if(delta > 1 || delta < 0)
                { fprintf(stderr, "Invalid delta\n"); exit(EXIT_FAILURE); }
                break;
            case 't':
                threads = atoi(optarg);
                if(threads <= 0)
                { fprintf(stderr, "Invalid num of threads\n"); exit(EXIT_FAILURE); }
                break;
            case 'r':
                trees = atoi(optarg);
                if(trees <= 0)
                { fprintf(stderr, "Invalid num of trees\n"); exit(EXIT_FAILURE); }
                break;
            case 'l':
                leaf_size = atoi(optarg);
                if(leaf_size <= 0)
                { fprintf(stderr, "Invalid leaf_size\n"); exit(EXIT_FAILURE); }
                break;
            case 'f':
                fast_sampling = true;
                break;
            default:
                fprintf(stderr, "Invalid CMD Line arguements\n");
                exit(EXIT_FAILURE);
        }
    } path = argv[optind];
    if(trees == 0)
    { trees = 12; }

    if(leaf_size == 0)
    { leaf_size = k/5; }

    cout << "File: " << path << " | k: " << k << " | Delta: " << delta << " | Sampling rate: " << sampling*100 << "%";
    cout << " | Fast sampling: " << fast_sampling << " | RPTrees: " << trees << " | RPT leaf size: " << leaf_size;
    cout << " | Threads: " << threads << endl;
    
    //Make and run the problem
    KNN knn_problem(path, threads);
    knn_problem.set_metric(NORM);
    knn_problem.solve(k, sampling, fast_sampling, delta, trees, leaf_size);
    knn_problem.accuracy(k, path);
    return 0;
}