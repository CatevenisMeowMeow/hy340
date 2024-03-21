#include"sym_table.h"


void insert(char* name, Type type, int line, int scope){
    symrec* tmp;
    //create node
    symrec *new_symbol = (symrec *)malloc(sizeof(symrec));
    assert(new_symbol != NULL);
    new_symbol->active = 1;
    new_symbol->scope = scope;
    new_symbol->line = line;
    new_symbol->type = type;
    new_symbol->next = NULL;
    new_symbol->next_scope = NULL;
    new_symbol -> name = (char*)malloc(strlen(name));
    assert(new_symbol->name != NULL);
    strcpy(new_symbol->name, name);

    //Insert to list
    //for the first one
    if(sym_table_head == NULL){
        sym_table_head = new_symbol;
        return;
    }
    symrec *prev = NULL;
    symrec *curr = sym_table_head;
    //search through scopes
    while(curr != NULL && curr->scope < scope){
        prev = curr;
        curr = curr->next_scope;
    }

    //First new scope
    if(curr == NULL){
        tmp = prev;
        if(tmp == NULL){
            tmp = sym_table_head;
            tmp->next_scope = new_symbol;
        }
        while(tmp->next != NULL){
            tmp->next_scope = new_symbol;
            tmp = tmp->next;
        }
        tmp->next_scope = new_symbol;
        tmp->next = new_symbol;
        return;
    }
    //found scope
    if(curr->scope == scope){
        new_symbol->next = curr;
        new_symbol->next_scope = curr->next_scope;
        if(prev == NULL){
            sym_table_head = new_symbol;
        }
        else{
            tmp = prev;
            while(tmp->scope < scope){
                prev = tmp;
                tmp->next_scope = new_symbol;
                tmp = tmp->next;
            }
            prev->next_scope = new_symbol;
            prev->next = new_symbol;
        }
        return;
    }
    //new scope somewhere between
    if(curr->scope > scope && prev->scope < scope){ 
        tmp = prev;
        while(tmp->scope < scope){
                prev = tmp;
                tmp->next_scope = new_symbol;
                tmp = tmp->next;
        }
        new_symbol->next = tmp;
        new_symbol->next_scope = tmp;
        prev->next = new_symbol;
        prev->next_scope = new_symbol;
        return;
    }

    
    printf("Insert function must never come here!!!!!!\n");
    printf("scope  %d  \n",scope);

}


void insert_library_functions(){
    insert("print",LIBFUNCTION,0,0);
    insert("input",LIBFUNCTION,0,0);
    insert("objectmemberkeys",LIBFUNCTION,0,0);
    insert("objecttotalmembers",LIBFUNCTION,0,0);
    insert("objectcopy",LIBFUNCTION,0,0);
    insert("totalarguments",LIBFUNCTION,0,0);
    insert("argument",LIBFUNCTION,0,0);
    insert("typeof",LIBFUNCTION,0,0);
    insert("strtonum",LIBFUNCTION,0,0);
    insert("sqrt",LIBFUNCTION,0,0);
    insert("cos",LIBFUNCTION,0,0);
    insert("sin",LIBFUNCTION,0,0);
}


symrec* lookup(char* name){
    symrec *tmp = sym_table_head;
    while(tmp != NULL){
        if(strcmp(tmp->name,name) == 0)
            break;
    }
    return tmp;
}

symrec* lookup_scope(char* name, int scope);


int is_hidden(symrec* rec){
    assert(rec != NULL);
    return rec->active; //1 if active. 0 if not
}

void hide(int scope){
    symrec* tmp = sym_table_head;
    while(tmp != NULL){
        if(tmp->scope == scope)
            tmp->active = 0;
        tmp = tmp->next;
    }
}
//oposite of hide
void activate(int scope){
    symrec* tmp = sym_table_head;
    while(tmp != NULL){
        if(tmp->scope == scope)
            tmp->active = 1;
        tmp = tmp->next;
    }
}


void print_symbol_table(){
    symrec *tmp = sym_table_head;
    int scope_change = 1;
    while(tmp != NULL){
        if(scope_change == 1){
            printf("\n-------    Scope  #%d    -------\n",tmp->scope);
            scope_change = 0;
        }
        printf("%s  [%s]  (line %d)  (scope %d)\n",tmp->name,type_to_string[tmp->type],tmp->line,tmp->scope);
        if(tmp->next != NULL && tmp->next->scope != tmp->scope)
            scope_change = 1;
        tmp = tmp->next; 
    }


}




//proswrinos tester code
int main(){

    insert_library_functions();
    insert("a", GLOBAL, 0, 0);
    insert("a", GLOBAL, 0, 0);
    insert("b", GLOBAL, 1, 2);
    insert("c", FUNCTION, 2, 0);
    insert("d", FORMAL, 3, 1);
    insert("e", LOCAL, 4, 1);
    insert("f", GLOBAL, 5, 7);
    insert("f2", GLOBAL, 5, 7);
    insert("k", GLOBAL, 6, 10);
    insert("g", GLOBAL, 7, 0);
    insert("h", FUNCTION, 8, 0);
    insert("i", FORMAL, 9, 0);
    insert("j", LOCAL, 10, 1);
  
  
    print_symbol_table();

    /*symrec* tmp = sym_table_head;
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
    */

    return 0;
}
