#include "acutest.h"
#include "distance.h"
#include "quicksort.h"
#include "binary_heap.h"

node* create_node(int arr_size  ){
    struct node* node  = new node[arr_size]
    for(unsigned int i = 0 ; i < arr_size ; i++){
        node[i].dim=  1;
        node[i].cord =  i ;
        node[i].reverse_edge.insert(&node[i], i);
    }
    return  node ;
}


void print_nodes(node* nodes,int arr_size){
    for(unsigned int i = 0 ; i < arr_size ; i++)
        cout << "node :" << i << " = " << nodes[i] << endl;
}

void test_euclidean_distance(void ){
    struct node* node = create_node(4);
    TEST_CHECK(euclidean_distance(node[0],node[1]) == 1);
    TEST_CHECK(euclidean_distance(node[0],node[3]) == 9);
}

void test_manhattan_distance(void ){
    node* node = create_node(4);
    TEST_CHECK(manhattan_distance(node[0],node[1]) == 1);
    TEST_CHECK(manhattan_distance(node[0],node[3]) == 3);
}

void test_binary_insert(void){
    
}

void test_binary_remove(void){

}

void test_binary_swap(void){
    
}
TEST_LIST = {
	{ "find_euclidean_distance", test_euclidean_distance },
	{ "find_manhattan_distance", test_manhattan_distance },
	{ NULL, NULL, NULL  } // τερματίζουμε τη λίστα με NULL
};