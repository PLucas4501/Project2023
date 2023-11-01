#include "KNN.h"

//Constructor takes dimensions of the problem's space and k
//Optionally, a distance metric and a dataset array
KNN::KNN(
    unsigned int dim,
    unsigned int k,
    double (*dist)(struct point, struct point) = euclidean_distance,
    struct point data[] = nullptr, 
    unsigned int size = 0)
{
    this->dim = dim;
    if(k < 2) //No meaning for this
        throw std::logic_error("Inappropriate problem");
    this->k = k;
    
    if(dist != nullptr)
        this->dist = dist;
    else this->dist = euclidean_distance;

    if(size > 0 && data != nullptr)
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


//Print one node
void KNN::print_node(unsigned int index)
{
    if(index >= this->graph.get_size())
        throw std::out_of_range("graph index out of range");

    std::cout << "Node " << index << ": (" << graph[index].cord[0];
    for(unsigned int i=1; i < this->dim; i++)
        std::cout <<  ", " << graph[index].cord[i];
    std::cout << ")" << std::endl;
}


//Print one node with its neighbors
void KNN::print_full_node(unsigned int index)
{
    if(index >= this->graph.get_size())
        throw std::out_of_range("graph index out of range");

    this->print_node(index);
    std::cout << "Neighbors (k best):" << std::endl;
    for(unsigned int i=0; i < graph[index].edge.get_size(); i++)
    {
        std::cout <<  "\t";
        this->print_node(graph[index].edge[i]);
    }

    std::cout << "Reverse neighbors:" << std::endl;
    for(unsigned int i=0; i < graph[index].reverse_edge.get_size(); i++)
    {
        std::cout <<  "\t";
        this->print_node(graph[index].reverse_edge[i]);
    } std::cout << std::endl;
}


//Print graph (no neighbors)
void KNN::print_graph()
{
    for(unsigned int i=0; i < this->graph.get_size(); i++)
        this->print_node(i);
}


//Print full graph (w/ neighbors)
//Not recommended for large graphs (large k/graph size/dimensions)...
void KNN::print_full_graph()
{
    for(unsigned int i=0; i < this->graph.get_size(); i++)
        this->print_full_node(i);
}


//We can add an arbitrary amount of nodes
void KNN::add_node(struct point data)
{
    struct node new_node;
    if(data.dim != this->dim) //Dimensions mismatch
        throw std::logic_error("Dataset mismatch: incorrect dimensions");

    new_node.cord = new double[this->dim];
    for(unsigned int i=0; i < this->dim; i++)
        new_node.cord[i] = data.cord[i];
    this->graph.push(new_node);
}


//Create k random neighbors for node@graph[index], and keep distances
//The approach takes a random index from one of two ranges [0, index), (index, last_index],
//and then splits that range into two based on the result. Then repeats k times.
void KNN::krand_neighbors(unsigned int index)
{
    if(index >= this->graph.get_size())
        throw std::out_of_range("graph index out of range");

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

    if(index < this->graph.get_size() - 2)
    {
        r.a = index + 1;
        r.b = this->graph.get_size();
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
        r.b = rarr[crange].b;
        rarr[crange].b = cindex;
        r.a = cindex + 1;

        if(rarr[crange].b - rarr[crange].a == 0)
            rarr.remove(crange);

        if(r.b - r.a > 0 )
            rarr.push(r);
    }
}

//Create neighbors for all nodes, and optionally set the distance metric
void KNN::initialize(double (*dist)(struct point, struct point))
{
    if(dist != nullptr)
        this->dist = dist;

    if(!initialized)
    {
        initialized = true;
        for(unsigned int i=0; i < this->graph.get_size(); i++)
            this->krand_neighbors(i);
    }
}


void KNN::initialize(void)
{ this->initialize(nullptr); }


//Solve the graph: iterate and find the k nearest neighbors for every node
void KNN::solve()
{
    double distance;
    unsigned int x, y;
    bool changed = false;
    struct point p1{ this->dim, nullptr };
    struct point p2{ this->dim, nullptr };

    //While change is still occuring
    do
    { 
        //For every node
        for(unsigned int i=0; i < this->graph.get_size(); i++)
        {
            //For every reverse neighbor of that node
            for(unsigned int j=0; j < this->graph[i].reverse_edge.get_size(); j++)
            {  
                x = graph[i].reverse_edge[j];
                p1.cord = graph[x].cord;

                //For every rev_neighbor - neighbor combination
                for (unsigned int k=0; k < graph[i].edge.get_size(); k++)
                {
                    y = graph[i].edge[k];
                    p2.cord = graph[y].cord;
                    distance = this->dist(p1, p2);

                    //Q1: Mustn't be inserted, keep track of changes and insert afterwards
                    //Q2: How do we measure "change"?
                    graph[x].edge.insert(y, distance);
                }
            }
        }
    } while(changed);
}



//Remove all neighbors from every node
void KNN::clear()
{
    for(unsigned int i=0; i < this->graph.get_size(); i++)
    {
        graph[i].edge.clear();
        graph[i].reverse_edge.clear();
    } initialized = false; //Can re-initialize graph
}