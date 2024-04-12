#include "quads.h"



//tester function
int main(){
     expr* result =  (expr*)newexpr_type(assignexpr_e);
     symrec* s;
     symrec* a1;
     symrec* a2;
     insert("res", GLOBALVAR, 0, 0);
     s = lookup("res");
     result->sym = s;
     
     expr* arg1 =  (expr*)newexpr_type(var_e);
     expr* arg2 = (expr*)newexpr_type(var_e);
     insert("arg1", GLOBALVAR, 0, 0);
     a1 = lookup("arg1");
     arg1->sym = a1;
     insert("arg2", GLOBALVAR, 0, 0);
     a2 = lookup("arg2");
     arg2->sym = a2;
     

   //  result->type =  arithexpr_e;


   emit(assign, result, arg1, arg2, 0, 0);
   emit(assign, result, arg1, arg2, 0, 0);
   emit(assign, result, arg1, arg2, 0, 0);
   emit(jump,(expr*)0,(expr*)0,(expr*)0,26,6);
     print_quads();

     

    return 1;
}