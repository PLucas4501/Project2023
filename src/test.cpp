#include <iostream>
#include <random>
#include <chrono>

#include "../include/AVL.h"

using namespace std;

int main() {
    vector<struct point> test;
    for(unsigned int i=0; i<5000; i++) {
        test[i].dim = 2;
        test[i].cord = new float[2];
        for(unsigned int j=0; j<2; j++)
            test[i].cord[j] = j;
    }

    KNN knn_problem(test);
    knn_problem.initialize(2);
    knn_problem.solve(4);

    for(unsigned int i=0; i<5000; i++) 
        delete test[i].cord;
}

/*
    struct pair range;
    vector<struct pair> rarr;

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
    std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
    for(unsigned int i=0; i < this->k; i++) {
        //Find neighbor
        crange = gen()%rarr.get_size();
        cindex = gen()%(rarr[crange].b - rarr[crange].a) + rarr[crange].a;
    
        //Add neighbor to node, and reverse neighbor to neighbor (untested)
        p1.cord = graph[index].cord;
        p2.cord = graph[cindex].cord;
        if(graph[index].edge.New.insert(this->dist(p1, p2), cindex))
            graph[cindex].Redge.New.insert(index);
        
        //Split range
        range.b = rarr[crange].b;
        rarr[crange].b = cindex;
        range.a = cindex + 1;
        if(rarr[crange].b - rarr[crange].a == 0)
            rarr.remove(crange);

        if(range.b - range.a > 0 )
            rarr.push(range);
    } */