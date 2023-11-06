#include "reading_datasets.h"


void reading_datasets(const char* dataset, KNN* KNN_problem) {
    FILE* file = fopen(dataset, "rb");
    struct point element{ 100, nullptr };
    unsigned int n;
    if(!file) return;

    if(fread(&n, sizeof(uint32_t), 1, file) != 1) {
        fclose(file);
        return;
    } std::cout << "Number of elements: " << n << std::endl;

    element.cord = new float[element.dim];
    for(unsigned int i=0; i < n; i++) {
        //std::cout << "Element " << i << ": (";
        if(fread(&element.cord[0], sizeof(float), 1, file) != 1) {
            std::cerr << "error reading coordinates" << std::endl;
            fclose(file);
            delete [] element.cord;
        } //std::cout << element.cord[0];

        for(unsigned int j=1; j < element.dim; j++) {
            if(fread(&element.cord[j], sizeof(float), 1, file) != 1) {
                //std::cerr << "error reading coordinates" << std::endl;
                fclose(file);
                delete [] element.cord;
            } //std::cout << ", " << element.cord[j];
        } //std::cout << ")" << std::endl;

        KNN_problem->add_node(element);
    }

    delete[] element.cord;
    return;
}  
    

