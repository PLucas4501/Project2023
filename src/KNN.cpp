#include "KNN.h"

//Key generating function
unsigned int KNN::key_function(struct pair n)
{
    //We don't want identical
    if(n.a == n.b)
        return 0;

    //Symmetry
    if(n.b < n.a) {
        unsigned int temp = n.b;
        n.b = n.a;
        n.a = temp;
    }

    unsigned int offset = 0;
    for(unsigned int i=0; i < n.a; i++)
        offset += this->graph.get_size() - (i+1);
    return offset + n.b;
}

unsigned int KNN::key_function(unsigned int a, unsigned int b)
{ 
    struct pair p{ a, b };
    return this->key_function(p);
}

//Create k unique random neighbors for node@graph[index], and calculate distances
//The approach takes a random index from one of two ranges [0, index), (index, last_index],
//and then splits that range into two based on the result. Then repeats k times.
void KNN::krand_neighbors(unsigned int index)
{
    if(index >= this->graph.get_size())
        throw std::out_of_range("graph index out of range");

    double dist;
    struct pair range;
    struct point p1, p2;
    vector<struct pair> rarr;
    unsigned int key, crange, cindex = index;

    //Create initial range(s)
    if(index > 1) //[0, index)
    {   
        range.a = 0;
        range.b = index;
        rarr.push(range);
    }

    if(index < this->graph.get_size() - 2) //(index, size -1]
    {
        range.a = index + 1;
        range.b = this->graph.get_size();
        rarr.push(range);
    }

    for(unsigned int i=0; i < this->k; i++)
    {
        //Find neighbor
        crange = rand()%rarr.get_size();
        cindex = rand()%(rarr[crange].b - rarr[crange].a) + rarr[crange].a;
    
        //Add neighbor after calculating distance
        //key generation guarantees symmetry
        key = key_function(index, cindex);
        if((dist = this->distances.find(key)) == 0) {
            p1.dim = p2.dim = this->dim;
            p1.cord = graph[index].cord;
            p2.cord = graph[cindex].cord;
            dist = this->dist(p1, p2);
            this->distances.insert(dist, key);
        } graph[index].edge.insert(cindex, dist);

        //Adds reverse neighbor to neighbor
        graph[cindex].reverse_edge.insert(index);
        
        //Split range: L is adjusted old, R is new
        range.b = rarr[crange].b;
        rarr[crange].b = cindex;
        range.a = cindex + 1;

        //Check if the ranges are valid
        if(rarr[crange].b - rarr[crange].a == 0)
            rarr.remove(crange);

        if(range.b - range.a > 0 )
            rarr.push(range);
    }
}

//------PUBLIC INTERFACE------//
//Constructor takes dimensions of the problem's space and k
//Optionally, a distance metric and a dataset array
KNN::KNN(
    unsigned int dim,
    unsigned int k,
    double (*dist)(struct point, struct point) = euclidean_distance,
    struct point data[] = nullptr, 
    unsigned int size = 0)
{
    this->k = k;
    this->dim = dim;
    if(dist != nullptr)
        this->dist = dist;
    else this->dist = euclidean_distance; //Default metric is euclidean

    srand(time(NULL));
    if(size > 0 && data != nullptr) {
        for(unsigned int i=0; i < size; i++) {
            if(data[i].dim != this->dim)
                throw std::logic_error("Dataset mismatch: incorrect dimensions");
            this->add_node(data[i]);
        }
    }
}


KNN::~KNN()
{
    for(unsigned int i=0; i < this->graph.get_size(); i++) {
        this->graph[i].edge.clear();
        this->graph[i].reverse_edge.clear();
        delete[] this->graph[i].cord;
    } this->graph.clear();

    this->distances.clear();
}

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
    vector<unsigned int> r_neighbors = this->graph[index].reverse_edge.all_elements();
    for(unsigned int i=0; i < r_neighbors.get_size(); i++)
    {
        std::cout <<  "\t";
        this->print_node(r_neighbors[i]);
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
    new_node.edge.set_cap(this->k); //Only k best neighbors are kept
    if(data.dim != this->dim) //Dimensions mismatch
        throw std::logic_error("Dataset mismatch: incorrect dimensions");

    new_node.cord = new double[this->dim];
    for(unsigned int i=0; i < this->dim; i++)
        new_node.cord[i] = data.cord[i];
    this->graph.push(new_node);
}

