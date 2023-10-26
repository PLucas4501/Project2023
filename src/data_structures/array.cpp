#include <array.h>

class Array
{
    unsigned int size;
    void **array;

    public:
        Array(unsined int size)
        {
            self.size = size;
            array = new void *[size];
        }

        Array operator[](unsigned int index)
        {
            return *array[index];
        }
}