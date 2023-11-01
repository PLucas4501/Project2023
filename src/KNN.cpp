#include "KNN.h"

//Constructor takes dimensions of the problem's space and k
//Optionally, a distance metric and a dataset array
KNN::KNN(
    unsigned int k, 
    unsigned int dim, 
    double (*dist)(struct point, struct point) = euclidean_distance,
    struct point data[] = nullptr, 
    unsigned int size = 0)
{
    this->k = k;
    this->dim = dim;
    this->dist = dist;
    if(size > 0)
        for(unsigned int i=0; i < size; i++)
            this->add_node(data[i]);
}


KNN::~KNN()
{
    for(unsigned int i=graph.get_size()-1; i > 0; i--)
    {
        delete graph[i].cord;
        graph[i].edge.clear();
        graph[i].reverse_edge.clear(); 
    }
}


//We can add an arbitrary amount of nodes
void KNN::add_node(struct point data)
{
    struct new_node;
    if(data.dim != this->dim) //Dimensions mismatch
        throw logic_exception("Dataset mismatch: incorrect dimensions");

    new_node.cord = new double[this->dim];
    for(unsigned int i=0; i < this->dim; i++)
        new_node.cord[i] = data.cord[i];
    this->graph.push(new_node);
}


//Create k random neighbors for node@graph[index], and keep distances
//The approach takes a random index from one of two ranges [0, index), (index, last_index],
//and then splits that range into two based on the result. Then repeats.
int KNN::krand_neighbors(unsigned int index)
{
    if(index >= this->graph.get_size())
        std::throw out_of_range("graph index out of range");

    //if k == size, neighbors are not random
    if(this->k >= this->size)
        std::throw logic_exception("Inappropriate problem");

    srand(time(NULL));
    struct range
    { unsigned int a,b; };
    
    double dist;
    struct range r;
    struct point p1, p2;
    vector<struct range> rarr;
    unsigned int crange, cindex = index;
    if(index > 1)
    {   
        r.a = 0;
        r.b = index;
        rarr.push(r);
    }

    if(index < this->size - 2)
    {
        range.a = index + 1;
        range.b = this->size;
        rarr.push(r);
    }


    for(unsigned int i=0; i < this->k; i++)
    {
        //Find neighbor
        crange = rand()%rarr.get_size();
        cindex = rand()%(rarr[crange].b - rarr[crange].a) + rarr[crange].a;

        //Add neighbor after calculating distance || REMEMBER TO ADD HASH <-------------------------------------------------------------
        p1.dim = p2.dim = this->dim;
        p1.cord = graph[index].cord;
        p2.cord = graph[cindex].cord;
        dist = this->dist(p1, p2);
        graph[index].edge.insert(cindex, dist);
        graph[cindex].reverse_edge.push(index); //Adds reverse neighbor to neighbor
        
        //Split range
        r.b = rarr[cindex].b;
        rarr[cindex].b = cindex;
        r.a = cindex + 1;

        if(rarr[cindex].b - rarr[cindex].a == 1)
            rarr.remove(cindex);

        if(r.b - r.a > 1)
            rarr.push(r);
    }
}

//Create neighbors for all nodes
int KNN::initialize()
{
    for(unsigned int i=0; i < this->graph.get_size(); i++)
        this->krand_neighbors(i);
}


//Remove neighbors only
void KNN::clear()
{
    for(unsigned int i=graph.get_size()-1; i > 0; i--)
    {
        graph[i].edge.clear();
        graph[i].reverse_edge.clear();
    }
}