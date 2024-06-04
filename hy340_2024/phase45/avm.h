#include "target_code.h"
#include <math.h>

extern void avm_warning(char* format);
extern void avm_error(char* err);

//from lectures
unsigned char executionFinished = 0;
unsigned pc = 0;
unsigned currLine = 0;
unsigned codeSize = 0;
instruction* code = (instruction*)0;
#define AVM_ENDING_PC codeSize
#define AVM_NUMACTUALS_OFFSET +4
#define AVM_SAVEDPC_OFFSET +3
#define AVM_SAVEDTOP_OFFSET +2
#define AVM_SAVEDTOPSP_OFFSET +1

unsigned totalActuals = 0;

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

#define AVM_STACKENV_SIZE 4
avm_memcell ax,bx,cx,retval;

unsigned _top,topsp;

avm_memcell stack[AVM_STACKSIZE];

static void avm_initstack(){
    for(unsigned i = 0; i<AVM_STACKSIZE; i++){
        AVM_WIPEOUT(stack[i]);
        stack[i].type = undef_m;
    }
}


#define AVM_TABLE_HASHSIZE 211

#define AVM_TOTAL_LIBFUNCS 12

typedef void (*execute_func_t)(instruction*);
typedef void (*library_func_t)(void);

typedef void (*memclear_func_t) (avm_memcell*);

typedef char* (*tostring_func_t)(avm_memcell*);
typedef unsigned char (*tobool_func_t)(avm_memcell*);

typedef double (*arithmetic_func_t)(double x, double y);
typedef unsigned char (*cmp_func)(double x, double y);


//A library function map
typedef struct library_func_map{
    char* id;
    library_func_t address;
}library_func_map;

library_func_map libmap[AVM_TOTAL_LIBFUNCS];
unsigned int curr_lib_func = 0;


//For arithmetics
#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

double add_impl(double x, double y){return x+y;}
double sub_impl(double x, double y){return x-y;}
double mul_impl(double x, double y){return x*y;}
double div_impl(double x, double y){if(y == 0) avm_error("Division by zero!!!"); return x/y;}
double mod_impl(double x, double y){ return ((unsigned) x) % ((unsigned) y); }

arithmetic_func_t arithmeticFuncs[] = {
    add_impl,
    sub_impl,
    mul_impl,
    div_impl,
    mod_impl
};

//For bools
#define execute_jge execute_cmp
#define execute_jgt execute_cmp
#define execute_jle execute_cmp
#define execute_jlt execute_cmp

unsigned char jge_impl(double x, double y){return x>=y;}
unsigned char jgt_impl(double x, double y){return x>y;}
unsigned char jle_impl(double x, double y){return x<=y;}
unsigned char jlt_impl(double x, double y){return x<y;}

cmp_func comparizonFuncs[] = {
    jle_impl,
    jge_impl,
    jlt_impl,
    jgt_impl
};


#define AVM_MAX_INSTRUCTIONS (unsigned) nop_v

//from lectures
typedef struct avm_table_bucket{
    avm_memcell key;
    avm_memcell value;
    struct avm_table_bucket* next;
} avm_table_bucket;

//from lectures
typedef struct avm_table{
    unsigned refCounter;
    avm_table_bucket* strIndexed[AVM_TABLE_HASHSIZE];
    avm_table_bucket* numIndexed[AVM_TABLE_HASHSIZE];
    //Bonus if implement other vars support
    unsigned total;
} avm_table;


//for to execute
extern void execute_assign(instruction*);
extern void execute_add(instruction*);
extern void execute_sub(instruction*);
extern void execute_mul(instruction*);
extern void execute_div(instruction*);
extern void execute_mod(instruction*);
extern void execute_uminus(instruction*);

extern void execute_and(instruction*);
extern void execute_or(instruction*);
extern void execute_not(instruction*);

extern void execute_jeq(instruction*);
extern void execute_jne(instruction*);
extern void execute_jle(instruction*);
extern void execute_jge(instruction*);
extern void execute_jlt(instruction*);
extern void execute_jgt(instruction*);

