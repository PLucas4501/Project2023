#include <iostream>
#include "node.h"
#include "distance.h"

using namespace std;

int main(int argc, char *argv[])
{  
    srand(time(NULL));
    double distance;
    unsigned int arr_size = 6, dim = 2;
    struct node node_array[arr_size];
    for(unsigned int i=0; i<arr_size; i++)
    {
        node_array[i].dim = dim;
        node_array[i].cord = new double [2];
        node_array[i].cord[0] = node_array[i].cord[1] = i;
    }

    struct node temp;
    unsigned int randindex, offset, k = 2, range = (unsigned int) arr_size/k;
    for(unsigned int i=0; i<arr_size; i++)
    {
        temp = node_array[0];
        node_array[0] = node_array[i];
        node_array[i] = temp;

        offset = 1;
        node_array[0].edge = new struct node *[k];
        for(unsigned int j=0; j < k; j++)
        {
            randindex = rand()%range + offset;
            if(j+1 == k)
            {
                if((arr_size - 1)%k > 0)
                {
                    randindex = rand()%(arr_size - offset) + offset;
                }
            }

            offset += range;
            if(randindex == i)
                randindex = 0;
            node_array[0].edge[j] = &node_array[randindex];
            distance = manhattan_distance(node_array[0], node_array[randindex]);
            node_array[randindex].reverse_edge.insert(&node_array[i], distance);
        }

        temp = node_array[0];
        node_array[0] = node_array[i];
        node_array[i] = temp;
        
    }


    for(unsigned int i=0; i<arr_size; i++)
    {
        cout << "Node " << i << endl;
        cout << "Normal neighbors:" << endl;
        for(unsigned int j=0; j<k; j++)
            cout << "\t(" << node_array[i].edge[j]->cord[0] << ", " << node_array[i].edge[j]->cord[1] << ")" << endl;

        struct node *rn;
        unsigned int size = node_array[i].reverse_edge.get_size();
        cout << "Reverse neighbors (" << size << "):" << endl;
        for(unsigned int j=0; j < size; j++)
        {  
            rn = (struct node *) node_array[i].reverse_edge[j];
            cout << "\t(" << rn->cord[0] << ", " << rn->cord[1]  << ")" << endl;
        }
        cout << endl;
    }

    bool changed = "False";
    do { 
        changed = "True";
        for(unsigned int i=0; i<arr_size; i++) //gia kathe node
        {
            struct node *rn;
            for(unsigned int j=0; j < size; j++)
            {  
                
                rn = (struct node *) node_array[i].reverse_edge[j];
                double dist=euclidean_distance( node_array[i] , rn[j] );
                cout << "euclidean Distance from node" << node_array[i].cord[0] << "," << node_array[1] << " to reverse node" ;
                cout << rn.cord[0] << "," << rn.cord[1] << "is :" << dist << endl;
                changed=rn[j].reverse_edge.heapifyMin(dist);
                dist=manhattan_distance( node_array[i] , rn[j] );
                cout << "And manhattan " << dist endl;
            
            }

        } //kane ta updates (vgale apo ola ta neighbor heap ton nodes ta perita neighbors - kopse mexri na minoun k)
    }while(changed);

    return 0;
}