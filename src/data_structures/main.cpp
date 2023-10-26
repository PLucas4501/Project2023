#include <iostream>
#inlcude "array.h"
#include "node.h"

using namespace std;

int main(int argc, char *argv[])
{  
    int index=0;
    Array KNN[argc];  // initialising the graph with K  neighboors
    while(index < argc){
        while(!argv[index].eof() ){
            KNN[index] = new  argv[index];
            index++;
            count << "inserted " << argv[index] << endl;            
        }
    }

    return 0;
}