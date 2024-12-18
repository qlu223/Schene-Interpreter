#include "value.h"
#include "talloc.h"
Value *hd;
// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size){
    void *p = malloc(size); //malloc blox of mem
    Value *cons = malloc(sizeof(Value));
    Value *ptr = malloc(sizeof(Value));
    cons->type = CONS_TYPE;
    ptr->type = PTR_TYPE;
    ptr->p = p;
    if(hd == NULL){
        Value *n = malloc(sizeof(Value));
        n->type = NULL_TYPE;
        hd = n;
    }
    cons->c.car = ptr;
    cons->c.cdr = hd;
    hd = cons;
    return p;
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree(){
    if(hd == NULL){
        return;
    }
    Value *cur = hd;
    while (cur->type == CONS_TYPE){
        Value *next = cur->c.cdr;
        free(cur->c.car->p);
        free(cur->c.car);
        free(cur);
        cur = next;
    }
    free(cur);
    hd= NULL;
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status){
    tfree();
    exit(status);
}