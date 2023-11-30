#ifndef AVL_H
#define AVL_H

#include <iostream>
#include <stdexcept>
#include "vector.h"

//A templated AVL tree data structure with basic operations.
//Supports functionality for one or two template arguments,
//and the code duality is managed with preprocessor directives.
//Templated arguments must support comparison operators (>, <)


//Improvements: merge functionalities

template <typename K>
class AVL {
    ///---Attributes---///
    //AVL node prototype
    struct node {
        K key;
        struct node *L{ nullptr };
        struct node *R{ nullptr };
        struct node *parent{ nullptr };
        short int balance{ 0 };
    
        node(K key): key(key) {}
    };

    struct node *root{ nullptr };
    unsigned int cap{ 0 }, size{ 0 };


    ///---Internal functions---///
    //Returns a vector of all nodes inorder under tgt
    void all_nodes(struct node *tgt, vector<struct node *> &v) {
        if(tgt == nullptr)
            return;

        all_nodes(tgt->L, v);
        v.push(tgt);
        all_nodes(tgt->R, v);       
    }

    //Traverses tree for steps < size
    void traverse(struct node *tgt, unsigned int &steps, K &key) {
        if(tgt->L && steps > 0) {
            steps -= 1;
            traverse(tgt->L, steps, key);
        }

        if(steps == 0)
            key = tgt->key;

        if(tgt->R && steps > 0){ 
            steps -= 1;
            traverse(tgt->R, steps, key);
        }
    }

    //Internal node comparison method
    short int compare(struct node *A, struct node *B) {
        if(A->key < B->key)
            return 1;
        else if(A->key > B->key)
            return -1;
        else return 0;
    }

    //Left rotate
    void rotateL(struct node *tgt) {
        struct node *new_tgt = tgt->R;
        tgt->R = new_tgt->L;
        if(tgt->R) 
            tgt->R->parent = tgt;

        new_tgt->parent = tgt->parent;
        tgt->parent = new_tgt;
        new_tgt->L = tgt;

        //Adjust parent
        if(new_tgt->parent == nullptr)
            this->root = new_tgt;
        else if(tgt == new_tgt->parent->L)
            new_tgt->parent->L = new_tgt;
        else new_tgt->parent->R = new_tgt;
    }

    //Right rotate
    void rotateR(struct node *tgt) {
        struct node *new_tgt = tgt->L;
        tgt->L = new_tgt->R;
        if(tgt->L) 
            tgt->L->parent = tgt;

        new_tgt->parent = tgt->parent;
        tgt->parent = new_tgt;
        new_tgt->R = tgt;

        //Adjust parent
        if(new_tgt->parent == nullptr)
            this->root = new_tgt;
        else if(tgt == new_tgt->parent->L)
            new_tgt->parent->L = new_tgt;
        else new_tgt->parent->R = new_tgt;
    }

    //Delete nodes under tgt recursively
    void clear(struct node *tgt) {
        //Delete children
        if(tgt->L)
            clear(tgt->L);
        if(tgt->R)
            clear(tgt->R);
        
        //Delete self
        if(tgt == this->root) {
            this->size = 0;
            delete this->root;
            this->root = nullptr;
        } else delete tgt;
    }

    //Finds and returns given node
    struct node *find(struct node *tgt) {
        short int result;
        struct node *found = this->root;
        while((result = this->compare(found, tgt)) != 0) {
            found = (result == 1) ? found->R : found->L;
            if(!found)
                return nullptr;
        } return found;
    }

    //Retrieves greatest node
    struct node *max_node() {
        if(this->size == 0)
            return nullptr;

        struct node *tgt = this->root;
        while(tgt->R)
            tgt = tgt->R;
        return tgt;
    }

    //Retrieves smallest node
    struct node *min_node() {
        if(this->size == 0)
            return nullptr;
        
        struct node *tgt = this->root;
        while(tgt->L)
            tgt = tgt->L;
        return tgt;
    }

