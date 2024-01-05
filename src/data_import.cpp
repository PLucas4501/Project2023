#include "data_import.h"

//Takes a dataset file with n elements of dimensions dim
void binary(const char* dataset, vector<struct point> &v) {
    FILE* file = fopen(dataset, "rb");
    if(!file)
        return;

    struct point element;
    unsigned int n, dim = 100;
    if(fread(&n, sizeof(uint32_t), 1, file) != 1) {
        fclose(file);
        return;
    }

    element.dim = dim;
    for(unsigned int i=0; i < n; i++) {
        element.cord = new float[dim];
        for(unsigned int j=0; j < dim; j++) {
            if(fread(&(element.cord[j]), sizeof(float), 1, file) != 1) {
                std::cerr << "error reading coordinates" << std::endl;
                delete [] element.cord;
                fclose(file);
                return;
            }
        } v.push(element);
    } fclose(file);
    return;
}  
