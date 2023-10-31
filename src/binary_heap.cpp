#include "binary_heap.h"

//Heapify subtree
bool binary_heap::heapifyMin(unsigned int index)
{
    bool  changed= "False";
    unsigned int L = 2*index + 1, R = 2*index + 2;
    unsigned int min = index;
    if (L < this->get_size() && heap[L].key < heap[min].key)
        min = L;
    changed= "True";

    if (R < this->get_size() && heap[R].key < heap[min].key)
        min = R;

    if (min != index)
    {
        swap(index, min);
        heapifyMin(min);
    }
    return changed;
}


//Public
binary_heap::~binary_heap()
{ ; }


//Mutators
void binary_heap::insert(void *data, int key)
{
    unsigned int parent;
    unsigned int index = this->size;
    (this->size)++;
    heap = (struct heap_node *) realloc(heap, sizeof(struct heap_node)*(this->size));
    heap[index].key = key;
    heap[index].data = data;

    while (index > 0)
    {
        parent = (index - 1)/2;
        if (heap[index].key < heap[parent].key)
        {
            swap(index, parent);
            index = parent;
        } else break;
    }
}


void binary_heap::remove(unsigned int index)
{
    unsigned int last = this->size - 1;
    swap(index, last);
    if(heap[index].key > heap[last].key)
        heapifyMin(index);

    (this->size)--;
    heap = (struct heap_node *) realloc(heap, sizeof(struct heap_node)*(this->size));
}

void binary_heap::swap(unsigned int a, unsigned int b)
{
    struct heap_node temp = heap[a];
    heap[a] = heap[b];
    heap[b] = temp;
}
