#include <iostream>
#include "node.h"
#include "distance.h"

using namespace std;

int main(int argc, char *argv[])
{  
    srand(time(NULL));
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
        }

        temp = node_array[0];
        node_array[0] = node_array[i];
        node_array[i] = temp;

        /*cout << "Node " << i << ": " << endl;
        for(unsigned int j=0; j<k; j++)
            cout << "\t(" << node_array[i].edge[j]->cord[0] << ", " << node_array[i].edge[j]->cord[1] << ")" << endl;
        cout << endl;*/

        
    }







    return 0;
}