    //Internal insertion method, returns true on successful insertion
    bool insert(struct node *tgt) {
        //Adjust parent
        if(tgt->parent) {
            if(compare(tgt->parent, tgt) == 1)
                tgt->parent->R = tgt;
            else tgt->parent->L = tgt;
        } else this->root = tgt;
        

        //Retrace and rebalance tree
        bool double_rotate = false;
        while(tgt->parent) {
            if(tgt == tgt->parent->R)
                tgt->parent->balance += 1;
            else tgt->parent->balance -= 1;

            //Perfectly balanced...
            if(tgt->parent->balance == 0)
                break;

            //Left-heavy
            if(tgt->parent->balance < -1) {
                if(tgt->balance == -1) {
                    //Right rotate
                    rotateR(tgt->parent);
                    tgt->R->balance = tgt->balance = 0;
                    break;
                } else {
                    //Right-left rotate
                    rotateL(tgt);
                    rotateR(tgt->parent->parent);
                    double_rotate = true; 
                }
            }

            //Right-heavy
            if(tgt->parent->balance > 1) {
                if(tgt->balance == 1) {
                    //Rotate & adjust balance factors
                    rotateL(tgt->parent);
                    tgt->L->balance = tgt->balance = 0;
                    break;
                } else {
                    //Left-right rotate
                    rotateR(tgt);
                    rotateL(tgt->parent->parent);
                    double_rotate = true;
                }
            }


            //Adjust balance for double rotates
            if(double_rotate) {
                tgt = tgt->parent;
                double_rotate = false;
                switch(tgt->balance) {
                    case 0:
                        tgt->L->balance = tgt->R->balance = 0;
                        break;
                    case -1:
                        tgt->L->balance = 0;
                        tgt->R->balance = 1;
                        break;
                    case 1:
                        tgt->L->balance = -1;
                        tgt->R->balance = 0;
                        break;
                    default:
                        break;
                } tgt->balance = 0;
                break;
            }

            tgt = tgt->parent;
        } this->size += 1;
        return true;
    }

    //Internal deletion method, returns true on successful deletion
    bool remove(struct node *tgt) {
        if(!tgt)
            return false;

        //Consider the cases...
        bool adjust = false;
        struct node *del = tgt;
        //At most one child
        if(!(tgt->L && tgt->R)) {
            //One child
            if((del = tgt->L ? tgt->L : tgt->R)) {
                tgt->key = del->key;
            }
            
            //Root with no children
            else if(this->size == 1) { 
                this->size = 0;
                delete this->root;
                this->root = nullptr;
                return true;
            } else del = tgt; //No children
        }
        
        //Two children
        else {
            //Find next greater element
            del = tgt->R;
            while(del->L) 
                del = del->L;

            //Copy data
            tgt->key = del->key;

            //Need to readjust child, if it exists
            if(del->R) 
                adjust = true;
        }


        //Trace back and rebalance tree
        struct node* ret = del;
        bool double_rotate = false;
        while(ret->parent) {
            if(ret == ret->parent->R)
                ret->parent->balance -= 1;
            else ret->parent->balance += 1;

            //Adjust tree after using deletion node
            //ret will remain as a ghost node for this iteration
            if(ret == del) {
                if(adjust) {
                    del->R->parent = del->parent;
                    if(del->parent == tgt)
                        tgt->R = del->R;
                    else del->parent->L = del->R;     
                }

                //Normal case, nullify child
                else if(del == del->parent->L)
                    del->parent->L = nullptr;
                else del->parent->R = nullptr;             
            }

            //Perfectly balanced...
            ret = ret->parent;
            if(ret->balance == 1 || ret->balance == -1)
                break;

            //Left-heavy
            if(ret->balance < -1) {
                if(ret->L->balance < 1) {
                    rotateR(ret);
                    ret = ret->parent;
                    if(ret->balance == 0) {
                        ret->balance = 1;
                        ret->R->balance = -1;
                        break;
                    } else ret->R->balance = ret->balance = 0;
                } else {
                    rotateL(ret->L);
                    rotateR(ret);
                    double_rotate = true;
                }
            }

            //Right-heavy
            else if(ret->balance > 1) {
                if(ret->R->balance > -1) {
                    rotateL(ret);
                    ret = ret->parent;
                    if(ret->balance == 0) {
                        ret->balance = -1;
                        ret->L->balance = 1;
                        break;
                    } else ret->L->balance = ret->balance = 0;
                } else {
                    rotateR(ret->R);
                    rotateL(ret);
                    double_rotate = true;
                }
            }

            //Adjust balance after double rotates
            if(double_rotate) {
                ret = ret->parent;
                double_rotate = false; 
                if(ret->balance == -1) {
                    ret->L->balance = 0;
                    ret->R->balance = 1;
                } 
                
                else if (ret->balance == 1) {
                    ret->L->balance = -1;
                    ret->R->balance = 0;
                }

                else {
                    ret->L->balance = 0;
                    ret->R->balance = 0;              
                } ret->balance = 0;
            }
        } delete del;
        this->size -= 1;
        return true;
    }


///---Public Interface---///
public:
    ///---Constructors & Destructors---///
    //Cap default is 0 (infinite size)
    AVL(unsigned int cap = 0): cap(cap) {}

