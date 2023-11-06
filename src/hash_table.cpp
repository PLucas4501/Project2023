#include "hash_table.h"
#include "point.h"
#include "distance.h"


class Hash_Table{
    private:
        static const int tablesize = 2   ///auto tha einai to K/2 tha to doume borei na to pairnei san timh kateutheian
        struct hash_node{
            struct point A;
            struct point B;
            double dist;
            hash_node* next;

            hash_node(struct point a,struct point b , double dist) : A(a) , B(b), dist(dist), next(nullptr) {} 
        };

        hash_node* table[tablesize];

        int hash_function(const struct point& a , const struct point& b ){
            return (a.cord[0] *10 + b.cord[0]) % tablesize ;
        }

    public:
        void insert(double dist , struct point a , struct point b){
            int index = hash_function(a , b);
            hash_node* new_node = new hash_node(a,b,dist);

            if(table[index] == nullptr )
                table[index] = new_node ;
            else{
                hash_node* current = table[index];
                while(current->next  != nullptr){
                    current = current -> next;
                }   
                current -> next = new_node;
            }
        }

        double Retrieve_dist(const struct point& a , const struct point& b){
            int index = hash_function(a);
            hash_node* current = table[index];
            while (current != nullptr) {
                if (current->a.cord[0] == a.cord[0] && current->a.cord[1]== a.cord[1]) 
                    return current->distance;
            }
        }

}

