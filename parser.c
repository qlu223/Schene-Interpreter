#include <assert.h>
#include <stdlib.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"
#include <stdio.h>
#include "parser.h"

Value *addToParseTree(Value *tree, int *depth, Value *token){
    if(token->type == OPEN_TYPE){
        (*depth)++;
    }
    return cons(token,tree); 
}

// Takes a list of tokens from a Scheme program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens) {
    Value *tree = makeNull();
    int depth = 0;
    Value *current = tokens;
    assert(current != NULL && "Error (parse): null pointer");
    while (current->type != NULL_TYPE) {
        //get next token
        Value *token = current->c.car;
        //push onto tree if its anything other than close paren
        if(token->type !=CLOSE_TYPE){
            tree = addToParseTree(tree, &depth, token);
            current = cdr(current);
        }else{
            //form a list until you pop off an open paren
            Value *newList = makeNull();
            while (tree->c.car->type != OPEN_TYPE) {
                token = tree->c.car;
                newList = cons(token, newList);
                tree = cdr(tree);
                if (tree->type == NULL_TYPE) {
                    printf("Syntax error: too many close parentheses\n");
                    texit(1);
                }
            }
            depth--;
            //popping the newList on the tree
            tree = cdr(tree);
            tree = cons(newList, tree);
            current = cdr(current); 
        }
          
    }
    //too many open parentheses, not enough close parentheses
    if (depth != 0) {
        printf("Syntax error: not enough close parentheses\n");
        texit(1);
    }
    Value *revTree = reverse(tree);
    return revTree;
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Scheme code; use parentheses to indicate subtrees.
void printTree(Value *tree) {
    Value *cur = tree;
    while(cur->type != NULL_TYPE){
        //subtrees
        if(cur->c.car->type == CONS_TYPE){
            printf("(");
            printTree(cur->c.car);
            printf(") ");
        } else if(cur->c.car->type == NULL_TYPE){
            printf("()");
        } else if(cur->c.cdr->type == NULL_TYPE){
            switch (cur->c.car->type){
                case INT_TYPE:
                    printf("%i", cur->c.car->i);
                    break;
                case DOUBLE_TYPE:
                    printf("%.2lf", cur->c.car->d);
                    break;
                case BOOL_TYPE:
                    if(cur->c.car->i){
                        printf("#t");
                    }else{
                        printf("#f");
                    }
                    break;
                case STR_TYPE:
                    printf("\"%s\"",cur->c.car->s);
                    break;
                case SYMBOL_TYPE:
                    printf("%s",cur->c.car->s);
                    break;
                case OPEN_TYPE:
                    printf("(");
                    break;
                case CLOSE_TYPE:
                    printf(")");
                    break;  
                default:
                    break;
            }
        } else{
            //print with a space if there are more items in the tree
            switch (cur->c.car->type){
                case INT_TYPE:
                    printf("%i ", cur->c.car->i);
                    break;
                case DOUBLE_TYPE:
                    printf("%.2lf ", cur->c.car->d);
                    break;
                case BOOL_TYPE:
                    if(cur->c.car->i){
                        printf("#t ");
                    }else{
                        printf("#f ");
                    }
                    break;
                case STR_TYPE:
                    printf("\"%s\" ",cur->c.car->s);
                    break;
                case SYMBOL_TYPE:
                    printf("%s ",cur->c.car->s);
                    break;
                case OPEN_TYPE:
                    printf("( ");
                    break;
                case CLOSE_TYPE:
                    printf(") ");
                    break;  
                default:
                    break;
            }
        }
        cur = cdr(cur);
    } 
}