    ~AVL() { this->clear(); }


    ///---Printers---///
    //Prints tree with balance factors
    void print() {
        vector<struct node *> pv;
        this->all_nodes(this->root, pv);

        std::cout << "[";
        if(!pv.is_empty())
            std::cout << pv[0]->key;
        
        for(unsigned int i=1; i < pv.get_size(); i++)
            std::cout << ", " << pv[i]->key;
        std::cout << "]" << std::endl;
    }

    K operator[](unsigned int index) { 
        if(index >= this->get_size())
            throw std::out_of_range("AVL index out of range");
        
        K key;
        traverse(this->root, index, key);
        return key;
    }

    ///---Accessors---///
    K const min_key() 
    { return this->min_node()->key; }

    K const max_key()
    { return this->max_node()->key; }

    unsigned int const get_size()
    { return this->size; }

    unsigned int const get_cap()
    { return this->cap; }

    bool const is_empty()
    { return this->get_size() == 0; }

    bool const find(K key) {
        struct node *tgt = new node(key);
        struct node *ret = this->find(tgt);
        delete tgt;
        return ret != nullptr;
    }

    ///---Mutators---///
    //Inserts key (and data if eligible) into the AVL
    //Insetion occurs only if key (or data) is unique and cap not reached
    bool insert(K key) {
        if(cap > 0 && size == cap)
            return false;

        //Find suitable place
        struct node *tgt = this->root;
        struct node *new_node = new struct node(key); 
        while(tgt) {
            new_node->parent = tgt;
            switch(compare(tgt, new_node)) {
                case 1:
                    tgt = tgt->R;
                    break;
                case -1:
                    tgt = tgt->L;
                    break;
                default: //Duplicate
                    delete new_node;
                    return false;
            }
        } return this->insert(new_node); //Internal insert does the rest
    }

    //Removes node with given key (& data), if found
    bool remove(K key) {
        struct node *tgt = new struct node(key);
        struct node *del = this->find(tgt);
        delete tgt;
        return this->remove(del);
    }

    bool remove_max()
    { return this->remove(this->max_node()); }

    bool remove_min()
    { return this->remove(this->min_node()); }

    void set_cap(unsigned int new_cap)
    { this->cap = new_cap; }

    //Erases the tree
    void clear() { 
        if(this->size > 0) 
            this->clear(this->root); 
    }
};


