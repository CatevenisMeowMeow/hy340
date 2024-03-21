#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>

typedef enum Type{GLOBAL, LOCAL, FORMAL, FUNCTION, LIBFUNCTION} Type;

//Will be used to make print easier
char *type_to_string[] = {
    "global variable",
    "local variable",
    "formal argument",
    "user function",
    "library function"
};

typedef struct sym_table{
    int active;
    int scope;
    int line;
    char* name;
    Type type;
    struct sym_table* next;
    struct sym_table* next_scope;
} symrec;

symrec *sym_table_head = NULL;

void insert(char* name, Type type, int line, int scope);

void insert_library_functions();

symrec* lookup(char* name);

symrec* lookup_scope(char* name, int scope);

int is_hidden(symrec* rec);

void hide(int scope);

void activate(int scope);



