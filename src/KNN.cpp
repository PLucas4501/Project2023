#include "KNN.h"

//Key generating function
unsigned int KNN::key_function(struct pair n) {
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
        offset += graph.get_size() - (i+1);
    return offset + n.b;
}

unsigned int KNN::key_function(unsigned int a, unsigned int b) { 
    struct pair p{ a, b };
    return this->key_function(p);
}

//Create k unique random neighbors for node@graph[index], and calculate distances
//The approach takes a random index from one of two ranges [0, index), (index, last_index],
//and then splits that range into two based on the result. Then repeats k times.
void KNN::krand_neighbors(unsigned int index) {
    if(index >= graph.get_size())
        throw std::out_of_range("graph index out of range");

    struct pair range;
    struct point p1, p2;
    vector<struct pair> rarr;
    unsigned int crange, cindex = index;

    //Create initial range(s)
    if(index > 1) { //[0, index)
        range.a = 0;
        range.b = index;
        rarr.push(range);
    }

    if(index < graph.get_size() - 2) { //(index, size -1]
        range.a = index + 1;
        range.b = graph.get_size();
        rarr.push(range);
    }

    //Add k neighbors
    p1.dim = p2.dim = this->dim;
    for(unsigned int i=0; i < this->k; i++) {
        //Find neighbor
        crange = rand()%rarr.get_size();
        cindex = rand()%(rarr[crange].b - rarr[crange].a) + rarr[crange].a;
    
        //Add neighbor to node, and reverse neighbor to neighbor (untested)
        p1.cord = graph[index].cord;
        p2.cord = graph[cindex].cord;
        if(graph[index].Uedge.insert(this->dist(p1, p2), cindex))
            graph[cindex].URedge.push(index);
        
        //Split range
        range.b = rarr[crange].b;
        rarr[crange].b = cindex;
        range.a = cindex + 1;
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
    float (*dist)(struct point, struct point) = euclidean_distance,
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


KNN::~KNN() {
    for(unsigned int i=0; i < graph.get_size(); i++) {
        graph[i].edge.clear();
        graph[i].Redge.clear();
        delete[] graph[i].cord;
    } graph.clear();
}

void KNN::print_node(unsigned int index) {
    if(index >= graph.get_size())
        throw std::out_of_range("graph index out of range");
    std::cout << "Node " << index << ": ";
    graph[index].edge.print();
}


//Print one node with its reverse neighbors
void KNN::print_full_node(unsigned int index) {
    if(index >= graph.get_size())
        throw std::out_of_range("graph index out of range");

    this->print_node(index);
    std::cout << "\tReverse:";
    graph[index].Redge.print();
}


//Print graph (no Rneighbors)
void KNN::print_graph() {
    for(unsigned int i=0; i < graph.get_size(); i++)
        this->print_node(i);
}


//Print full graph (w/ Rneighbors)
//Not recommended for large graphs (large k/graph size/dimensions)...
void KNN::print_full_graph() {
    for(unsigned int i=0; i < graph.get_size(); i++)
        this->print_full_node(i);
}


//We can add an arbitrary amount of nodes
void KNN::add_node(struct point data) {
    if(initialized) {
        std::cout << "Error: graph is already initialized" << std::endl;
        return;
    }

    struct node new_node;
    new_node.edge.set_cap(this->k); //Only k best neighbors are kept
    new_node.Cedge.set_cap(this->k);
    new_node.Uedge.set_cap(this->k); //Unecessary, but why not
    if(data.dim != this->dim) //Dimensions mismatch
        throw std::logic_error("Dataset mismatch: incorrect dimensions");

    new_node.cord = new float[this->dim];
    for(unsigned int i=0; i < this->dim; i++)
        new_node.cord[i] = data.cord[i];
    graph.push(new_node);
}

//Create neighbors for all nodes, and optionally set the distance metric and k
void KNN::initialize(float (*dist)(struct point, struct point) = nullptr, unsigned int k = 0) {
    if(initialized) {
        std::cout << "Error: graph is already initialized" << std::endl;
        return;
    }

    if(dist != nullptr)
        this->dist = dist;

    if(this->k > graph.get_size())
        throw std::logic_error("unable to initialize knn graph: k > graph");

    if(k > 0)
        this->k = k;

    for(unsigned int i=0; i < graph.get_size(); i++)
    {
        graph[i].edge.set_cap(this->k);
        this->krand_neighbors(i);
    } initialized = true;
}


void KNN::initialize(void)
{ this->initialize(nullptr, 0); }


//Solve the graph: iterate and find the k nearest neighbors for every node
void KNN::solve() {
    unsigned int iter = 0;
    if(!initialized) {
        std::cout << "Graph not initialized for solving" << std::endl;
        return;
    }

    //While change is still occuring
    do { 
        //For every node
        for(unsigned int i=0; i < graph.get_size(); i++) {
            if(graph[i].Uedge.is_empty() && graph[i].URedge.is_empty()) //No new neighbors
                continue;

            //Call thread
            funA(i);
        }

        //Update graph: for every node, check the best k candidates found,
        //and see if they can replace the current ones.
        //If any change happens, we proceed to the next iteration.
        this->change = false;
        for(unsigned int i=0; i < graph.get_size(); i++) {
            if(!graph[i].Cedge.is_empty())
                funB(i);
        } iter++;
    } while(this->change);
    std::cout << "Graph solved at " << iter << " iterations." << std::endl;
}


//Thread A placeholder: calculate candidates
//Cedges must either have locks, or candidates will be calculated localy on index,
//then evaluated on the next step (in which case, locks must be acquired on Uedge/URedge)
void KNN::funA(unsigned int index) {
    float dist;
    unsigned int nodeA, nodeB;
    struct point p1{ this->dim, nullptr };
    struct point p2{ this->dim, nullptr };

    //Step 1: new neighbors X old reverse neighbors
    while(!graph[index].Uedge.is_empty()) {
        nodeA = graph[index].Uedge.min();
        for(unsigned int i=0; i < graph[index].Redge.get_size(); i++) { 
            nodeB = graph[index].Redge[i]; //[] access to AVL is a bit time consuming
            if(nodeA == nodeB)
                continue;

            p1.cord = graph[nodeA].cord;
            p2.cord = graph[nodeB].cord;
            dist = this->dist(p1, p2); //Maybe remember to not calculate again in neighbor if possible?
            graph[nodeA].Cedge.insert(dist, nodeB); //<<<LOCK>>>
            graph[nodeB].Cedge.insert(dist, nodeA); //<<<LOCK>>>
        } graph[index].edge.insert(graph[index].Uedge.min_key(), graph[index].Uedge.min());
        graph[index].Uedge.remove_min();
    }

    //Step 2: New reverse neighbors X all neighbors (new + old)
    for(unsigned int i=0; i < graph[index].URedge.get_size(); i++) {  
        //Sensitive with bigger k (edge is AVL), can convert to array if space is OK
    
        nodeA = graph[index].URedge[i];
        graph[index].Redge.insert(nodeA); //Add to reverse neighbors
        for(unsigned int j=0; j < graph[index].edge.get_size(); j++) {
            nodeB = graph[index].edge[j];
            if(nodeA == nodeB)
                continue;

            p1.cord = graph[nodeA].cord;
            p2.cord = graph[nodeB].cord;
            dist = this->dist(p1, p2);
            graph[nodeA].Cedge.insert(dist, nodeB); //<<<LOCK>>>
            graph[nodeB].Cedge.insert(dist, nodeA); //<<<LOCK>>>
        }
    } graph[index].URedge.clear();
}


//Thread B placeholder: evaluate candidates and update graph
//Improvements: remake AVL::remove/insert to return data-key pair removed
void KNN::funB(unsigned int index) {
    //std::cout << "Node " << index << " candidates VS edges:" << std::endl << "\t";
    //graph[index].Cedge.print(); std::cout << "\t";
    //graph[index].edge.print();
    while(!graph[index].Cedge.is_empty()) {
        if(graph[index].Cedge.min_key() < graph[index].edge.max_key()) { //Better key found
            if(graph[index].edge.find(graph[index].Cedge.min_key())) {
                graph[index].Cedge.remove_min();
                continue;
            } graph[graph[index].Cedge.min()].URedge.push(index); //Add untested reverse neighbor to new neighbor <<<LOCK>>>
            graph[graph[index].edge.max()].Redge.remove(index); //Remove reverse neighbor from old neighbor <<<LOCK>>>  
            graph[index].Uedge.insert(graph[index].Cedge.min_key(), graph[index].Cedge.min());
            graph[index].edge.remove_max();
            graph[index].Cedge.remove_min();
            this->change = true;
        } else graph[index].Cedge.clear();
    }
}


//Remove all neighbors from every node
void KNN::clear() {
    for(unsigned int i=0; i < graph.get_size(); i++) {
        graph[i].edge.clear();
        graph[i].Redge.clear();
    } initialized = false; //Can re-initialize graph
}