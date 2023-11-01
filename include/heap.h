
#ifndef HEAP_H
#define HEAP_H

//A templated binary heap data structure with basic operations
//Depends on vector

#include <cstdlib>
#include <stdexcept>
#include "vector.h"

template <typename T>
class heap 
{
    //heap node prototype
    struct heap_node
    {
        T data;
        double key;
    };

    //The heap itself (vector is made by us)
    vector<struct heap_node> heap_array;

    //Private function for heapification
    void heapifyMin(unsigned int index)
    {
        unsigned int L = 2*index + 1, R = 2*index + 2;
        unsigned int min = index;
        if (L < heap_array.get_size() && heap_array[L].key < heap_array[min].key)
            min = L;

        if (R < heap_array.get_size() && heap_array[R].key < heap_array[min].key)
            min = R;

        if (min != index)
        {
            heap_array.swap(index, min);
            heapifyMin(min);
        }
    }

    //Swaps 2 elements, only useful for internal functions
    void swap(unsigned int a, unsigned int b)
    {
        if(a >= this->size || b >= this->size)
            throw std::out_of_range("heap index out of range");
        heap_array.swap(a,b);
    }

public:
    //Accessors
    unsigned int const get_size()
    { return heap_array.get_size(); }

    bool const empty()
    { return heap_array.empty(); }

    T const operator[](unsigned int index)
    { 
        if(index >= this->get_size())
            throw std::out_of_range("heap index out of range");
        return heap_array[index].data;
    }

    //Mutators
    //Inserts element & rebalances tree
    void insert(T data, double key)
    {
        unsigned int parent;
        unsigned int index = heap_array.get_size();
        struct heap_node new_node{ data, key };
        heap_array.push(new_node);

        while(index > 0)
        {
            parent = (index - 1)/2;
            if (heap_array[index].key < heap_array[parent].key)
            {
                heap_array.swap(index, parent);
                index = parent;
            } else break;
        }
    }


    //Removes element@heap_array[index] and rebalances tree
    T remove(unsigned int index = heap_array.get_size() - 1)
    {
        unsigned int last = heap_array.get_size() - 1;
        heap_array.swap(index, last);
        if(heap_array[index].key > heap_array[last].key)
            heapifyMin(index);
        return heap_array.pop();
    }


    //Remove all elements
    void clear()
    { this->heap_array.clear(); }
};

#endif