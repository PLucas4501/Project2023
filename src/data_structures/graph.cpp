#include "graph.h"

class graph{
    int K;
    list<node> *l; //List tha holds K nodes

    public:
        graph(int K){
            this->K = K;
            l = new list<nodes>[K];
        }

        void addedge(node X, node y){
            l[x].push_back(y);
            l[y].push_back(x);
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