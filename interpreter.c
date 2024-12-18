#include <assert.h>
#include <stdlib.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"
#include <stdio.h>
#include "parser.h"
#include "interpreter.h"
#include <string.h>
//prints an evaluation error with a reason as a string for the parameter
void evaluationError(char *string){
    printf("Evaluation error: ");
    printf("%s\n",string);
    texit(1);
}
//checks if the symbol is in the frame
Value *lookUpSymbol(Value *tree, Frame *frame){
   Value *result = makeNull();
   Frame *curFrame = frame;
   bool isduplicate = 0;
   bool exists = 0;
   while (curFrame != NULL){
      Value *cur = curFrame->bindings;
      while(cur->type !=NULL_TYPE){
         if(!strcmp(cur->c.car->c.car->s,tree->s)){
            if(isduplicate == 0){
               exists = 1;
               result = cdr(car(cur));
               isduplicate = 1;
            }else {
               evaluationError("duplicate variable");
            }
         }
         cur = cdr(cur);
      }
      if(result->type !=NULL_TYPE && exists == 0){
         return result;
      }
      isduplicate = 0;
      curFrame = curFrame->parent;
   }
   
   if(result->type == NULL_TYPE){
      printf("Evaluation error: symbol '%s' not found.\n", tree->s);
      texit(1);
   }
   return result;

}
//evaluates each argument
Value *evalEach(Value *args, Frame *frame){
   Value *evaledArgs = makeNull();
   while(!isNull(args)){
      evaledArgs = cons(eval(car(args), frame), evaledArgs);
      args = cdr(args);
   }
   return reverse(evaledArgs);
}
//evaluate the if statements
Value *evalIf(Value *args, Frame *frame){
   Value *result= makeNull(); 
   if(args->type != CONS_TYPE || args->c.cdr->type == NULL_TYPE || args->c.cdr->c.cdr->type == NULL_TYPE){
      evaluationError("no consequent following of an if.");
   }
   if(eval(car(args), frame)->type == BOOL_TYPE){
      Value *current = eval(car(args), frame);
      if(current->i){
         result = eval(car(cdr(args)), frame);
      } else {
         result = eval(car(cdr(cdr(args))),frame);
      }
   }
    return result;
}
//evaluates let statements
Value *evalLet(Value *args, Frame *frame){
   if(args->c.cdr->type != CONS_TYPE){
      evaluationError("no args following the bindings in let.");
   }
   Value *remainArgs = cdr(args);
   Frame *newFrame = talloc(sizeof(Frame));
   newFrame->parent = frame;
   newFrame->bindings = makeNull();
   Value *addBindings = makeNull();
   Value *headBindings = car(args);
   if(headBindings->type != CONS_TYPE){
      return remainArgs;
   }
   if(headBindings->c.car->type != CONS_TYPE){
      evaluationError("");
   }
   while(headBindings->type != NULL_TYPE){
      Value *bindingsPair = car(headBindings);
      Value *key = car(bindingsPair);
      Value *value = eval(car(cdr(bindingsPair)), frame);
      Value *pair = cons(key, value); 
      

      if(key->type != SYMBOL_TYPE){
         evaluationError("");
      }
      addBindings = cons(pair, addBindings);
      headBindings = headBindings->c.cdr;
   }
   newFrame->bindings = addBindings;
   Value *result = makeNull();
   if(remainArgs->type == SYMBOL_TYPE){
      result = lookUpSymbol(remainArgs, newFrame);
   } else {
      if(cdr(remainArgs)->type != NULL_TYPE){
         Value *evalVoid = eval(car(remainArgs), newFrame);
         if(evalVoid->type != VOID_TYPE){
            result = eval(car(remainArgs), newFrame);
         }
         remainArgs = cdr(remainArgs);
      }
      result = eval(car(remainArgs), newFrame);
   }  
   return result;
   
}
//evaluates quote statements
Value *evalQuote(Value *args){
   //if there are no arguments to quote
   if(args->type == NULL_TYPE){
      evaluationError("not enough arguments to quote");
   }
   //if there is more than one argument to quote
   if(cdr(args)->type != NULL_TYPE){
      evaluationError("too many arguments to quote");
   }
   return car(args);
}
//evaluates define statements
Value *evalDefine(Value *args, Frame *frame){
   if(args->type == NULL_TYPE){
      evaluationError("no arguments following define");
   }
   if(car(args)->type != SYMBOL_TYPE){
      evaluationError("define must bind to a symbol");
   }
   if(cdr(args)->type == NULL_TYPE){
      evaluationError("no arguments following symbol in define");
   }
   if(cdr(cdr(args))->type != NULL_TYPE){
      evaluationError("too many arguments");
   }
   Value *void_val = makeNull();
   void_val->type = VOID_TYPE;

   Value *key = car(args);
   Value *value = eval(car(cdr(args)), frame);
   Value *pair = cons(key, value);
   Value *addBindings = cons(pair, frame->bindings);
   frame->bindings = addBindings;
   return void_val;
}
//evaluates lambda statements
Value *evalLambda(Value *args, Frame *frame){
   if(isNull(args)){
      evaluationError("no args following lambda.");
   }
   if(isNull(cdr(args))){
      evaluationError("no code in lambda following parameters");
   }
   if(!(car(args)->type == NULL_TYPE ||car(car(args))->type == SYMBOL_TYPE)){ 
      evaluationError("formal parameters for lambda must be symbols.");
   }
   Value *params = car(args);
   //tests for duplicate identifiers in lambda
   if(!isNull(params)){
      Value *current = car(args);
      while(current->type != NULL_TYPE){                                
         Value *next = cdr(current);
         while(next->type != NULL_TYPE){
            if(!strcmp(car(current)->s, car(next)->s)){
               evaluationError("duplicate identifier in lambda");
            }
            next = cdr(next);
         }
         current = cdr(current);
      }
   }
   Value *code = car(cdr(args));
   Value *newClosure = talloc(sizeof(Value));
   newClosure->type = CLOSURE_TYPE;
   newClosure->cl.frame = frame;
   newClosure->cl.paramNames = params;
   newClosure->cl.functionCode = code;
   return newClosure;
}
//evaluates let statements recursively
Value *evalLetrec(Value *args, Frame *frame){
   if(args->c.cdr->type != CONS_TYPE){
      evaluationError("no args following the bindings in letrec.");
   }
   Frame *newEnv = talloc(sizeof(Frame));
   newEnv->bindings = makeNull();
   newEnv->parent = frame;
   Value *addBindings = makeNull();
   Value *current = car(args);
   Value *currentBindings = frame->bindings;
   while(current->type != NULL_TYPE){
      Value *bindingsPair = car(current);
      Value *key = car(bindingsPair);
      Value *value = eval(car(cdr(bindingsPair)), newEnv);
      if(value->type == UNSPECIFIED_TYPE){
         evaluationError("");
      }
      value->type = UNSPECIFIED_TYPE;
      Value *pair = cons(key, value);
      addBindings = cons(pair, addBindings);
      newEnv->bindings = addBindings;
      current = current->c.cdr; 
   }
   return eval(car(cdr(args)), newEnv);
}
//evaluates set! statements
Value *evalSetBang(Value *args, Frame *frame){
   Value *current = args;
   Value *void_val = makeNull();
   void_val->type = VOID_TYPE;
   
   Value *key = car(args);
   Value *checkForValue = lookUpSymbol(key, frame);
   Value *value = eval(car(cdr(args)), frame);
   Frame *curFrame = frame;
   while (curFrame != NULL){
      Value *cur = curFrame->bindings;
      while(cur->type !=NULL_TYPE){
         if(!strcmp(cur->c.car->c.car->s,key->s)){
            cur->c.car->c.cdr = value;
            break;
         }
         cur = cdr(cur);
      }
      curFrame = curFrame->parent;
   }
   return void_val;
}
//evaluates begin statements
Value *evalBegin(Value *args, Frame *frame){
   Value *evaledArgs = evalEach(args, frame);
   Value *void_val = makeNull();
   void_val->type = VOID_TYPE;
   while(!isNull(evaledArgs)){
      if(isNull(cdr(evaledArgs))){
         if(car(evaledArgs)->type == NULL_TYPE || car(evaledArgs)->type == VOID_TYPE){
            return void_val;
         }
         return car(evaledArgs);
      }
      evaledArgs = cdr(evaledArgs);
      
   }
   return void_val;
}
//applies the function parameter on the arguments and returns the evaluated code in the closure
Value *apply(Value *function, Value *args){
   if(function->type == PRIMITIVE_TYPE){
      return function->pf(args);
   }
   Frame *newFrame = talloc(sizeof(Frame));
   newFrame->parent = function->cl.frame;
   Value *addBindings = makeNull();
   Value *current = function->cl.paramNames;
   //mapping bindings
   while(current->type != NULL_TYPE && current->type == CONS_TYPE){
      Value *key = car(current);
      Value *value = car(args);
      Value *pair = cons(key, value);
      addBindings = cons(pair,addBindings);
      current = cdr(current);
      args = cdr(args);
   }
   newFrame->bindings = addBindings;
   Value *evaledBody = eval(function->cl.functionCode,newFrame);
   return evaledBody;
}
Value *primitiveAdd(Value *args){
   Value *result = makeNull();
   if(isNull(args)){
      result->type = INT_TYPE;
      result->i = 0;
      return result;
   }
   Value *current = args;
   bool isDouble = 0;
   float value = 0;   
   while(current->type != NULL_TYPE){ 
      if(car(current)->type != INT_TYPE && car(current)->type != DOUBLE_TYPE){
         evaluationError("+ must take numbers");
      }
      if(car(current)->type == DOUBLE_TYPE){
         isDouble = 1;
         value = value + car(current)->d;
      } else{
         value = value + car(current)->i;
      }
      current = cdr(current);
   }
   if(isDouble){
      result->type = DOUBLE_TYPE;
      result->d = value;
   } else{
      result->type = INT_TYPE;
      result->i = value;
   }
   return result;
}
Value *primitiveIsNull(Value *args){
   Value *result = makeNull();
   result->type = BOOL_TYPE;
   if(isNull(args)){
      evaluationError("args cannot be empty");
   } else{
      if(args->type == CONS_TYPE && cdr(args)->type != NULL_TYPE){
         evaluationError("null can only take one argument");
      }
      if(isNull(car(args))){
         result->i = 1;
      } else{
         result->i = 0;
      }
   }
   return result;
}
//evaluates primitive car functions
Value *primitiveCar(Value *args)
{
   if (args->type != CONS_TYPE || cdr(args)->type != NULL_TYPE)
   {
      evaluationError("Bad form");
   }

   Value *list = car(args);

   if (list->type != CONS_TYPE)
   {
      evaluationError("Not a list");
   }

   return car(list);
}
//evaluates primitive cdr functions
Value *primitiveCdr(Value *args)
{
   if (args->type != CONS_TYPE)
   {
      evaluationError("Bad form");
   }
   
   Value *list = car(args);

   if (list->type != CONS_TYPE)
   {
      evaluationError("Not a list");
   }
   return cdr(list);
}
//evaluates cons primitive functions
Value *primitiveCons(Value *args)
{
   // if(isNull(args) || isNull(car(args)) || 
   if(isNull(args) || isNull(cdr(args))){
      evaluationError("invalid number of arguments");
   }

   if (args->type != CONS_TYPE || cdr(cdr(args))->type !=NULL_TYPE)
   {
      evaluationError("Bad form");
   }
   Value *list1 = car(args);
   Value *list2 = car(cdr(args));
   
   return cons(list1, list2);
}
Value *primitiveSubtract(Value *args){
   Value *result = makeNull();
   if(isNull(args)){
      result->type = INT_TYPE;
      result->i = 0;
      return result;
   }
   if(isNull(cdr(args))){
      result->type = car(args)->type;
      if(car(args)->type == DOUBLE_TYPE){
         result->d = (0-car(args)->d); 
      }else{
         result->i =(0-car(args)->i);
      }
      return result;
   }
   Value *current = args;
   bool isDouble = 0;
   float value = 0;   
   if(car(current)->type != INT_TYPE && car(current)->type != DOUBLE_TYPE){
      evaluationError("+ must take numbers");
   }
   if(car(current)->type == DOUBLE_TYPE){
      isDouble = 1;
      value = car(current)->d + primitiveSubtract(cdr(current))->d;
   } else{ 
      value = car(current)->i + primitiveSubtract(cdr(current))->i;
   }
   if(isDouble){
      result->type = DOUBLE_TYPE;
      result->d = value;
   } else{
      result->type = INT_TYPE;
      result->i = value;
   }
   return result;
}
Value *primitiveLessThan(Value *args){
   Value *result = makeNull();
   result->type = BOOL_TYPE;
   if(isNull(args)){
      evaluationError("args cannot be empty");
   } else{
      if(isNull(cdr(args)) || !isNull(cdr(cdr(args)))){
         evaluationError("less than accepts two arguments");
      }
      if(car(args)->type == DOUBLE_TYPE || car(cdr(args))->type == DOUBLE_TYPE){
         if(car(args)->d < car(cdr(args))->d){
            result->i = 1;
         } else{
            result->i = 0;
         }
      } else{
         if(car(args)->i < car(cdr(args))->i){
            result->i = 1;
         } else{
            result->i = 0;
         }
      }
      
   }
   return result;
}
Value *primitiveGreaterThan(Value *args){
   Value *result = makeNull();
   result->type = BOOL_TYPE;
   if(isNull(args)){
      evaluationError("args cannot be empty");
   } else{
      if(isNull(cdr(args)) || !isNull(cdr(cdr(args)))){
         evaluationError("less than accepts two arguments");
      }
      if(car(args)->type == DOUBLE_TYPE || car(cdr(args))->type == DOUBLE_TYPE){
         if(car(args)->d > car(cdr(args))->d){
            result->i = 1;
         } else{
            result->i = 0;
         }
      } else{
         if(car(args)->i > car(cdr(args))->i){
            result->i = 1;
         } else{
            result->i = 0;
         }
      }
   }
   return result;
}
Value *primitiveEqual(Value *args){
   Value *result = makeNull();
   result->type = BOOL_TYPE;
   if(car(args)->type == DOUBLE_TYPE || car(cdr(args))->type == DOUBLE_TYPE){
         if(car(args)->d == car(cdr(args))->d){
            result->i = 1;
         } else{
            result->i = 0;
         }
   } else{
      if(car(args)->i == car(cdr(args))->i){
         result->i = 1;
      } else{
         result->i = 0;
      }
   }
   return result;
}
void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
   // Add primitive functions to top-level bindings list
   Value *value = talloc(sizeof(Value));
   value->type = PRIMITIVE_TYPE;
   value->pf = function;
   Value *key = makeNull();
   key->type = SYMBOL_TYPE;
   key->s = name;
   Value *pair = cons(key, value);
   frame->bindings = cons(pair, frame->bindings);
}
//interprets list if tree is a cons type
void interpretLists(Value *tree)
{
   if (tree->type != CONS_TYPE)
   {
      switch (tree->type)
      {
      case SYMBOL_TYPE:
         printf("%s", tree->s);
         break;
      case BOOL_TYPE:
         if (tree->i)
         {
            printf("#t");
         }
         else
         {
            printf("#f");
         }
         break;
      case INT_TYPE:
         printf("%i", tree->i);
         break;
      case DOUBLE_TYPE:
         printf("%lf", tree->d);
         break;
      case STR_TYPE:
         printf("\"%s\"", tree->s);
         break;
      default:
         break;
      }
   }
   else
   {
      if (car(tree)->type == CONS_TYPE && (tree)->type == CONS_TYPE)
      {
         printf("(");
         interpretLists(car(tree));
         printf(") ");
         interpretLists(cdr(tree));  
      }
      else if(car(tree)->type != CONS_TYPE && (cdr(tree)->type != CONS_TYPE && cdr(tree)->type != NULL_TYPE))
      {
         interpretLists(car(tree));
         printf(" . ");
         interpretLists(cdr(tree));  
      } else{
         interpretLists(car(tree));
         printf(" ");
         interpretLists(cdr(tree));  
                
      }
   }
}
// interprets the evaluated scheme code associated with a parsed scheme tree
void interpret(Value *tree){
   if(tree->type == NULL_TYPE){
      return;
   } 
   Frame *global = talloc(sizeof(Frame));
   global->parent = NULL;
   global->bindings = makeNull();

   bind("+", primitiveAdd, global);
   bind("null?", primitiveIsNull, global);
   bind("car", primitiveCar, global);
   bind("cdr", primitiveCdr, global);
   bind("cons", primitiveCons, global);
   bind("-", primitiveSubtract, global);
   bind("<", primitiveLessThan, global);
   bind(">", primitiveGreaterThan, global);
   bind("=", primitiveEqual, global);
   
   //if cdr is also a linked list of scheme statements
   Value *current = tree;
   while (current->type != NULL_TYPE)
   {
      Value *val = eval(car(current), global);
      switch (val->type)
      {
      case SYMBOL_TYPE:
         printf("%s\n",val->s);
         break;
      case BOOL_TYPE:
         if(val->i){
            printf("#t\n");
         }else{
            printf("#f\n");
         }
         break;
      case INT_TYPE:
         printf("%i\n", val->i);
         break;
      case DOUBLE_TYPE:
         printf("%lf\n", val->d);
         break;
      case STR_TYPE:
         printf("\"%s\"\n",val->s);
         break;
      case CONS_TYPE:
         printf("(");
         interpretLists(val);
         printf(")\n");
         break;
      case NULL_TYPE:
         printf("()\n");
         break;
      case CLOSURE_TYPE:
         printf("#<procedure>\n");
         break;
      default: 
         break;
      }
      current = cdr(current);
   }  
   
}

