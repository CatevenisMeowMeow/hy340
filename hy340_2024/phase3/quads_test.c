#include "quads.h"


int main(){
    extend_quads_array();
     printf("%s\n",newtempname());
     printf("%s\n",newtempname());
     printf("%d\n",is_temp_val("_t"));
     printf("%d\n",currscope());
    // expr* result =  (expr* )malloc(sizeof(expr));
   //  result->type =  arithexpr_e;


   // emit(assign, expr* result, expr* arg1, expr* arg2, unsigned label, unsigned line){
     print_quads();

     

    return 1;
}