///------------------------------------------------------///
///------------------------------------------------------///
//Keeps minimum key-data pairs
template <typename K, typename D>
class minAVL {
    ///---Attributes---///
    //AVL node prototype
    struct node {
        K key;
        D data;
        struct node *L{ nullptr };
        struct node *R{ nullptr };
        struct node *parent{ nullptr };
        short int balance{ 0 };
    
        node(K key, D data): key(key), data(data) {}
    };

    struct node *root{ nullptr };
    unsigned int cap{ 0 }, size{ 0 };


    ///---Internal functions---///
    //Returns a vector of all nodes inorder under tgt
    void all_nodes(struct node *tgt, vector<struct node *> &v) {
        if(tgt == nullptr)
            return;

        all_nodes(tgt->L, v);
        v.push(tgt);
        all_nodes(tgt->R, v);       
    }

    void traverse(struct node *tgt, unsigned int &steps, D &data) {
        if(tgt->L && steps > 0) {
            steps -= 1;
            traverse(tgt->L, steps, data);
        }

        if(steps == 0)
            data = tgt->data;

        if(tgt->R && steps > 0){ 
            steps -= 1;
            traverse(tgt->R, steps, data);
        }
    }

    //Internal node comparison method
    short int compare(struct node *A, struct node *B) {
        if(A->key < B->key)
            return 1;
        else if(A->key > B->key)
            return -1;
        else return 0;
    }

    //Internal node comparison method
    short int data_compare(struct node *A, struct node *B) {
        if(A->data < B->data)
            return 1;
        else if(A->data > B->data)
            return -1;
        else return 0;
    }

    //Left rotate
    void rotateL(struct node *tgt) {
        struct node *new_tgt = tgt->R;
        tgt->R = new_tgt->L;
        if(tgt->R) 
            tgt->R->parent = tgt;

        new_tgt->parent = tgt->parent;
        tgt->parent = new_tgt;
        new_tgt->L = tgt;

        //Adjust parent
        if(new_tgt->parent == nullptr)
            this->root = new_tgt;
        else if(tgt == new_tgt->parent->L)
            new_tgt->parent->L = new_tgt;
        else new_tgt->parent->R = new_tgt;
    }

    //Right rotate
    void rotateR(struct node *tgt) {
        struct node *new_tgt = tgt->L;
        tgt->L = new_tgt->R;
        if(tgt->L) 
            tgt->L->parent = tgt;

        new_tgt->parent = tgt->parent;
        tgt->parent = new_tgt;
        new_tgt->R = tgt;

        //Adjust parent
        if(new_tgt->parent == nullptr)
            this->root = new_tgt;
        else if(tgt == new_tgt->parent->L)
            new_tgt->parent->L = new_tgt;
        else new_tgt->parent->R = new_tgt;
    }

    //Delete nodes under tgt recursively
    void clear(struct node *tgt) {
        //Delete children
        if(tgt->L)
            clear(tgt->L);
        if(tgt->R)
            clear(tgt->R);
        
        //Delete self
        if(tgt == this->root) {
            this->size = 0;
            delete this->root;
            this->root = nullptr;
        } else delete tgt;
    }

    //Finds and returns given node
    struct node *find(struct node *tgt) {
        if(this->size == 0)
            return nullptr;

        short int result;
        struct node *found = this->root;
        while((result = this->compare(found, tgt)) != 0) {
            found = (result == 1) ? found->R : found->L;
            if(!found)
                return nullptr;
        } return found;
    }

    //Retrieves greatest node
    struct node *max_node() {
        if(this->size == 0)
            return nullptr;

        struct node *tgt = this->root;
        while(tgt->R)
            tgt = tgt->R;
        return tgt;
    }

    //Retrieves smallest node
    struct node *min_node() {
        if(this->size == 0)
            return nullptr;
        
        struct node *tgt = this->root;
        while(tgt->L)
            tgt = tgt->L;
        return tgt;
    }

