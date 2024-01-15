#include "acutest.h"
#include "KNN.h"
#include "data_import.h"


node* create_node(int arr_size) {
    struct node* node  = new node[arr_size]
    for(unsigned int i = 0 ; i < arr_size ; i++){
        node[i].dim=  1;
        node[i].cord =  i ;
        node[i].reverse_edge.insert(&node[i], i);
    }
    return  node ;
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

void test_reading_datasets(void ){
    const char* dataset
    KNN KNN(2,2);
    reading_datasets(dataset , KNN);
    TEST_CHECK(KNN->graph[0].cord[0] = nullptr);
}

void test_KNN_insert(void) {
    struct node* node = create_node(1);
    KNN KNN(1,2);
    KNN.add_node(node[0])
    TEST_CHECK(KNN->graph[0].cord[0] == 0);
}


void test_data_import(void) {
    vector<struct point> point_vector;
    char dataset_path[100] = { '\0' };
    sprintf(dataset_path, "%s%s", IMPORT_PATH, path);   
    TEST_CHECK(binary(dataset_path, point_vector) < 0) 
}

void test_KNN_Problem(void) {
    // Redirect std::cerr to a stringstream to capture the output
    std::stringstream errorStream;
    std::streambuf* oldCerr = std::cerr.rdbuf();
    std::cerr.rdbuf(errorStream.rdbuf());
    char* path;
    int k=20,threads=4;
    double sampling = 0.8, delta = 0.001;

    KNN KNN_Problem(path,threads);
    TEST_CHECK(KNN_Problem.threads == 4);

    KNN_Problem.set_metric(NORM);
    TEST_CHECK(KNN_Problem.distf == &KNN::norm_distance);

    KNN_Problem.solve(k,0.8,0.001);
    TEST_CHECK(KNN_Problem.solve(k,sampling,delta));

    KNN_Problem.true_solve(k=0);
    // Restore the original std::cerr
    std::cerr.rdbuf(oldCerr);
    // Check the contents of the stringstream
    std::string errorMessage = errorStream.str();
    TEST_CHECK(errorMessage.find("true_solve() failed: distance metric not set") != std::string:npos);

    TEST_CHECK(KNN_Problem.accuracy(k,path));
    TEST_CHECK(KNN_Problem.export_graph(path,k));

    KNN_Problem.krand_neighbors(unsigned int index=1 , k)
    // Restore the original std::cerr
    std::cerr.rdbuf(oldCerr);
    // Check the contents of the stringstream
    std::string errorMessage = errorStream.str();
    TEST_CHECK(errorMessage.find("krand_neighbors() failed: graph index out of range") != std::string:npos);

    KNN_Problem.add_node(path[0]);
    // Restore the original std::cerr
    std::cerr.rdbuf(oldCerr);
    // Check the contents of the stringstream
    std::string errorMessage = errorStream.str();
    TEST_CHECK(errorMessage.find("add_node() failed: dimensions mismatch") != std::string:npos);
}

TEST_LIST = {
	{ "test_data_import",test_data_import },
    {"test_KNN_Problem",test_KNN_Problem},
	{ "find_euclidean_distance", test_euclidean_distance },
	{ "find_manhattan_distance", test_manhattan_distance },
    { "test_reading_datasets" , test_reading_datasets },
    { "test_KNN_insert" , test_KNN_insert },
	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};

int main(void){
   //Run Tests
   TEST_RUN();
   return TEST_REPORT();
}