#include <iostream>
#include "data_structures/node.h"
#include "distance_functions/euclidean_distance.h"

using namespace std;

int main(int argc, char *argv[])
{  
    srand(time(NULL));

    bool negative;
    node node_array[10];
    unsigned int dim = rand()%9 + 1;
    for(unsigned int i=0; i<10; i++)
    {
        node_array[i].N = dim;
        node_array[i].cord = new double[dim];
        cout << "Node " << i << ":" << endl;
        for(unsigned j=0; j<dim; j++)
        {
            node_array[i].cord[j] = rand()%101;
            node_array[i].cord[j] += (float)(1000000000)/rand();
            negative = rand()%2;
            negative ? node_array[i].cord[j] = -node_array[i].cord[j] : node_array[i].cord[j] = node_array[i].cord[j];
            cout << "\t" << node_array[i].cord[j] << endl;
        }
    }

    double distances[9];
    for(unsigned int i=1; i<10; i++)
    {
        cout << "Node " << i << ": ";
        distances[i] = euclidean_distance(node_array[0], node_array[i]);
        cout << distances[i] << endl;
    }

    return 0;
}