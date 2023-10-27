#include <iostream>
#inlcude "array.h"
#include "node.h"

using namespace std;

int main(int argc, char *argv[])
{  
    int index=0;
    Array KNN[argc];  
    while(index < argc){
        while(!argv[index].eof() ){
            KNN[index] = new  argv[index];  //  initialising the graph with values from the dataset
            index++;
            count << "inserted " << argv[index] << "at index" << index << endl;            
        }
    }

    return 0;
}