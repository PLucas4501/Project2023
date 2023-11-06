#include "AVL.h"

AVL::~AVL()
{ this->clear(); }

void AVL::rotateL(struct node *tgt)     
{
    struct node *parent = tgt->parent;
    struct node *old = tgt;
    tgt = old->R;
    tgt->parent = old->parent;
    old->parent = tgt;

    old->R = tgt->L;
    if(old->R != nullptr)
        old->R->parent = old;

    tgt->L = old;

    if(old->balance == 0) {
        tgt->balance -= 1;
        tgt->L->balance += 1;
    } else tgt->balance = tgt->L->balance = 0;

    if(parent == nullptr)
        this->root = tgt;
    else if(tgt->key < parent->key)
        parent->L = tgt;
    else parent->R = tgt;
}

void AVL::rotateR(struct node *tgt) 
{
    struct node *parent = tgt->parent;
    struct node *old = tgt;
    tgt = old->L;
    tgt->parent = parent;
    old->parent = tgt;

    old->L = tgt->R;
    if(old->L != nullptr)
        old->L->parent = old;
    tgt->R = old;

    if(old->balance == 0) {
        tgt->balance -= 1;
        tgt->R->balance += 1;
    } else tgt->balance = tgt->R->balance = 0;

    if(parent == nullptr) //root
        this->root = tgt;
    else if(tgt->key < parent->key)
        parent->L = tgt;
    else parent->R = tgt;
}


//Delete all nodes
void AVL::clear(struct node *node)
{
;
}

void AVL::all_elements(struct node *tgt, vector<unsigned int> &v)
{
    if(tgt == nullptr)
        return;

    v.push(tgt->key);
    all_elements(tgt->L, v);
    all_elements(tgt->R, v);
}

//------------Public interface------------//
vector<unsigned int> AVL::all_elements() 
{
    vector<unsigned int> elements;
    all_elements(this->root, elements);
    return elements;
}

float AVL::find(unsigned int key) 
{
    struct node *tgt = this->root;
    if(tgt == nullptr)
        return 0;

    while(tgt->key != key) {
        if(key < tgt->key)
            tgt = tgt->L;
        else tgt = tgt->R;

        if(tgt == nullptr)
            return 0;
    }

    return tgt->data;
}

void AVL::print_tree()
{
    if(this->size == 0) {
        std::cout << "[]" << std::endl;
        return;
    }

    vector<unsigned int> v = this->all_elements();
    std::cout << "[" << v[0];
    for(unsigned int i=1; i < v.get_size(); i++)
        std::cout << ", " << v[i];
    std::cout << "]" << std::endl;
}

//Insertion will happen only if key is unique. Returns false otherwise.
bool AVL::insert(float data, unsigned int key)
{
    struct node *parent = nullptr;
    struct node *tgt = this->root;
    while(tgt != nullptr) {
        parent = tgt;
        if(key < tgt->key)
            tgt = tgt->L;

        else if(key > tgt->key)
            tgt = tgt->R;

        //Duplicates not allowed
        else return false;
    }

    //Reached leaf
    if((tgt = new struct node(data, key, parent)) == nullptr)
        throw std::runtime_error("memory allocation failure");
    
    if(tgt->parent == nullptr)
        this->root = tgt;
    else {
        if(key < parent->key)
            tgt->parent->L = tgt;
        else tgt->parent->R = tgt;
    }

    //Retrace and rebalance tree
    while(parent != nullptr) {
        if(tgt == parent->L)
            parent->balance -= 1;
        else parent->balance += 1;

        //Perfectly balanced...
        if(parent->balance == 0)
            break;

        //Left-heavy
        if(parent->balance < -1) {
            if(parent->L->balance == -1) //RR rotation
                rotateR(parent);
            else { //LR rotation
                rotateL(parent->L);
                rotateR(parent);
            } break;
        }

        //Right-heavy
        if(parent->balance > 1) {
            if(parent->R->balance == 1) //LL rotation
                rotateL(parent);
            else { //RL rotation
                rotateR(parent->R);
                rotateL(parent);
            } break;
        }

        tgt = parent;
        parent = tgt->parent;
    } this->size += 1;

    return true;
}

bool AVL::insert(unsigned int key)
{ return this->insert(0, key); }

//Remove node with given key
bool AVL::remove(unsigned int key)
{
    struct node *del;
    struct node *parent;
    struct node *tgt = this->root;
    this->print_tree();
    std::cout << "wants to remove " << key << ": ";

    if(tgt == nullptr)
        return false;

    while(key != tgt->key) {
        if(key < tgt->key)
            tgt = tgt->L;
        else tgt = tgt->R;

        //Not found
        if(tgt == nullptr) 
            return false;
    }

    //Consider the cases...
    //No children
    if(tgt->L == nullptr && tgt->R == nullptr) {
        if(tgt->parent == nullptr) { //Delete root
            delete this->root;
            this->root = nullptr;
            return true;
        } del = tgt;
    } 
    
    //One child
    else if(tgt->L == nullptr || tgt->R == nullptr) {
        del = tgt->L ? tgt->L : tgt->R;

        //Copy data
        tgt->key = del->key;
        tgt->data = del->data;
        tgt = del;
    }
    
    //Two children
    else {
        del = tgt->R;

        //Find min
        while(del->L) 
            del = del->L;
        
        //Copy min data
        tgt->data = del->data;
        tgt->key = del->key;

        //Adjust tree properly, and prepare for rebalancing
        if(del == tgt->R) {
            if(del->R)
                del->R->parent = tgt;
            tgt->R = del->R;
            tgt = del;
        } else {
            if(del->R)
                del->R->parent = del->parent;
            del->parent->L = del->R;
            tgt = del->parent->L;
        }
    }

    //Retrace and rebalance tree before deletion
    parent = tgt->parent;
    while(parent != nullptr) {
        if(tgt == parent->L)
            parent->balance += 1;
        else parent->balance -= 1;

        //Perfectly balanced...
        if(parent->balance == 1 || parent->balance == -1)
            break;

        //Left-heavy
        if(parent->balance < -1) {
            if(parent->L->balance == -1) //RR rotation
                rotateR(parent);
            else { //LR rotation
                rotateL(parent->L);
                rotateR(parent);
            } break;
        }

        //Right-heavy
        if(parent->balance > 1) {
            if(parent->R->balance == 1) //LL rotation
                rotateL(parent);
            else { //RL rotation
                rotateR(parent->R);
                rotateL(parent);
            } break;
        }

        tgt = parent;
        parent = tgt->parent;
    }


    //Nullify child & delete
    std::cout << "about to delete" << std::endl;
    if(del->parent == nullptr)
        delete del;
    else if(del == del->parent->L) {
        delete del->parent->L;
        del->parent->L = nullptr;
    }

    else {
        delete del->parent->R;
        del->parent->R = nullptr;
    }

    this->size -= 1;
    this->print_tree();

    return true;
}

//Removes node with old_key and inserts new tih new_key
bool AVL::replace(unsigned int old_key, unsigned int new_key)
{
    if(old_key == new_key)
        return false;

    if(this->remove(old_key))
        return this->insert(new_key);
    else return false;
}

void AVL::clear()
{ this->clear(this->root); this->size = 0; }