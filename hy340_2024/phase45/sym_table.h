#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#define MAX_SIZE 128


int scope = 0;


typedef enum Type{GLOBALVAR, LOCALVAR, FORMAL, USERFUNCTION, LIBFUNCTION} Type;

typedef enum scopespace_t{programvar, functionlocal, formalarg} scopespace_t;

//Will be used to make print easier
char *type_to_string[] = {
    "global variable",
    "local variable",
    "formal argument",
    "user function",
    "library function"
};


typedef struct returnList{
    unsigned label;
    struct returnList *next;
} returnList;


typedef struct sym_table{
    int active;
    int scope;
    int line;
    char* name;
    Type type;
    scopespace_t space;
    unsigned offset;
    unsigned totalLocals;
    unsigned iaddress;
    unsigned taddress;
    struct returnList* returnlist;
    struct sym_table* next;
    struct sym_table* next_scope;
} symrec;



//will be usefull
typedef struct stack{
    int top;
    int items[MAX_SIZE];
} Stack;


//STACK FUNCTIONS

Stack *newStack();

//Initializer
void initializeStack(Stack *stack);

// Function to check if the stack is empty
int isEmpty(Stack *stack);

// Function to check if the stack is full
int isFull(Stack *stack);

// Function to push an item onto the stack
void push(Stack *stack, int item);

// Function to pop an item from the stack
int pop(Stack *stack);

// Function to return the top element of the stack without popping it
int top(Stack *stack);
 
//SYMBOL TABLE FUNCTIONS

symrec *sym_table_head = NULL;

void print_symbol_table();

void insert(char* name, Type type, int line, int scope);

symrec* new_symbol(char* name,Type type, int line, int scope);

void insert_library_functions();

symrec* lookup(char* name);

symrec* lookup_scope(char* name, int scope);

int is_hidden(symrec* rec);

void hide_scope(int scope);

void activate_scope(int scope);

int is_library_function(char* name);



//STACK FUNCTIONS
Stack *newStack(){
    Stack *s = malloc(sizeof(struct stack));
    return s;
}

void initializeStack(Stack *stack) {
    stack->top = -1;
    Stack *tmp = stack;
    for(int i = 0; i < MAX_SIZE; i++)
        tmp->items[i] = 0;
}


int isEmpty(Stack *stack) {
    return (stack->top == -1);
}


int isFull(Stack *stack) {
    return (stack->top == MAX_SIZE - 1);
}


void push(Stack *stack, int item) {
    if (isFull(stack)) {
        fprintf(stderr,"Stack overflow! Cannot push element %d\n", item);
        return;
    }
    stack->items[++stack->top] = item;
}


int pop(Stack *stack) {
    if (isEmpty(stack)) {
        fprintf(stderr,"Stack underflow! Cannot pop from empty stack\n");
        exit(1);
    }
    return stack->items[stack->top--];
}

int top(Stack *stack) {
    if (isEmpty(stack)) {
        fprintf(stderr,"Stack is empty\n");
        exit(1);
    }
    return stack->items[stack->top];
}

void top_increment(Stack *stack){
    if (isEmpty(stack)) {
        fprintf(stderr,"Stack is empty\n");
        exit(1);
    }
     stack->items[stack->top]++;
}

void top_decrement(Stack *stack){
    if (isEmpty(stack)) {
        fprintf(stderr,"Stack is empty\n");
        exit(1);
    }
    stack->items[stack->top]--;
}

symrec* new_symbol(char* name,Type type, int line, int scope){
    //create node
    symrec *new_symbol = (symrec *)malloc(sizeof(symrec));
    assert(new_symbol != NULL);
    new_symbol->active = 1;
    new_symbol->scope = scope;
    new_symbol->line = line;
    new_symbol->type = type;
    new_symbol->next = NULL;
    new_symbol->next_scope = NULL;
    new_symbol -> name = (char*)malloc(strlen(name)+1);
    assert(new_symbol->name != NULL);
    strcpy(new_symbol->name, name);
    return new_symbol;

}



//SYMBOL TABLE FUNCTIONS
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
    new_symbol -> name = (char*)malloc(strlen(name)+1);
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
        tmp = tmp->next;
    }
    return tmp;
}

symrec* lookup_scope(char* name, int scope){
    symrec* curr = sym_table_head;
    symrec* prev = NULL;
    
    while(curr != NULL && curr->scope != scope){
        prev = curr;
        curr = curr->next_scope;
    }
    //scope not found
    if(curr == NULL)
        return NULL;
    //searching through scope
    while(curr != NULL && curr->scope == scope){
        if(strcmp(curr->name,name) == 0)
            return curr;
        curr = curr->next;
    }
    return NULL;
}


int is_hidden(symrec* rec){
    assert(rec != NULL);
    return rec->active; //1 if active. 0 if not
}

void hide_scope(int scope){
    symrec* tmp = sym_table_head;
    while(tmp != NULL){
        if(tmp->scope == scope)
            tmp->active = 0;
        tmp = tmp->next;
    }
}
//oposite of hide
void activate_scope(int scope){
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

int is_library_function(char* name){
    symrec *tmp = sym_table_head;
    tmp = lookup_scope(name,0);
    if(tmp != NULL && tmp->type == LIBFUNCTION)
        return 1;
    return 0;
}