extern void execute_call(instruction*);
extern void execute_pusharg(instruction*);
extern void execute_funcenter(instruction*);
extern void execute_funcexit(instruction*);

extern void execute_newtable(instruction*);
extern void execute_jmp(instruction*);
extern void execute_tablegetelem(instruction*);
extern void execute_tablesetelem(instruction*);

extern void execute_nop(instruction*);


execute_func_t executeFuncs[] = {
    execute_assign,
    execute_add,
    execute_sub,
    execute_mul,
    execute_div,
    execute_mod,
    execute_uminus,
    execute_and,
    execute_or,
    execute_not,
    execute_jeq,
    execute_jne,
    execute_jle,
    execute_jge,
    execute_jlt,
    execute_jgt,
    execute_call,
    execute_pusharg,
    execute_funcenter,
    execute_funcexit,
    execute_newtable,
    execute_jmp, //It was missing from lectures
    execute_nop,
    execute_tablegetelem,
    execute_tablesetelem
};

extern userfunc* avm_getfuncinfo(unsigned address);


extern void avm_assign(avm_memcell* lv, avm_memcell* rv);

extern char* avm_tostring(avm_memcell*);
library_func_t avm_getlibraryfunc(char *id);
extern void avm_callibfunc(char* funcName);
extern void avm_callsaveenvironment();
extern void avm_call_functor(avm_table* t);
extern void avm_push_table_arg(avm_table* t);


//for to string
extern char* number_tostring(avm_memcell* m){
    char* tmp = malloc(sizeof(char*));
    sprintf(tmp,"%.4f",m->data.numVal);
    return tmp;
}

extern char* string_tostring(avm_memcell* m){
    return m->data.strVal;
}

extern char* bool_tostring(avm_memcell* m){
    if(m->data.boolVal)
        return "1";
    return "0";
}

extern char* table_tostring(avm_memcell* m){
    //TODO???
    return "table";
}

extern char* userfunc_tostring(avm_memcell* m){
    char* tmp = malloc(sizeof(char*));
    sprintf(tmp,"Function: %u",m->data.funcVal);
    return tmp;
}

extern char* libfunc_tostring(avm_memcell* m){
    return m->data.libfuncVal;
}

extern char* nil_tostring(avm_memcell* m){
    return "nil";
}

extern char* undef_tostring(avm_memcell* m){
    return "undef";
}

tostring_func_t tostringFuncs[] = {
    number_tostring,
    string_tostring,
    bool_tostring,
    table_tostring,
    userfunc_tostring,
    libfunc_tostring,
    nil_tostring,
    undef_tostring
};

char* avm_tostring(avm_memcell* m){
    assert(m->type >= 0 && m->type <= undef_m);
    return (*tostringFuncs[m->type])(m);
}

//for to bool
unsigned char number_tobool(avm_memcell* m){ return m->data.numVal != 0; }
unsigned char string_tobool(avm_memcell* m){ return m->data.strVal[0] != 0; }
unsigned char bool_tobool(avm_memcell* m){ return m->data.boolVal; }
unsigned char table_tobool(avm_memcell* m){ return 1; }
unsigned char userfunc_tobool(avm_memcell* m){ return 1; }
unsigned char libfunc_tobool(avm_memcell* m){ return 1; }
unsigned char nil_tobool(avm_memcell* m){ return 0; }
unsigned char undef_tobool(avm_memcell* m){ return 0; }
tobool_func_t toboolFuncs[] = {
    number_tobool,
    string_tobool,
    bool_tobool,
    table_tobool,
    userfunc_tobool,
    libfunc_tobool,
    nil_tobool,
    undef_tobool
};

unsigned char avm_tobool(avm_memcell* m){
    assert(m->type >= 0 && m->type < undef_m);
    return (*toboolFuncs[m->type])(m);
}

char* typeStrings[] = {
    "number",
    "string",
    "bool",
    "table",
    "userfunc",
    "libfunc",
    "nil",
    "undef"
};


//prodhlwsh synarthsewn
void avm_initialize();
avm_table* avm_tablenew();
void avm_tabledestroy();
void avm_memcellclear(avm_memcell* m);

