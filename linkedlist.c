#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "linkedlist.h"
#include "value.h"
#include <string.h>
#include "talloc.h"

// Create a new NULL_TYPE value node.
Value *makeNull(){
    Value *null_node = (Value *)talloc(sizeof(Value));
    null_node->type = NULL_TYPE;

    return null_node;
}


// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr){
    Value *cons_node = (Value *)talloc(sizeof(Value));
    cons_node->type = CONS_TYPE;

    cons_node->c.car = newCar;
    cons_node->c.cdr = newCdr;
    return cons_node;
    

}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list){
    // Value *cur = list;
    switch (list->type){
        case INT_TYPE:
            printf("%i\n", list->i);
            break;
        case DOUBLE_TYPE:
            printf("%.2lf\n", list->d);
            break;
        case BOOL_TYPE:
            printf("%s", list->s);
            break;
        case STR_TYPE:
            printf("%s\n",list->s);
            break;
        case OPEN_TYPE:
            printf("(");
            break;
        case CLOSE_TYPE:
            printf(")");
            break;
        case NULL_TYPE:
            printf("\n");
            break;
        case CONS_TYPE:
            display(list->c.car);
            display(list->c.cdr);
            break; 
        case PTR_TYPE:
            break;       
        default:
            break;
    }
}

// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; there should be no shared
// memory whatsoever between the original list and the new one.
//
// FAQ: What if there are nested lists inside that list?
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.


Value *reverse(Value *list){
    if(list->type == NULL_TYPE){
        return makeNull();
    }
    Value *cur =list;
    Value *prev = makeNull();
    while(cur->type == CONS_TYPE){
        Value *new_value = cur->c.car;
        prev = cons(new_value,prev);
        cur = cur->c.cdr;
    }
    return prev;
}


// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list){
    assert(list->type == CONS_TYPE);

    return list->c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list){
    assert(list->type == CONS_TYPE);

    return list->c.cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value){
    // assert(value->type == NULL_TYPE);
    if(value->type == NULL_TYPE){
        return 1;
    } else{
        return 0;
    }
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value){
    assert(value != NULL);
    int c = 0;
    Value* cur = value;

    while(cur->type == CONS_TYPE){
        c++;
        cur = cur->c.cdr;
    }
    return c;
}


