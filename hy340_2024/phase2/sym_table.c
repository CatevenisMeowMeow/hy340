#include"sym_table.h"

void insert(char* name, Type type, int line, int scope){
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


    //TODOOOOOOOOO NOT WORKING WELL
    //Insert to list
    if(sym_table_head == NULL){
        sym_table_head = new_symbol;
        return;
    }
    symrec *prev = NULL;
    symrec *curr = sym_table_head;
    while(curr != NULL && curr->scope < scope){
        prev = curr;
        curr = curr->next_scope;
    }

    if(curr == NULL){
        prev->next_scope = new_symbol;
        prev->next = new_symbol;
        return;
    }
    
    if(prev == NULL){
        symrec* tmp = curr;
        while(tmp->next != NULL){
            tmp = tmp->next;
        }
        tmp->next = new_symbol;
        return;
    }
    symrec* tmp = curr;
    while(tmp->next != NULL && tmp->next->scope == scope){
        tmp = tmp->next;
    }  
    new_symbol->next_scope = tmp->next_scope;
    new_symbol->next = tmp->next;
    tmp->next = new_symbol;
    
    


}

symrec* lookup(char* name){



}

symrec* lookup_scope(char* name, int scope);

int is_hidden(symrec* rec);



//Tester code
int main(){

   
    insert("aaa", GLOBAL, 0, 0);
    insert("bbb", GLOBAL, 1, 0);
    insert("a function", FUNCTION, 2, 0);
    insert("a", FORMAL, 2, 1);
    insert("b", LOCAL, 3, 1);
    insert("c", GLOBAL, 0, 0);
    insert("d", GLOBAL, 1, 0);
    insert("rwfunction", FUNCTION, 2, 0);
    insert("fll", FORMAL, 2, 0);
    insert("l", LOCAL, 3, 1);
    insert("aaa", GLOBAL, 0, 0);

    symrec* tmp = sym_table_head;
    while(tmp!=NULL){
        printf("%s is %s at line %d and scope %d\n",tmp->name,type_to_string[tmp->type],tmp->line,tmp->scope);
        tmp = tmp->next;
    }


    return 0;
}
