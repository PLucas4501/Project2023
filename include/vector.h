
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

    //Proxy class to intercept calls to operator[]
    class vector_proxy
    {
        vector<T> &array;
        unsigned int index;
    public:
        vector_proxy(vector<T> &array, unsigned int index):
            array(array), index(index) {}
        
        unsigned int &operator=(T data)
        {
            this->array.array[index] = data;
            return this->array.array[index];
        }

        T operator[](unsigned int index)
        { return this->array.array[index]; }
    };

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
    { if(this->size > 0) delete [] this->array; }

    unsigned int const get_size()
    { return this->size; }

    bool const empty()
    { return this->size == 0; }


    //Just for element access
    T &operator[](unsigned int index)
    { 
        if(index >= this->size)
            throw std::out_of_range("vector index out of range");
        return this->array[index];
    }


    //Insert data@array[index] and resize appropriately
    void insert(T data, unsigned int index)
    {
        if(index > this->size)
            throw std::out_of_range("vector index out of range");

        T *new_array = new T[this->size + 1];
        if(new_array == nullptr)
            throw std::runtime_error("memory allocation failure");
        this->size += 1;

        for(unsigned int i = 0; i < index; i++)
            new_array[i] = this->array[i];
        new_array[index] = data;

        for(unsigned int i = index + 1; i < this->size; i++)
            new_array[i] = this->array[i-1];
            
        delete [] this->array;
        this->array = new_array;
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

        this->size -= 1;
        if(this->size > 0)
        {
            T *new_array = new T[this->size - 1];
            if(new_array == nullptr)
                throw std::runtime_error("memory allocation failure");

            *new_array = *(this->array);
            delete [] this->array;
            this->array = new_array;
        }
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