
#ifndef BHEAP_H
#define BHEAP_H

#include <cstdlib> //realloc

class binary_heap 
{
    struct heap_node
    {
        int key;
        void *data;
    };

    unsigned int size{ 0 };
    struct heap_node *heap{ nullptr };

    void heapifyMin(unsigned int index);

public:
    //Accessors
    unsigned int const get_size()
    { return this->size; }

    bool const empty()
    { return this->size == 0; }

    void *const operator[](unsigned int index)
    { return heap[index].data; }

    ~binary_heap();
    void insert(void *data, int key);
    void remove(unsigned int index);
    void swap(unsigned int a, unsigned int b); //Swaps heap nodes with index a,b
};

#endif