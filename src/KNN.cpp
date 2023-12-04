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
    unsigned int k,
    vector<struct point>& data,
    float (*dist)(struct point, struct point),
    char *path
) {
    this->k = k;
    this->dist = dist;
    this->dim = data[0].dim;
    this->dataset = path;
    for(unsigned int i=0; i < data.get_size(); i++) {
        if(data[i].dim != this->dim)
            throw std::logic_error("Dataset mismatch: incorrect dimensions");
        this->add_node(data[i]);
    }
}

KNN::KNN(unsigned int k, unsigned int dim) {
    this->k = k;
    this->dim = dim;
    this->dist = dist;
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
void KNN::initialize(double sampling = 1, double delta = 0.001) {
    if(initialized) {
        std::cout << "Error: graph is already initialized" << std::endl;
        return;
    }

    srand(time(NULL));
    if(this->k > graph.get_size())
        throw std::logic_error("unable to initialize knn graph: k > graph");

    this->delta = delta;
    this->sample_size = sampling * this->k;
    for(unsigned int i=0; i < graph.get_size(); i++) {
        graph[i].edge.set_cap(this->k);
        this->krand_neighbors(i);
    } initialized = true;
}


//Solve the graph: iterate and find the k nearest neighbors for every node
void KNN::solve() {
    unsigned int iter = 0;
    if(!initialized) {
        std::cout << "Graph not initialized for solving" << std::endl;
        return;
    }

    //While change is still occuring
    unsigned int termination = this->delta * this->k * this->graph.get_size();
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
        this->change = 0;
        for(unsigned int i=0; i < graph.get_size(); i++) {
            if(!graph[i].Cedge.is_empty())
                funB(i);
        } iter++;
    } while(this->change > termination);

    //fix this
    struct minAVL<float, unsigned int>::info rem;
    for(unsigned int i=0; i < graph.get_size(); i++) {
        while(!graph[i].Uedge.is_empty()) {
            rem = graph[i].Uedge.remove_max();
            graph[i].edge.insert(rem.key, rem.data);
        }
    } this->print_graph();
    std::cout << "Graph solved at " << iter << " iterations" << std::endl;

    //Open true graph
    int i = -1;
    unsigned int neighbor;
    char true_path[100] = { '\0' };
    vector<unsigned int> true_neighbors;
    double diff, accuracy, correct, total_accuracy = 0;
    strcat(true_path, EXPORT_PATH);
    while(this->dataset[++i] != '\0') 
        if(this->dataset[i] == '.')
            break;

    memcpy(true_path + strlen(true_path), this->dataset, i);
    FILE *true_graph = fopen(true_path, "r");
    if(!true_graph) {
        char command[100] = { '\0' };
        strcat(command, "GraphSolve ");
        strcat(command, this->dataset);
        system(command);
        if(!(true_graph = fopen(true_path, "r"))) {
            std::cout << "Unable to check for accuracy: could not open true graph file" << std::endl;
            return;
        }
    }

    fscanf(true_graph, "%*[^\n]\n");
    correct = graph.get_size()*this->k;
    for(unsigned int i=0; i < graph.get_size(); i++) {
        for(unsigned int j=0; j < graph.get_size() - 1; j++) {
            if(fscanf(true_graph, "%u", &neighbor) < 0) {
                std::cerr << "Error reading file (Node " << i << ", iteration " << j << ")" << std::endl;
                fclose(true_graph);
                return;                
            } true_neighbors.push(neighbor);
        }

        //Match found neighbors with real
        accuracy = 0;
        for(unsigned int j=0; j < this->k; j++) {
            if(graph[i].edge[j] != true_neighbors[j]) {
                correct--;
                for(unsigned int l = 0; l < true_neighbors.get_size(); l++) {
                    if(graph[i].edge[j] == true_neighbors[l]) {
                        diff = abs(j - l);
                        break;
                    }
                } accuracy += ((double) true_neighbors.get_size() - diff)/true_neighbors.get_size();
            } else accuracy += 1;
        } total_accuracy += accuracy/this->k;
        true_neighbors.clear();
    } fclose(true_graph);
    std::cout << "Accuracy: " << (total_accuracy/graph.get_size())*100 << "% | ";
    std::cout << (correct/(graph.get_size()*this->k))*100 << "\% perfect neighbors" << std::endl;
}


void KNN::funA(unsigned int index) {
    float dist;
    struct point p1{ this->dim, nullptr };
    struct point p2{ this->dim, nullptr };
    struct minAVL<float, unsigned int>::info rem;
    unsigned int nodeA, nodeB, sample = this->sample_size;

    //Step 1: new neighbors X old (neighbors + reverse neighbors)
    while(!graph[index].Uedge.is_empty()) {
        if(sample%2 == 0)
            rem = graph[index].Uedge.remove_min();
        else rem = graph[index].Uedge.remove_max();
        nodeA = rem.data;
        if(sample == 0) {
            graph[index].edge.insert(rem.key, rem.data);
            continue;
        } else sample--;

        //Other neighbors
        for(unsigned int i=0; i < graph[index].edge.get_size(); i++) {
            nodeB = graph[index].edge[i];
            p1.cord = graph[nodeA].cord;
            p2.cord = graph[nodeB].cord;
            dist = this->dist(p1, p2); //Maybe remember to not calculate again in neighbor if possible?
            graph[nodeA].Cedge.insert(dist, nodeB); //<<<LOCK>>>
            graph[nodeB].Cedge.insert(dist, nodeA); //<<<LOCK>>>
        } graph[index].edge.insert(rem.key, rem.data);
    
        //Old reverse
        sample = this->sample_size;
        for(unsigned int i=0; i < graph[index].Redge.get_size(); i++) {
            nodeB = graph[index].Redge[i];
            if(nodeA == nodeB)
                continue;
            if(sample-- == 0)
                break;
    
            p1.cord = graph[nodeA].cord;
            p2.cord = graph[nodeB].cord;
            dist = this->dist(p1, p2); //Maybe remember to not calculate again in neighbor if possible?
            graph[nodeA].Cedge.insert(dist, nodeB); //<<<LOCK>>>
            graph[nodeB].Cedge.insert(dist, nodeA); //<<<LOCK>>>
        }
    }

    //Step 2: New reverse neighbors X all neighbors (new + old)
    sample = sample_size;
    for(unsigned int i=0; i < graph[index].URedge.get_size(); i++) {  
        nodeA = graph[index].URedge[i];
        graph[index].Redge.insert(nodeA); //Add to reverse neighbors
        if(sample == 0)
            continue;
        else sample--;
    
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
void KNN::funB(unsigned int index) {
    //unsigned int local_change = 0; //For multithreading
    struct minAVL<float, unsigned int>::info rem;
    while(!graph[index].Cedge.is_empty()) {
        if(graph[index].Cedge.min_key() < graph[index].edge.max_key()) { //Better key found
            if(graph[index].edge.find(graph[index].Cedge.min_key())) {
                graph[index].Cedge.remove_min();
                continue;
            } rem = graph[index].Cedge.remove_min();
            graph[index].Uedge.insert(rem.key, rem.data);            
            graph[rem.data].URedge.push(index); //Add untested reverse neighbor to new neighbor <<<LOCK>>>
            rem = graph[index].edge.remove_max();
            graph[rem.data].Redge.remove(index); //Remove reverse neighbor from old neighbor <<<LOCK>>>  
            this->change += 1; //Multithreading: local_change += 1
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