avm_memcell* avm_tablegetelem(avm_table* table, avm_memcell* index);
void avm_tablesetelem(avm_table* table, avm_memcell* index, avm_memcell* content);

//from lectures
void avm_tableincrefcounter(avm_table* t);
void avm_tabledecrefcounter(avm_table* t);
void avm_tablebucketsinit(avm_table_bucket** p);

double consts_getnumber(unsigned index);
char* consts_getstring(unsigned index);
char* libfuncs_getused(unsigned index);
userfunc* userfuncs_getfunc(unsigned index);

void avm_dec_top();
void avm_push_envvalue(unsigned val);
unsigned avm_get_envvalue(unsigned i);
void avm_callsaveenvironment();

unsigned avm_totalactuals();
avm_memcell* avm_getactual(unsigned i);

void libfunc_print();
void libfunc_typeof();
void libfunc_totalarguments();
void libfunc_input();
void libfunc_objectmemberkeys();
void libfunc_objecttotalmembers();
void libfunc_objectcopy();
void libfunc_argument();
void libfunc_strtonum();
void libfunc_sqrt();
void libfunc_cos();
void libfunc_sin();

void avm_registerlibfunc(char* id, library_func_t addr);

avm_memcell* avm_translate_operand(vmarg* arg, avm_memcell* reg);

void execute_cycle();

void avm_load_instructions(const char* filename);

void avm_load_instructions(const char* filename) {
    //proswrinh lysh
    code = instructions;
    codeSize = currInstruction;
    //TODO
    /*
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    codeSize = file_size / sizeof(instruction);
    code = (instruction*)malloc(codeSize * sizeof(instruction));

    if (fread(code, sizeof(instruction), codeSize, file) != codeSize) {
        perror("Failed to read instructions");
        fclose(file);
        free(code);
        exit(1);
    }
    printf("total instructions: %u\n",codeSize);
    fclose(file);*/
}



void avm_initialize(){
    avm_initstack();
    avm_registerlibfunc("print",libfunc_print);
    avm_registerlibfunc("typeof",libfunc_typeof);
    avm_registerlibfunc("totalarguments",libfunc_totalarguments);
    avm_registerlibfunc("input",libfunc_input);
    avm_registerlibfunc("objectmemberkeys",libfunc_objectmemberkeys);
    avm_registerlibfunc("objecttotalmembers",libfunc_objecttotalmembers);
    avm_registerlibfunc("objectcopy",libfunc_objectcopy);
    avm_registerlibfunc("argument",libfunc_argument);
    avm_registerlibfunc("strtonum",libfunc_strtonum);
    avm_registerlibfunc("sqrt",libfunc_sqrt);
    avm_registerlibfunc("cos",libfunc_cos);
    avm_registerlibfunc("sin",libfunc_sin);
}

void libfunc_print(){
    unsigned n = avm_totalactuals();
    for(unsigned i = 0; i<n; ++i){
        char* s = avm_tostring(avm_getactual(i));
        puts(s);
        free(s);
    }
}

void libfunc_typeof(){
    unsigned n = avm_totalactuals();
    if(n!=1)
        avm_error("one argument expected in typeof !!");
    else{
        avm_memcellclear(&retval);
        retval.type = string_m;
        retval.data.strVal = strdup(typeStrings[avm_getactual(0)->type]);
    }
}


void libfunc_totalarguments(){
    unsigned p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
    avm_memcellclear(&retval);
    if(!p_topsp){
        avm_error("totalarguments called outside of a function!");
        retval.type = nil_m;
    }
    else{
        retval.type = number_m;
        retval.data.numVal = avm_get_envvalue(p_topsp + AVM_NUMACTUALS_OFFSET);
    }
}
//TODO!!!!
void libfunc_input(){

}

void libfunc_objectmemberkeys(){
    
}

void libfunc_objecttotalmembers(){

}

void libfunc_objectcopy(){

}

void libfunc_argument(){

}

void libfunc_strtonum(){

}

void libfunc_sqrt(){
    avm_memcell *m = avm_getactual(0);
    assert(m);
    assert(m->type == number_m);
    m->data.numVal = sqrt(m->data.numVal);
}

