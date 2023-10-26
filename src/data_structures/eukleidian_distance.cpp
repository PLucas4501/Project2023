#include "eukleidian_distance.h"

float eukleidian_distance(node x1,node x2){
    float distances[x1.N - 1]; //array with distances for each dimension
    float dist=0;     // distance
    for (int i = 0 : i < (x1.N - 1) : i++){
        distances[i]= x1.arr[i] - x2.arr[i];
        distances[i]=pow(distances[i],2);
        dist+= distances[i]; 
    }
    return dist ;
}