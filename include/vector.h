
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
    unsigned int cap{ 0 };
    unsigned int size{ 0 };

public:
    //Constructor can take an array of data + size (and hope size is right)
    vector(T data[] = nullptr, unsigned int size = 0) {
        if(data != nullptr && size > 0)
        {
            this->cap = this->size = size;
            this->array = new T[size];
            for(unsigned int i=0; i < size; i++)
                this->array[i] = data[i];
        }
    }

    //Alternatively, reserve with size given
    vector(unsigned int size) {
        this->cap = size; 
        this->array = new T[size]; 
    }

    ~vector() {
        if(this->cap > 0)
            delete [] this->array;
    }

    //Just for element access
    T &operator[](unsigned int index) { 
        if(index >= this->cap)
            throw std::out_of_range("vector index out of range");
        return this->array[index];
    }

    void print() {
        std::cout << "[";
        if(!is_empty())
            std::cout << this->array[0];
        for(unsigned int i=1; i < this->size; i++)
            std::cout << ", " << this->array[i];
        std::cout << "]" << std::endl;
    }

    unsigned int const get_size()
    { return this->size; }

    bool const is_empty()
    { return this->size == 0; }


    //Insert data@array[index] and resize appropriately
    void insert(T data, unsigned int index) {
        if(index > this->cap)
            throw std::out_of_range("vector index out of range");

        if(index == cap) {
            this->cap += 4;
            T *new_array = new T[this->cap];
            if(new_array == nullptr)
                throw std::runtime_error("memory allocation failure");

            for(unsigned int i = 0; i < index; i++)
                new_array[i] = this->array[i];
            new_array[index] = data;

            for(unsigned int i = index + 1; i < this->size + 1; i++)
                new_array[i] = this->array[i-1];
            
            if(this->array)
                delete[] this->array;
            this->array = new_array;
        }

        else {
            this->array[this->size] = data;
            if(index != this->size)
                swap(index, this->size);
        } this->size += 1;

    }


    void push(T data)
    { this->insert(data, this->size); }


    //Removes element@array[index] and resizes appropriately
    T remove(unsigned int index) {
        if(index >= this->size)
            throw std::out_of_range("vector index out of range");

        T value = this->array[index];
        if(index != this->size - 1) {
            swap(index, this->size - 1);
        } this->size -= 1;
        return value;
    }


    //Remove last element
    T pop()
    { return this->remove(this->size - 1); }


    //Remove all elements
    void clear() 
    { this->size = 0; }


    //Swap elements array[a] <--> array[b]
    void swap(unsigned int a, unsigned int b) {
        if(a >= this->size || b >= this->size)
            throw std::out_of_range("vector index out of range");   

        if(a == b)
            return;

        T temp = this->array[a];
        this->array[a] = this->array[b];
        this->array[b] = temp;
    }
};


//Vector split in half based on a binary value
//We assume insert won't be used
/*template <typename T>
class hvector : public vector<T>
{
    unsigned int split_index{ 0 };

public:
    void push(T data, bool flag) { 
        vector<T>::insert(data, this->size);
        if(flag)
            swap(this->size - 1, split_index++);
    }


    //By default, push inserts with true
    void push(T data)
    { this->push(data, true); }

    //Returns the first non-true index
    const unsigned int split_point()
    { return split_index; }


    //Returns flag of index
    const bool flag(unsigned int index) {
        if(index >= this->size)
            throw std::out_of_range("vector index out of range");
        return index < split_index ? true : false;
    }


    //Changes flag of index
    void reverse_flag(unsigned int index) {
        if(index >= this->size)
            throw std::out_of_range("vector index out of range");
        
        if(index < split_index) //T -> F
            swap(--split_index, index);
        else 
            swap(split_index++, index);
    }
};*/


#endif