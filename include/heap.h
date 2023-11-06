
#ifndef HEAP_H
#define HEAP_H

//A templated binary minheap data structure with basic operations

#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include "vector.h"

template <typename T>
class heap 
{
protected:
    //heap node prototype
    struct heap_node
    {
        T data;
        double key;
    };

    bool rev; //Default is max heap (false), rev is min
    vector<struct heap_node> heap_array; //The heap itself (vector is made by us)

    //Changes depending on max or min heap mode
    bool compare(unsigned int a, unsigned int b)
    {
        if(!is_rev())
            return heap_array[a].key > heap_array[b].key;
        return heap_array[a].key < heap_array[b].key;
    }

    //Private function for heapification
    void heapify(unsigned int index)
    {
        if(index >= this->get_size())
            throw std::out_of_range("heap index out of range");

        unsigned int L = 2*index + 1, R = 2*index + 2;
        unsigned int best = index;
        if (L < this->get_size() && compare(L, best))
            best = L;

        if (R < this->get_size() && compare(R, best))
           best = R;

        if (best != index)
        {
            heap_array.swap(index, best);
            heapify(best);
        }
    }

    //Swaps 2 elements with indices a & b, only useful for internal functions
    void swap(unsigned int a, unsigned int b)
    {
        if(a >= this->get_size() || b >= this->get_size())
            throw std::out_of_range("heap index out of range");
        heap_array.swap(a,b);
    }

public:
    //Constructor optionally takes two arrays, one with data,
    //the other with the corresponding keys, and the size of them
    heap(T data[] = nullptr, double keys[] = nullptr, unsigned int size = 0, bool rev = false)
    {
        this->rev = rev;
        if(is_rev())
            std::cout << "Real shit?" << std::endl;
        if(data != nullptr && keys != nullptr && size > 0)
            for(unsigned int i=0; i < size; i++)
                this->insert(data[i], keys[i]);
    }

    bool const is_rev()
    { return this->rev; }

    //Accessors
    unsigned int const get_size()
    { return heap_array.get_size(); }

    unsigned int const key(unsigned int index)
    { 
        if(index >= this->get_size())
            throw std::out_of_range("heap index out of range");
        return heap_array[index].key; 
    }

    bool const empty()
    { return heap_array.empty(); }

    T operator[](unsigned int index)
    { 
        if(index >= this->get_size())
            throw std::out_of_range("heap index out of range");
        return heap_array[index].data;
    }

    void print()
    {
        if(heap_array.get_size() == 0) {
            std::cout << "[]" << std::endl;
            return;
        }   

        std::cout << "[" << heap_array[0].data;
        for(unsigned int i=1; i < heap_array.get_size(); i++)
            std::cout << ", " << heap_array[i].data;
        std::cout << "]" << std::endl;
    }

    void reverse()
    { if(this->empty()) rev = !rev; }

    //Inserts element into heap
    //Defined with bool return, so next class can override (look at k_rheap below)
    bool insert(T data, double key)
    {
        unsigned int parent, index;
        struct heap_node new_node{ data, key };
        heap_array.push(new_node);

        index = this->get_size() - 1;
        while(index > 0)
        {
            parent = (index - 1)/2;
            if(compare(index, parent))
            {
                heap_array.swap(index, parent);
                index = parent;
            } else break;
        }

        return true;
    }

    //Removes element@heap_array[index]
    T remove(unsigned int index)
    {

        unsigned int last = heap_array.get_size() - 1;
        this->swap(index, last);
        if(compare(last, index)) {
            unsigned int data = heap_array.pop().data;
            heapify(index);
            return data;
        } else return heap_array.pop().data;
    }


    //Remove all elements
    void clear()
    { this->heap_array.clear(); }
};


/*k_rheap (k-rev heap) inherits heap and allows only k worst elements to exist inside the heap.
Although counter-intuitive at first, it is useful for our problem:
A max k_rheap will fill itself like a normal heap until it reaches k elements,
after which it will insert new elements only if the new element is lesser than the root (max).
Essentially, a max k_rheap is a revd min heap, with O(1) access to the worst element.
Thus, we can measure improvements to our KNN graph by recording insertions on a node's neighbor k_rheap.
Note: uniqueness of the elements is not guaranteed here, but it is by the KNN implementation*/
template <typename T>
class k_rheap : public heap<T>
{
    //Heap capacity, 0 is infinite (normal heap)
    unsigned int cap;

public:
    //Constructor remains the same, let for the new capacity element
    k_rheap(
        unsigned int cap = 0,
        T data[] = nullptr, 
        double keys[] = nullptr,
        unsigned int size = 0,
        bool rev = false)
    {
        this->cap = cap;
        if(cap == 0)
            heap<T>(data, keys, size, rev);
        else heap<T>(data, keys, this->cap, rev);
    }


    //If cap is reached, k_rheap will insert only if the candidate is worse
    //We also want to know if insertion happened or not, so return bool
    bool insert(T data, double key)
    {
        unsigned int size = heap<T>::get_size();
        if(size < cap || cap == 0) {
            heap<T>::insert(data, key);
            return true;
        }
        
        //If k is reached and new element is worse,
        //Replace root and heapify
        struct heap<T>::heap_node new_node{ data, key };
        if(size == cap) {
            if(!heap<T>::is_rev() && key < heap<T>::heap_array[0].key) {
                heap<T>::heap_array[0] = new_node;
                heap<T>::heapify(0);
                return true;
            }

            if(heap<T>::is_rev() && key > heap<T>::heap_array[0].key) {
                heap<T>::heap_array[0] = new_node;
                heap<T>::heapify(0);
                return true;
            }
        }

        return false;
    }

    unsigned int const get_cap()
    { return this->cap; }

    //Set capacity to k, after doing the necessary changes
    void set_cap(unsigned int k) {
        int diff = this->cap - k;
        this->cap = k;
        if(k == 0)
            return;

        while(diff-- > 0)
            heap<T>::remove(0);
    }
};


#endif