    //Internal insertion method, returns true on successful insertion
    bool insert(struct node *tgt) {
        //Adjust parent
        if(tgt->parent) {
            if(compare(tgt->parent, tgt) == 1)
                tgt->parent->R = tgt;
            else tgt->parent->L = tgt;
        } else this->root = tgt;
        

        //Retrace and rebalance tree
        bool double_rotate = false;
        while(tgt->parent) {
            if(tgt == tgt->parent->R)
                tgt->parent->balance += 1;
            else tgt->parent->balance -= 1;

            //Perfectly balanced...
            if(tgt->parent->balance == 0)
                break;

            //Left-heavy
            if(tgt->parent->balance < -1) {
                if(tgt->balance == -1) {
                    //Right rotate
                    rotateR(tgt->parent);
                    tgt->R->balance = tgt->balance = 0;
                    break;
                } else {
                    //Right-left rotate
                    rotateL(tgt);
                    rotateR(tgt->parent->parent);
                    double_rotate = true; 
                }
            }

            //Right-heavy
            if(tgt->parent->balance > 1) {
                if(tgt->balance == 1) {
                    //Rotate & adjust balance factors
                    rotateL(tgt->parent);
                    tgt->L->balance = tgt->balance = 0;
                    break;
                } else {
                    //Left-right rotate
                    rotateR(tgt);
                    rotateL(tgt->parent->parent);
                    double_rotate = true;
                }
            }


            //Adjust balance for double rotates
            if(double_rotate) {
                tgt = tgt->parent;
                double_rotate = false;
                switch(tgt->balance) {
                    case 0:
                        tgt->L->balance = tgt->R->balance = 0;
                        break;
                    case -1:
                        tgt->L->balance = 0;
                        tgt->R->balance = 1;
                        break;
                    case 1:
                        tgt->L->balance = -1;
                        tgt->R->balance = 0;
                        break;
                    default:
                        break;
                } tgt->balance = 0;
                break;
            }

            tgt = tgt->parent;
        } this->size += 1;
        return true;
    }

    //Internal deletion method, returns true on successful deletion
    bool remove(struct node *tgt) {
        if(!tgt)
            return false;

        //Consider the cases...
        bool adjust = false;
        struct node *del = tgt;
        //At most one child
        if(!(tgt->L && tgt->R)) {
            //One child
            if((del = tgt->L ? tgt->L : tgt->R)) {
                tgt->key = del->key;
                tgt->data = del->data;
            }
            
            //Root with no children
            else if(this->size == 1) { 
                this->size = 0;
                delete this->root;
                this->root = nullptr;
                return true;
            } else del = tgt; //No children
        }
        
        //Two children
        else {
            //Find next greater element
            del = tgt->R;
            while(del->L) 
                del = del->L;

            //Copy data
            tgt->key = del->key;
            tgt->data = del->data;

            //Need to readjust child, if it exists
            if(del->R) 
                adjust = true;
        }


        //Trace back and rebalance tree
        struct node* ret = del;
        bool double_rotate = false;
        while(ret->parent) {
            if(ret == ret->parent->R)
                ret->parent->balance -= 1;
            else ret->parent->balance += 1;

            //Adjust tree after using deletion node
            //ret will remain as a ghost node for this iteration
            if(ret == del) {
                if(adjust) {
                    del->R->parent = del->parent;
                    if(del->parent == tgt)
                        tgt->R = del->R;
                    else del->parent->L = del->R;     
                }

                //Normal case, nullify child
                else if(del == del->parent->L)
                    del->parent->L = nullptr;
                else del->parent->R = nullptr;             
            }

            //Perfectly balanced...
            ret = ret->parent;
            if(ret->balance == 1 || ret->balance == -1)
                break;

            //Left-heavy
            if(ret->balance < -1) {
                if(ret->L->balance < 1) {
                    rotateR(ret);
                    ret = ret->parent;
                    if(ret->balance == 0) {
                        ret->balance = 1;
                        ret->R->balance = -1;
                        break;
                    } else ret->R->balance = ret->balance = 0;
                } else {
                    rotateL(ret->L);
                    rotateR(ret);
                    double_rotate = true;
                }
            }

            //Right-heavy
            else if(ret->balance > 1) {
                if(ret->R->balance > -1) {
                    rotateL(ret);
                    ret = ret->parent;
                    if(ret->balance == 0) {
                        ret->balance = -1;
                        ret->L->balance = 1;
                        break;
                    } else ret->L->balance = ret->balance = 0;
                } else {
                    rotateR(ret->R);
                    rotateL(ret);
                    double_rotate = true;
                }
            }

            //Adjust balance after double rotates
            if(double_rotate) {
                ret = ret->parent;
                double_rotate = false; 
                if(ret->balance == -1) {
                    ret->L->balance = 0;
                    ret->R->balance = 1;
                } 
                
                else if (ret->balance == 1) {
                    ret->L->balance = -1;
                    ret->R->balance = 0;
                }

                else {
                    ret->L->balance = 0;
                    ret->R->balance = 0;              
                } ret->balance = 0;
            }
        } delete del;
        this->size -= 1;
        return true;
    }

public:
    minAVL(unsigned int cap = 0): cap(cap) {}

