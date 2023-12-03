#include "KNN.h"

using namespace std;

int main(int argc, char *argv[])
{ 
    int opt, k = 3;
    char metric[100] = { '\0' };
    double sampling = 1, delta = 0.001;
    while((opt = getopt(argc, argv, "k:s:m:d:")) != -1) {
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
            default:
                fprintf(stderr, "Invalid CMD Line arguements\n");
                exit(EXIT_FAILURE);
        }
    }

    char path[100] = { '\0' };
    strcat(path, IMPORT_PATH); 
    strcat(path, argv[optind]);

    vector <struct point> v;
    binary(path, 100, v);
    KNN knn_problem(k, v, euclidean_distance, argv[optind]);
    knn_problem.initialize(sampling, delta);
    knn_problem.solve();

    for(unsigned int i=0; i < v.get_size(); i++)
        delete [] v[i].cord;
    return 0;
}