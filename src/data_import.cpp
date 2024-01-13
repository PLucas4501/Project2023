#include "data_import.h"

//Takes a dataset file with n elements of dimensions dim
int binary(const char* dataset, vector<struct point> &v) {
    FILE* file;
    if((file = fopen(dataset, "r")) == nullptr) {
        std::cerr << "binary failed(): could not open dataset file" << std::endl;
        return -1;
    }

    struct point element;
    unsigned int n, dim = 100;
    if(fread(&n, sizeof(unsigned int), 1, file) != 1) {
        std::cerr << "binary failed(): could not read dataset size" << std::endl;
        fclose(file);
        return -1;
    }

    element.dim = dim;
    for(unsigned int i=0; i < n; i++) {
        element.cord = new float[dim];
        if(fread(element.cord, sizeof(float), dim, file) < dim) {
            std::cerr << "error reading coordinates" << std::endl;
            delete [] element.cord;
            fclose(file);
            return-1;
        } v.push(element);
    } fclose(file);
    return 0;
}  