void libfunc_cos(){
    avm_memcell *m = avm_getactual(0);
    assert(m);
    assert(m->type == number_m);
    m->data.numVal = cos(m->data.numVal);
}

void libfunc_sin(){
    avm_memcell *m = avm_getactual(0);
    assert(m);
    assert(m->type == number_m);
    m->data.numVal = sin(m->data.numVal);
}

void avm_registerlibfunc(char* id, library_func_t addr){
    assert(curr_lib_func < AVM_TOTAL_LIBFUNCS);
    libmap[curr_lib_func].id = malloc(sizeof(char*));
    strcpy(libmap[curr_lib_func].id, id);
    libmap[curr_lib_func].address = addr;
    curr_lib_func++;
}



double consts_getnumber(unsigned index){
    assert(index <= totalNumConsts);
    return numConsts[index];
}

char* consts_getstring(unsigned index){
    assert(index <= totalStringConsts);
    return stringConsts[index];

}

char* libfuncs_getused(unsigned index){
    assert(index < totalNamedLibFuncs);
    return namedLibFuncs[index];
}

userfunc* userfuncs_getfunc(unsigned index){
    assert(index <= totalUserFuncs);
    return &userFuncs[index];
}


void avm_warning(char* format){
    printf("AVM warning: %s\n",format);
}

void avm_error(char* err){
    fprintf(stderr,"AVM error: %s\n",err);
    executionFinished = 1;
}

void avm_dec_top(){
    if(!_top){
        avm_error("Stack overflow");
        executionFinished = 1;
    }
    else{
        --_top;
    }
}

void avm_push_envvalue(unsigned val){
    stack[_top].type = number_m;
    stack[_top].data.numVal = val;
    avm_dec_top();
}

void avm_callsaveenvironment(){
    avm_push_envvalue(totalActuals);
    assert(code[pc].opcode == call_v);
    avm_push_envvalue(pc + 1);
    avm_push_envvalue(_top + totalActuals + 2);
    avm_push_envvalue(topsp);
}

extern void avm_call_functor(avm_table* t){
    cx.type = string_m;
    cx.data.strVal = "()";
    avm_memcell* f = avm_tablegetelem(t, &cx);
    if(!f)
        avm_error("in calling table: no '()' element found!");
    else if(f->type == table_m)
        avm_call_functor(f->data.tableVal);
    else if(f->type == userfunc_a){
        avm_push_table_arg(t);
        avm_callsaveenvironment();
        pc = f->data.funcVal;
        assert(pc < AVM_ENDING_PC && code[pc].opcode == funcstart_v);
    }
    else
        avm_error("in calling table: illegal '()' element value!");
    
}

extern userfunc* avm_getfuncinfo(unsigned address){
    for(unsigned i = 0; i < totalUserFuncs; i++){
        if(userFuncs[i].address == address){
            return &userFuncs[i];
        }
    }
    return NULL;
}

//TODO

/*
typedef struct avm_table_bucket{
    avm_memcell key;
    avm_memcell value;
    struct avm_table_bucket* next;
} avm_table_bucket;

//from lectures
typedef struct avm_table{
    unsigned refCounter;
    avm_table_bucket* strIndexed[AVM_TABLE_HASHSIZE];
    avm_table_bucket* numIndexed[AVM_TABLE_HASHSIZE];
    //Bonus if implement other vars support
    unsigned total;
} avm_table;*/

avm_memcell* avm_tablegetelem(avm_table* table, avm_memcell* index){
    /*assert(table);
    assert(index);
    if(index->type == number_m){
        for(unsigned i = 0;i<AVM_TABLE_HASHSIZE;i++){
            if(table->numIndexed[i]->key == index->data.numVal){
                return table->numIndexed[i]->va
            }
        } 
    }*/

}

void avm_tablesetelem(avm_table* table, avm_memcell* index, avm_memcell* content){

}




//from lectures
extern void memclear_string(avm_memcell* m){
    assert(m->data.strVal);
    free(m->data.strVal);
}

//from lectures
extern void memclear_table(avm_memcell* m){
    assert(m->data.tableVal);
    avm_tabledecrefcounter(m->data.tableVal);
}


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
    return t;
}