//evaluates the parsed scheme tree
Value *eval(Value *tree, Frame *frame) {
   switch (tree->type)  {
      case SYMBOL_TYPE: {
         return lookUpSymbol(tree, frame);
         break;
      }  
      case CONS_TYPE: {
         Value *first = car(tree);
         Value *args = cdr(tree);
         Value  *result = makeNull();
         
         if(first->type != SYMBOL_TYPE){
            result = apply(eval(first,frame), evalEach(args, frame));
         }else{
            if (!strcmp(first->s,"if")) {
               result = evalIf(args,frame);
            } else if(!strcmp(first->s, "let")){
               result = evalLet(args, frame);
            } else if(!strcmp(first->s, "quote")){
               result = evalQuote(args);
            } else if(!strcmp(first->s, "define")){
               result = evalDefine(args, frame);
            } else if(!strcmp(first->s, "lambda")){
               result = evalLambda(args, frame);
            } else if(!strcmp(first->s, "letrec")){
               result = evalLetrec(args, frame);
            } else if(!strcmp(first->s, "set!")){
               result = evalSetBang(args, frame);
            } else if(!strcmp(first->s, "begin")){
               result = evalBegin(args, frame); 
            }else{
               Value *evaledOperator = eval(first, frame);
               Value *evaledArgs = evalEach(args, frame);
               return apply(evaledOperator, evaledArgs);
            }
         }
         // .. other special forms here...
         return result;
         break;
      }
      default: 
         return tree;
         break;

    }    
}