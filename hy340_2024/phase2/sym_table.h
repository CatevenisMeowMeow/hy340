#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>

typedef enum Type{GLOBAL, LOCAL, FORMAL, FUNCTION, LIBFUNCTION} Type;

char *type_to_string[] = {
    "GLOBAL",
    "LOCAL",
    "FORMAL",
    "FUNCTION",
    "LIBFUNCTION"
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

symrec* lookup(char* name);

symrec* lookup_scope(char* name, int scope);

int is_hidden(symrec* rec);