memclear_func_t memclearFuncs[] = {
    0, //number
    memclear_string,
    0, //bool
    memclear_table,
    0, //userfunc
    0, //libfunc
    0, //nil
    0 //undef
};

//from lectures
void avm_memcellclear(avm_memcell* m){
    if(m->type != undef_m){
        memclear_func_t f = memclearFuncs[m->type];
        if(f)
            (*f)(m);
        m->type = undef_m;
    }
}

//from lectures
void avm_tablebucketsdestroy(avm_table_bucket** p){
    for(unsigned i = 0; i<AVM_TABLE_HASHSIZE; i++, p++){
        for(avm_table_bucket* b = *p; b;){
            avm_table_bucket* del  = b;
            b = b->next;
            avm_memcellclear(&del->key);
            avm_memcellclear(&del->value);
            free(del);
        }
        p[i] = (avm_table_bucket*) 0;
    }
}

//from lectures
void avm_tabledestroy(avm_table* t){
    avm_tablebucketsdestroy(t->strIndexed);
    avm_tablebucketsdestroy(t->numIndexed);
}


avm_memcell* avm_translate_operand(vmarg* arg, avm_memcell* reg){
    switch(arg->type){
        case global_a: return &stack[AVM_STACKSIZE-1-arg->val];
        case local_a: return &stack[topsp-arg->val];
        case formal_a: return &stack[topsp + AVM_STACKENV_SIZE + 1 + arg->val];
        case retval_a: return &retval;
        case number_a:{
            reg->type = number_m;
            reg->data.numVal = consts_getnumber(arg->val);
            return reg;
        }
        case string_a:{
            reg->type = string_m;
            reg->data.strVal = strdup(consts_getstring(arg->val));
            return reg;
        }
        case bool_a:{
            reg->type = bool_m;
            reg->data.boolVal = arg->val;
            return reg;
        }
        case nil_a: {reg->type = nil_m; return reg;}
        case userfunc_a:{
            reg->type = userfunc_m;
            reg->data.funcVal = arg->val;
            reg->data.funcVal = userfuncs_getfunc(arg->val)->address;
            return reg;
        }
        case libfunc_a:{
            reg->type = libfunc_m;
            reg->data.libfuncVal = libfuncs_getused(arg->val);
            return reg;
        }
        default: break; //What with labels???
    }

}

void execute_cycle(){
    if(executionFinished)
        return;
    else if(pc == AVM_ENDING_PC){
        executionFinished = 1;
        return;
    }
    else{
        assert(pc<AVM_ENDING_PC);
        instruction* instr = code + pc;
        assert(instr->opcode >= 0 && instr->opcode <= AVM_MAX_INSTRUCTIONS);
        if(instr->srcLine)
            currLine = instr->srcLine;
        unsigned oldPC = pc;
        (*executeFuncs[instr->opcode])(instr);
        if(pc == oldPC)
            ++pc;
    }
}

//TODO
void execute_uminus(instruction *instr){

}

void execute_and(instruction *instr){

}

void execute_or(instruction *instr){

}

void execute_not(instruction *instr){

}

void execute_nop(instruction *instr){
 //Do nothing???
}

void execute_assign(instruction* instr){
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*) 0);
    avm_memcell* rv = avm_translate_operand(&instr->arg1, &ax);
    assert(lv && (&stack[AVM_STACKSIZE/*???*/ -1] >= lv && lv > &stack[_top] || lv == &retval));
    assert(rv);
    avm_assign(lv,rv);
}

void avm_assign(avm_memcell* lv, avm_memcell* rv){
    if(lv == rv)
        return;
    if(lv->type == table_m && rv->type == table_m && lv->data.tableVal == rv->data.tableVal)
        return;
    if(rv->type == undef_m)
        avm_warning("assigning from 'undef' content!");
    avm_memcellclear(lv);
    memcpy(lv,rv, sizeof(avm_memcell));
    if(lv->type == string_m)
        lv->data.strVal = strdup(rv->data.strVal);
    else if(lv->type == table_m)
        avm_tableincrefcounter(lv->data.tableVal);
}

