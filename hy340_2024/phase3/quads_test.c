#include "quads.h"



//tester function
int main(){
     expr* result =  (expr*)newexpr_type(assignexpr_e);
     symrec* s;
     symrec* a1;
     symrec* a2;
     char buff[32];
     insert("res", GLOBALVAR, 0, 0);
     s = lookup("res");
     result->sym = s;
     
     expr* arg1 =  (expr*)newexpr_type(var_e);
     expr* arg2 = (expr*)newexpr_type(var_e);
     expr* arg3 = (expr*)newexpr_numConst(99.22);
     insert("arg1", GLOBALVAR, 0, 0);
     a1 = lookup("arg1");
     arg1->sym = a1;
     insert("arg2", GLOBALVAR, 0, 0);
     a2 = lookup("arg2");
     arg2->sym = a2;

     //params
     expr* elist = NULL; 
     expr* e = newexpr_type(var_e);
     sprintf(buff,"arg0");
      insert(buff,FORMAL,0,1);
      e->sym = lookup(buff);
     elist = e;
    for(int i=1;i<=10;i++){
      e = newexpr_type(var_e);
      sprintf(buff,"arg%d",i);
      insert(buff,FORMAL,0,1);
      e->sym = lookup(buff);
      add_to_expr_list(elist,e);
    }
    emit_function_params(elist);
   //end of params test

   //  result->type =  arithexpr_e;


   emit(assign, result, arg1, arg2, 0, 0);
   emit(assign, result, arg1, arg3, 0, 0);
   emit(assign, result, arg1, arg2, 0, 0);
   emit(jump,NULL,NULL,NULL,26,6);
     print_quads();

     

    return 1;
}