#include <getopt.h>

#include "KNN.h"

using namespace std;

int main(int argc, char *argv[])
{ 
    //Default parameters
    int opt, k = 20, threads = 4;
    char *path, metric[100] = { '\0' };
    double sampling = 0.8, delta = 0.001;

    //Very basic cmdl arg parsing, without testing input
    while((opt = getopt(argc, argv, "k:s:m:d:t:")) != -1) {
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
                if(delta > 1 || delta <= 0)
                { fprintf(stderr, "Invalid delta\n"); exit(EXIT_FAILURE); }
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
    cout << "File: " << path << " | k: " << k << " | Delta: " << delta << " | Sampling rate: " << sampling*100 << "% | Threads: " << threads << endl;
    
    //Make and run the problem
    KNN knn_problem(path, threads);
    knn_problem.initialize(k, sampling, delta);
    knn_problem.solve();
    knn_problem.accuracy();
    return 0;
}