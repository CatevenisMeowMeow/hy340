#include"sym_table.h"



//proswrinos tester code
int main(){

    insert_library_functions();
    insert("a", GLOBALVAR, 0, 0);
    insert("a", GLOBALVAR, 0, 0);
    insert("b", GLOBALVAR, 1, 2);
    insert("c", USERFUNCTION, 2, 0);
    insert("d", FORMAL, 3, 1);
    insert("e", LOCALVAR, 4, 1);
    insert("f", GLOBALVAR, 5, 7);
    insert("f2", GLOBALVAR, 5, 7);
    insert("k", GLOBALVAR, 6, 10);
    insert("g", GLOBALVAR, 7, 0);
    insert("h", USERFUNCTION, 8, 0);
    insert("i", FORMAL, 9, 0);
    insert("j", LOCALVAR, 10, 1);

    //symrec* f = lookup("f");
     symrec* f = lookup("input");
     if(is_library_function("objectmemberkeys"))
        printf("library function!!!!!!!!!!!\n");
    if(f != NULL)
        printf("Found %s at line %d and scope %d\n",f->name,f->line,f->scope);
    else
        printf("Not found\n");
  
  
    //print_symbol_table();

    symrec* tmp = sym_table_head;
    while(tmp!=NULL){
        printf("%s is %s at line %d and scope %d\n",tmp->name,type_to_string[tmp->type],tmp->line,tmp->scope);
        tmp = tmp->next;
    }
    tmp = sym_table_head;
    printf("\n\n");
    printf("Now we print the first one for each scope\n");
    while(tmp!=NULL){
        printf("%s is %s at line %d and scope %d\n",tmp->name,type_to_string[tmp->type],tmp->line,tmp->scope);
        tmp = tmp->next_scope;
    }
    

    return 0;
}
