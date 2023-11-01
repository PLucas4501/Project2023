
#ifndef VEC_H
#define VEC_H

//A templated vector data structure with basic operations

#include <iostream>
#include <cstdlib>
#include <stdexcept>

template <typename T> 
class vector
{
    T* array{ nullptr };
    unsigned int size{ 0 };

public:
    //Constructor can take an array of data + size (and hope size is right)
    vector(T data[] = nullptr, unsigned int size = 0)
    {
        if(data != nullptr && size > 0)
        {
            this->array =  new T[size];
            for(unsigned int i=0; i < size; i++)
                this->array[i] = data[i];
        }
    }

    ~vector()
    { if(this->size > 0) delete this->array; }

    unsigned int const get_size()
    { return this->size; }

    bool const empty()
    { return this->size == 0; }


    //Just for element access
    T const operator[](unsigned int index)
    { 
        if(index >= this->size)
            throw std::out_of_range("vector index out of range");
        return array[index];
    }


    //Works only for primitives
    /*void print()
    {
        if(this->size == 0)
        {
            std::cout << "[]" << std::endl;
            return;
        }

        std::cout << "[" << this->array[0];
        for(unsigned int i=1; i < this->size; i++)
            std::cout << ", " << this->array[i];
        std::cout << "]" << std::endl;
    }*/


    //Insert data@array[index] and resize appropriately
    void insert(T data, unsigned int index)
    {
        if(index > this->size)
            throw std::out_of_range("vector index out of range");

        if((this->array = (T*) realloc(this->array, (this->size + 1)*sizeof(T))) == nullptr)
            throw std::runtime_error("realloc failure");
        this->size += 1;

        for(unsigned int i = this->size - 1; i > index; i--)
            this->array[i] = this->array[i-1];
        this->array[index] = data;
    }


    void push(T data)
    { this->insert(data, this->size); }


    //Removes element@array[index] and resizes appropriately
    T remove(unsigned int index)
    {
        if(index >= this->size)
            throw std::out_of_range("vector index out of range");

        T value = this->array[index];
        for(unsigned int i = index; i < this->size - 1; i++)
            this->array[i] = this->array[i+1];

        if(this->size > 1)
            if((this->array = (T*) realloc(this->array, (this->size - 1)*sizeof(T))) == nullptr)
                throw std::runtime_error("realloc failure");
        this->size -= 1;

        return value;
    }


    //Remove last element
    T pop()
    { this->remove(this->size - 1); }


    //Remove all elements
    //We delete the array... we don't expect many clear-insertions to happen
    void clear()
    {
        this->size = 0;
        delete this->array;
    }


    //Swap elements array[a] <--> array[b]
    void swap(unsigned int a, unsigned int b)
    {
        if(a >= this->size || b >= this->size)
            throw std::out_of_range("vector index out of range");   

        T temp = this->array[a];
        this->array[a] = this->array[b];
        this->array[b] = temp;
    }
};

#endif