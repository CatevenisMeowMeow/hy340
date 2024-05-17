
#include "quads.h"

//from lectures
typedef enum vmopcode{
    assign_v, add_v, sub_v,
    mul_v, div_v, mod_v,
    uminus_v, and_v, or_v,
    not_v, if_eq_v, if_noteq_v,
    if_lesseq_v, if_greatereq_v, if_less_v,
    if_greater_v, call_v, pusharg_v,
    getretval_v,  funcstart_v,
    funcend_v, tablecreate_v, jump_v,
    nop_v,  tablegetelem_v, tablesetelem_v
} vmopcode;

//from lectures
typedef enum vmarg_t{
    label_a = 0,
    global_a = 1,
    formal_a = 2,
    local_a = 3,
    number_a = 4,
    string_a = 5,
    bool_a = 6,
    nil_a = 7,
    userfunc_a = 8,
    libfunc_a = 9,
    retval_a = 10
} vmarg_t;

//from lectures
typedef struct vmarg{
    vmarg_t type;
    unsigned val;
} vmarg;

//from lectures
typedef struct instruction{
    vmopcode opcode;
    vmarg result;
    vmarg arg1;
    vmarg arg2;
    unsigned srcLine; 
} instruction;

typedef struct userfunc{
    unsigned address;
    unsigned localSize;
    char* id;
} userfunc;

//Global vars from lectures
double* numConsts;
unsigned totalNumConsts;
char** stringConsts;
unsigned totalStringConsts;
char** namedLibFuncs;
unsigned totalNamedLibFuncs;
userfunc* userFuncs;
unsigned totalUserFuncs;


//from lectures
typedef enum avm_memcell_t{
    number_m = 0,
    string_m = 1,
    bool_m = 2,
    table_m = 3,
    userfunc_m = 4,
    libfunc_m = 5,
    nil_m = 6,
    undef_m  = 7
} avm_memcell_t;

typedef struct avm_table avm_table;
typedef struct avm_memcell{
    avm_memcell_t type;
    union{
        double numVal;
        char* strVal;
        unsigned char boolVal;
        avm_table* tableVal;
        unsigned funcVal;
        char* libfuncVal;
    } data;
} avm_memcell;

#define AVM_STACKSIZE 4096
#define AVM_WIPEOUT(m) memset(&(m), 0, sizeof(m))

avm_memcell stack[AVM_STACKSIZE];
static void avm_initstack(){
    for(unsigned i = 0; i<AVM_STACKSIZE; i++){
        AVM_WIPEOUT(stack[i]);
        stack[i].type = undef_m;
    }
}

avm_table* avm_tablenew();
void avm_tabledestroy();
avm_memcell* avm_tablegetelem(avm_memcell* key);
void avm_tablesetelem(avm_memcell* key, avm_memcell* value);
void avm_memcellclear(avm_memcell* m);

#define AVM_TABLE_HASHSIZE 211

typedef struct avm_table_bucket{
    avm_memcell key;
    avm_memcell value;
    struct avm_table_bucket* next;
} avm_table_bucket;

//TODO
typedef struct avm_table{
    unsigned refCounter;
    avm_table_bucket* strIndexed[AVM_TABLE_HASHSIZE];
    avm_table_bucket* numIndexed[AVM_TABLE_HASHSIZE];
    unsigned total;
} avm_table;

//from lectures
void avm_tableincrefcounter(avm_table* t){
    ++t->refCounter;
}

//from lectures
void avm_tabledecrefcounter(avm_table* t){
    assert(t->refCounter > 0);
    if(!--t->refCounter)
        avm_tabledestroy(t);
}

//from lectures
void avm_tablebucketsinit(avm_table_bucket** p){
    for(unsigned i = 0; i<AVM_TABLE_HASHSIZE; i++)
        p[i] = (avm_table_bucket*) 0;
}

//from lectures
avm_table* avm_tablenew(){
    avm_table* t = (avm_table*)malloc(sizeof(avm_table));
    AVM_WIPEOUT(*t);
    t->refCounter = t->total = 0;
    avm_tablebucketsinit(t->numIndexed);
    avm_tablebucketsinit(t->strIndexed);
    //TODO other vars?

    return t;
}

//from lectures
void avm_tablebucketsdestroy(avm_table_bucket** p){
    for(unsigned i = 0; i<AVM_TABLE_HASHSIZE; i++, p++)
    //TODOOO lecture 13 slide 27
}



void make_operand(expr* e, vmarg* arg);