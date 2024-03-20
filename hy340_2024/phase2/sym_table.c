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
    //search through scopes to go faster
    while(curr != NULL && curr->scope < scope){
        prev = curr;
        curr = curr->next_scope;
    }
    
    //First new scope
    if(curr == NULL){
        prev->next_scope = new_symbol;
        prev->next = new_symbol;
        return;
    }
    //found scope
    if(curr->scope == scope){
        tmp = curr;
        while(tmp->next != NULL && tmp->scope == scope){
            tmp = tmp->next;
        }
        if(tmp->next == NULL){
            tmp->next_scope = new_symbol;
            tmp->next = new_symbol;
        }
        else{
            new_symbol->next = tmp->next;
            new_symbol->next_scope = tmp->next_scope;
            tmp->next = new_symbol;
        }
        return;
    }
    printf("Insert function must never come here!!!!!!\n");

}

symrec* lookup(char* name){



}

symrec* lookup_scope(char* name, int scope);


int is_hidden(symrec* rec){
    assert(rec != NULL);
    return rec->active; //1 if active. 0 if not
}



//proswrinos tester code
int main(){

   
    insert("a", GLOBAL, 0, 0);
    insert("b", GLOBAL, 1, 0);
    insert("c", FUNCTION, 2, 0);
    insert("d", FORMAL, 2, 1);
    insert("e", LOCAL, 3, 1);
    insert("f", GLOBAL, 0, 0);
    insert("g", GLOBAL, 1, 0);
    insert("h", FUNCTION, 2, 0);
    insert("i", FORMAL, 2, 0);
    insert("j", LOCAL, 3, 1);
    insert("k", GLOBAL, 7, 10);
  


    symrec* tmp = sym_table_head;
    while(tmp!=NULL){
        printf("%s is %s at line %d and scope %d\n",tmp->name,type_to_string[tmp->type],tmp->line,tmp->scope);
        tmp = tmp->next;
    }


    return 0;
}