void execute_call(instruction *instr){
    avm_memcell* func = avm_translate_operand(&instr->arg1, &ax);
    assert(func);
    switch(func->type){
        case userfunc_m:{
            avm_callsaveenvironment();
            pc = func->data.funcVal;
            assert(pc<AVM_ENDING_PC);
            assert(code[pc].opcode == funcstart_v);
            break;
        }
        case string_m:{ avm_callibfunc(func->data.strVal); break;}
        case libfunc_m: {avm_callibfunc(func->data.libfuncVal); break;}
        case table_m: {avm_call_functor(func->data.tableVal); break;}
        default:{
            char* s = avm_tostring(func);
            avm_error("call: cannot bind to function!");
            free(s);
            executionFinished = 1;
        }

    }
}

void execute_funcenter(instruction* instr){
    avm_memcell* func = avm_translate_operand(&instr->result, &ax);
    assert(func);
    assert(pc == func->data.funcVal);

    totalActuals = 0;
    userfunc* funcInfo = avm_getfuncinfo(pc);
    topsp = _top;
    _top = _top - funcInfo->localSize;
}

unsigned avm_get_envvalue(unsigned i){
    assert(stack[i].type = number_m);
    unsigned val = (unsigned)stack[i].data.numVal;
    assert(stack[i].data.numVal == ((double)val));
    return val;
}

void execute_funcexit(instruction* unused){
    unsigned oldTop = _top;
    _top = avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);
    pc = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);
    topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
    while(++oldTop <= _top)
        avm_memcellclear(&stack[oldTop]);
}

//Not very efficient when we have like hundrends of libfuncs. But for 12 it works fine
library_func_t avm_getlibraryfunc(char *id){
    unsigned int tmp = 0;
    while(tmp < AVM_TOTAL_LIBFUNCS){
        if(strcmp(libmap[tmp].id, id) == 0){
            return libmap[tmp].address;
        }
        tmp++;
    }
    avm_error("Library function not found!!");
}

void avm_callibfunc(char* id){
    library_func_t f = avm_getlibraryfunc(id);
    if(!f){
        avm_error("unsupported lib func called!");
        executionFinished = 1;
    }
    else{
        avm_callsaveenvironment();
        topsp = _top;
        totalActuals = 0;
        (*f)();
        if(!executionFinished)
            execute_funcexit((instruction*)0);
    }
}

unsigned avm_totalactuals(){
    return avm_get_envvalue(topsp + AVM_NUMACTUALS_OFFSET);
}

avm_memcell* avm_getactual(unsigned i){
    assert(i<avm_totalactuals());
    return &stack[topsp + AVM_STACKENV_SIZE + 1 + i];
}


extern void avm_push_table_arg(avm_table* t){
    stack[_top].type = table_m;
    avm_tableincrefcounter(stack[_top].data.tableVal = t);
    ++totalActuals;
    avm_dec_top();
}

void execute_pusharg(instruction* instr){
    avm_memcell* arg = avm_translate_operand(&instr->arg1, &ax);
    assert(arg);
    avm_assign(&stack[_top], arg);
    ++totalActuals;
    avm_dec_top();
}


void execute_arithmetic(instruction* instr){
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

    assert(lv && (&stack[AVM_STACKSIZE/*????*/ - 1] >= lv && lv > &stack[_top] || lv == &retval));
    assert(rv1 && rv2);
    if(rv1->type != number_m || rv2->type != number_m){
        avm_error("not a number in arithmetic!");
        executionFinished = 1;
    }
    else{
        arithmetic_func_t op = arithmeticFuncs[instr->opcode - add_v];
        avm_memcellclear(lv);
        lv->type = number_m;
        lv->data.numVal = (*op)(rv1->data.numVal, rv2->data.numVal);
    }
}

void execute_cmp(instruction* instr){
    avm_memcell* lv; //= avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

   //assert(lv && (&stack[AVM_STACKSIZE/*????*/ - 1] >= lv && lv > &stack[_top] || lv == &retval));
    assert(rv1 && rv2);
   /* if(rv1->type != number_m || rv2->type != number_m){
        avm_error("not a number in comparizon!");
        executionFinished = 1;
    }
    else{*/
        cmp_func op = comparizonFuncs[instr->opcode - if_lesseq_v];
        avm_memcellclear(lv);
        lv->type = bool_m;
        lv->data.boolVal = (*op)(rv1->data.numVal, rv2->data.numVal);
   // }

}

