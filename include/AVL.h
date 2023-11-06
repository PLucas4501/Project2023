#ifndef AVL_H
#define AVL_H

//A small AVL tree implementation for the problem

#include <iostream>
#include <stdexcept>
#include "vector.h"

class AVL
{
    struct node
    {
        double data;
        unsigned int key;
        struct node *parent;
        struct node *L;
        struct node *R;
        signed char balance;

        node(double data, unsigned int key, struct node *parent): 
            data(data), key(key), parent(parent), L(nullptr), R(nullptr), balance(0) {}
    };

    struct node *root{ nullptr };
    unsigned int size{ 0 };

    void rotateL(struct node *);
    void rotateR(struct node *);
    void all_elements(struct node *, vector<unsigned int>&);
    void clear(struct node *);

public:
    ~AVL();

    unsigned int const get_size()
    { return this->size; }

    vector<unsigned int> all_elements();
    void print_tree();
    bool insert(double, unsigned int);
    bool insert(unsigned int);
    bool remove(unsigned int);
    bool replace(unsigned int, unsigned int);
    double find(unsigned int);
    void clear();
};

#endif