//Create neighbors for all nodes, and optionally set the distance metric and k
void KNN::initialize(double (*dist)(struct point, struct point) = nullptr, unsigned int k = 0)
{
    if(initialized) {
        std::cout << "Error: graph is already initialized" << std::endl;
        return;
    }

    if(dist != nullptr)
        this->dist = dist;

    if(this->k > this->graph.get_size())
        throw std::logic_error("unable to initialize knn graph: k > graph");

    if(k > 0)
        this->k = k;

    for(unsigned int i=0; i < this->graph.get_size(); i++)
    {
        this->graph[i].edge.set_cap(this->k);
        this->krand_neighbors(i);
    } initialized = true;
}


void KNN::initialize(void)
{ this->initialize(nullptr, 0); }


//Solve the graph: iterate and find the k nearest neighbors for every node
void KNN::solve()
{
    if(!initialized) {
        std::cout << "Error: graph is not initialized for solving" << std::endl;
        return;
    }

    double dist, new_dist;
    struct pair nodes; //The pair of examined nodes each time
    struct point p1{ this->dim, nullptr };
    struct point p2{ this->dim, nullptr };
    unsigned int key, iter, gsize = this->graph.get_size();

    vector<heap<unsigned int>> new_edges(gsize); 
    for(unsigned int i=0; i < new_edges.get_size(); i++) //Default is max, we want min heaps
        new_edges[i].reverse(); 

    //While change is still occuring
    bool change, global_change;
    do { 
        //For every node
        for(unsigned int i=0; i < gsize; i++)
        {
            //For every reverse neighbor of that node
            vector<unsigned int> r_neighbors = this->graph[i].reverse_edge.all_elements();
            for(unsigned int j=0; j < r_neighbors.get_size(); j++)
            {  
                nodes.a = r_neighbors[j];

                //For every rev_neighbor - neighbor combination
                for (unsigned int k=0; k < graph[i].edge.get_size(); k++)
                {
                    nodes.b = graph[i].edge[k];
                    if((key = key_function(nodes.a, nodes.b)) == 0) //a == b
                        continue;
            
                    //Looks if distance has already been calculated
                    //If it has, this neighbor has alredy been considered,
                    //so do not add him to candidates
                    if((dist = this->distances.find(key)) == 0) {
                        p1.cord = graph[nodes.a].cord;
                        p2.cord = graph[nodes.b].cord;
                        dist = this->dist(p1, p2);
                        this->distances.insert(dist, key);
                        new_edges[nodes.a].insert(nodes.b, dist); //Records possible neighbor, updates later
                        new_edges[nodes.b].insert(nodes.a, dist); //Notifies neighbor also
                    }
                }
            }
        }

        //Update graph: for every node, check the best k candidates found,
        //and see if they can replace the current ones.
        //If any change happens, we proceed to the next iteration.;
        iter = 0;
        global_change = false;
        for(unsigned int i=0; i < gsize; i++)
        {
            change = false;
            std::cout << "Candidates for " << i << ": ";
            new_edges[i].print();
            this->print_full_node(i);
            while(!new_edges[i].empty()) {
                dist = graph[i].edge.key(0);
                new_dist = new_edges[i].key(0);
                if(new_dist < dist) {
                    //std::cout << "---->Swapping " << graph[i].edge[0] << " with " << new_edges[i][0] << std::endl;
                    graph[new_edges[i][0]].reverse_edge.insert(i); //Add reverse neighbor to new neighbor
                    graph[graph[i].edge[0]].reverse_edge.remove(i); //Remove reverse neighbor from old neighbor
                    graph[i].edge.insert(new_edges[i][0], new_dist); //Replace edge with new edge
                    new_edges[i].remove(0); //Look for the next one
                    change = true;
                    global_change = true;
                } else break;
            } std::cout << std::endl;
            
            if(change) {
                std::cout << "--------NEW--------" << std::endl;
                this->print_full_node(i);
            }
        } iter++;
    } while(global_change);

    std::cout << iter << " iterations" << std::endl;
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
