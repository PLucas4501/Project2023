#include "graph.h"

class graph{
    int K;   // number of dimensions
    node *arr; // array with nodes

    public:
        graph(node arr[],int K){
            this->K = K;
            for(unsigned int i = 0 ; i < K ; i++){
                this -> arr[i] = arr[i] ;
            }
        }

        void PrintGraph(){
            for(int i=0;i<K;i++){
                cout << "vertex" << i << "->" ;
                for(int nbr:l[i]){
                    cout << nbr << "," << ;
                }
                cout  << endl;
            }
        }
}