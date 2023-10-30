#include "acutest.h"
#include "../include/distance.h"
#include "../include/quicksort.h"

node* create_node(int arr_size  ){
    struct node* node  = new node[arr_size]
    for(unsigned int i = 0 ; i < arr_size ; i++){
        node[i].dim=  1;
        node[i].cord = new double ;
        node[i].cord =  i ;
    }
    return  node ;
}
void print_nodes(node* nodes,int arr_size){
    for(unsigned int i = 0 ; i < arr_size ; i++)
        cout << "node :" << i << " = " << nodes[i] << endl;
}

void test_euclidean_distance(void ){
    node* node = create_node(4);
    TEST_CHECK(euclidean_distance(node[0],node[1]) == 1);
    TEST_CHECK(euclidean_distance(node[0],node[3]) == 9);
}

void test_manhattan_distance(void ){
    node* node = create_node(4);
    TEST_CHECK(manhattan_distance(node[0],node[1]) == 1);
    TEST_CHECK(manhattan_distance(node[0],node[3]) == 3);
}

void test_quickSort(void ){
    node* node = create_node(4);
    TEST_CHECK(manhattan_distance(node,0,4);
    print_nodes(node , 4);
}

TEST_LIST = {
	{ "find_euclidean_distance", test_euclidean_distance },
	{ "find_manhattan_distance", test_manhattan_distance },
    {"find_test_quicksort", test_quickSort},
	{ NULL, NULL, NULL  } // τερματίζουμε τη λίστα με NULL
};