void execute_jmp(instruction* instr){
    assert(instr->result.type == label_a);
    if(!executionFinished)
        pc = instr->result.val;
}

void execute_jeq(instruction *instr){
    assert(instr->result.type == label_a);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    unsigned char result = 0;

    if(rv1->type == undef_m || rv2->type == undef_m)
        avm_error("undef involved in equality!");
    else if(rv1->type == bool_m || rv2->type == bool_m)
        result = (avm_tobool(rv1) == avm_tobool(rv2));
    else if(rv1->type == nil_m || rv2->type == nil_m)
        result = rv1->type == nil_m && rv2->type == nil_m;
    else if(rv1->type != rv2->type)
        avm_error("illegal comparizon types");
    else{
        //Equality check with dispatching
    }
    if(!executionFinished && result)
        pc = instr->result.val;
}

void execute_jne(instruction *instr){
    assert(instr->result.type == label_a);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    unsigned char result = 0;

    if(rv1->type == undef_m || rv2->type == undef_m)
        avm_error("undef involved in equality!");
    else if(rv1->type == bool_m || rv2->type == bool_m)
        result = (avm_tobool(rv1) == avm_tobool(rv2));
    else if(rv1->type == nil_m || rv2->type == nil_m)
        result = rv1->type == nil_m && rv2->type == nil_m;
    else if(rv1->type != rv2->type)
        avm_error("illegal comparizon types");
    else{
        //Equality check with dispatching
    }
    if(!executionFinished && result)
        pc = instr->result.val;
}


void execute_newtable(instruction* instr){
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    assert(lv && (&stack[AVM_STACKSIZE/*????*/ - 1] >= lv && lv > &stack[_top] || lv == &retval));
    avm_memcellclear(lv);
    lv->type = table_m;
    lv->data.tableVal = avm_tablenew();
    avm_tableincrefcounter(lv->data.tableVal);
}

void execute_tablegetelem(instruction* instr){
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* t = avm_translate_operand(&instr->arg1,(avm_memcell*)0);
    avm_memcell* i = avm_translate_operand(&instr->arg2, &ax);

    assert(lv && (&stack[AVM_STACKSIZE/*????*/ - 1] >= lv && lv > &stack[_top] || lv == &retval));
    assert(t && &stack[AVM_STACKSIZE/*????*/ - 1] >= t && t > &stack[_top]);
    assert(i);

    avm_memcellclear(lv);
    lv->type = nil_m;
    
    if(t->type != table_m)
        avm_error("Illegal use of type as table!");
    else{
        avm_memcell* content = avm_tablegetelem(t->data.tableVal, i);
        if(content)
            avm_assign(lv,content);
        else{
            char* ts = avm_tostring(t);
            char* is = avm_tostring(i);
            avm_warning("ts[is] not found");
            free(ts);
            free(is);
        }
    }
}

void execute_tablesetelem(instruction* instr){
    avm_memcell* t = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* i = avm_translate_operand(&instr->arg1,&ax);
    avm_memcell* c = avm_translate_operand(&instr->arg2, &bx);

    assert(t && &stack[AVM_STACKSIZE - 1] >= t && t > &stack[_top] );
    assert(i && c);

    if(t->type != table_m)
        avm_error("illegal use as table!");
    else
        avm_tablesetelem(t->data.tableVal, i, c);

}

void print_curr_instr(){
    printf("%s   ",vmopcode_to_string[code[pc].opcode]);
    if(code[pc].result.type != nil_a)
        printf("%u  ", code[pc].result.val);
    if(code[pc].arg1.type != nil_a)
        printf("%u  ", code[pc].arg1.val);
    if(code[pc].arg2.type != nil_a)
        printf("%u  ", code[pc].arg2.val);
    printf("[line: %d]",code[pc].srcLine);
    printf("\n");

}