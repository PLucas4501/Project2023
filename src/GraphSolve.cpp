#include <iostream>
#include <cstring>
#include "data_import.h"
#include "distance.h"
#include "AVL.h"

int main(int argc, char *argv[]) {
    char path[100] = { '\0' };
    if(argc < 2) {
        std::cerr << "Invalid arguments" << std::endl;
        return -1;
    } strcat(path, IMPORT_PATH); 
    strcat(path, argv[1]);
    const unsigned int dim = 100;

    float dist;
    vector<struct point> graph;
    binary(path, dim, graph);

    int i = -1;
    char out[100] = { '\0' };
    strcat(out, EXPORT_PATH);
    while(argv[1][++i] != '\0') 
        if(argv[1][i] == '.')
            break;

    memcpy(out + strlen(out), argv[1], i);
    if(freopen(out, "a+", stdout) < 0) {
        std::cerr << "Error opening file" << std::endl;
        return -1;
    } std::cout << graph.get_size() << std::endl;
    vector<minAVL2<float, unsigned int>> neighbors(graph.get_size());
    for(unsigned int i=0; i < graph.get_size(); i++) {
        for(unsigned int j = i+1; j < graph.get_size(); j++) {
            dist = euclidean_distance(graph[i], graph[j]);
            neighbors[i].insert(dist, j);
            neighbors[j].insert(dist, i);
        } neighbors[i].print();
        neighbors[i].clear();
        delete [] graph[i].cord;
    } return 0;
}