#include "buffer.h"
#include <linux/slab.h>
// function that empties list.
void empty_list ( list * lst )
{
    node * element = lst-> front->next ; // list is first value, that is next of the front node
    if (lst ->size==0) {
        return;
    }
    while ( element != lst -> rear ) { // while list is not rear node
        node * next = element -> next ; // remember next node before freeing
        kfree ( element -> key ) ;    // keys are kmalloc'd by the driver
        kfree ( element ) ;           // free the node
        element = next ;              // go to the next node
    }
    lst -> front -> next = lst -> rear ;
    lst -> rear -> prev = lst -> front ;
    lst -> size = 0 ;
}
void free_list (list * lst)   // freeing the list
{
    if ( lst -> front == NULL ) {
        return;
    }
    empty_list ( lst ) ;     // first, clear the list
    kfree ( lst -> front ) ; // free the list -> front
    kfree ( lst -> rear ) ; // free the list -> rear
}
MODULE_LICENSE("GPL");