    ~minAVL() { this->clear(); }

    ///---Public Interface---///
    void print() {
        vector<struct node *> pv;
        all_nodes(this->root, pv);

        std::cout << "[";
        if(!pv.is_empty())
            std::cout << pv[0]->data;
        
        for(unsigned int i=1; i < pv.get_size(); i++)
            std::cout << ", " << pv[i]->data;
        std::cout << "]" << std::endl;
    }

    D operator[](unsigned int index) { 
        if(index >= this->get_size())
            throw std::out_of_range("AVL index out of range");
        
        D data;
        traverse(this->root, index, data);
        return data;
    }

    ///---Accessors---///
    unsigned int const get_size()
    { return this->size; }

    unsigned int const get_cap()
    { return this->cap; }

    bool const is_empty()
    { return this->get_size() == 0; }

    bool const find(K key) {
        struct node *tgt = new node(key, (D) 0);
        struct node *ret = this->find(tgt);
        delete tgt;
        return ret != nullptr;
    }

    K const min_key() 
    { return this->min_node()->key; }

    K const max_key()
    { return this->max_node()->key; }

    D const min() 
    { return this->min_node()->data; }

    D const max()
    { return this->max_node()->data; }
    
    bool insert(K key, D data) {
        //Max size, see if replacement is possible
        struct node *tgt;
        struct node *new_node = new struct node(key, data);
        if((tgt = this->find(new_node))) {
            if(data_compare(tgt, new_node) > -1) {
                delete new_node;
                return false;
            } else this->remove(tgt);
        } else if(cap > 0 && size == cap) {
            if(compare(max_node(), new_node) < 1) {
                delete new_node;
                return false;
            } else remove_max();
        }
    
        tgt = this->root;
        new_node->parent = nullptr;
        while(tgt) {
            new_node->parent = tgt;
            switch(compare(tgt, new_node)) {
                case 1:
                    tgt = tgt->R;
                    break;
                case -1:
                    tgt = tgt->L;
                    break;
                default: //Duplicate
                    delete new_node;
                    return false;
            }
        } return this->insert(new_node);
    }

    //Removes node with given key (& data), if found
    bool remove(K key) {
        struct node *tgt = new struct node(key);
        struct node *del = this->find(tgt);
        delete tgt;
        return this->remove(del);
    }

    bool remove_max()
    { return this->remove(this->max_node()); }

    bool remove_min()
    { return this->remove(this->min_node()); }

    void set_cap(unsigned int new_cap)
    { this->cap = new_cap; }

    //Erases the tree
    void clear() { 
        if(this->size > 0) 
            this->clear(this->root); 
    }
};

#endif