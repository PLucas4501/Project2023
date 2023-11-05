#incldue "reading_datasets.h"


void reading_datasets(const char* dataset ,KNN*  KNN_problem ){
    FILE* file = fopen(dataset, "rb");
    if (file){
        while(){
            struct point element ;
            if(fread(&element.dim,sizeof(int),1,file) != 1)
                break;
            element.cord = new float[element.dim];
            if(fread(element.cord,sizeof(float),1,file) != 1){
                cout << "error reading coordinates" << endl;
                delete [] element.cord;
            }
            cout << "Number of dimensions: " << element.dimensions <<endl;
            cout << "Coordinates: ";
            for(unsigned int i = 0 ; i < element.dim < i++)
                cout << point.cord[i] << ""<< ;
            cout << endl;

            KNN_problem.add_node(element);

            delete[] point.cord; //free the dynamically allocated memory

        }        
        fclose(file);
    }
    
    else{
        cerr << "unable to open file" << endl;
    }
}

