#include "quads.h"


//This one stores functions and symbols. Requested from lectures for FUNCSTART
struct functionstack {
    struct functionstack* next;
    symrec* top;
};

struct functionstack *funcstack = NULL;


//from lectures
typedef enum vmopcode{
    assign_v, add_v, sub_v,
    mul_v, div_v, mod_v,
    uminus_v, and_v, or_v,
    not_v, if_eq_v, if_noteq_v,
    if_lesseq_v, if_greatereq_v, if_less_v,
    if_greater_v, call_v, pusharg_v,
    funcstart_v,
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


const char* vmopcode_to_string[] = {
    "assign_v", "add_v", "sub_v", "mul_v", "div_v", "mod_v",
    "uminus_v", "and_v", "or_v", "not_v", "if_eq_v", "if_noteq_v",
    "if_lesseq_v", "if_greatereq_v", "if_less_v", "if_greater_v",
    "call_v", "pusharg_v","funcstart_v", "funcend_v",
    "tablecreate_v", "jump_v", "nop_v", "tablegetelem_v", "tablesetelem_v"
};

const char* vmarg_type_to_string[] = {
    "label_a", "global_a", "formal_a", "local_a", "number_a",
    "string_a", "bool_a", "nil_a", "userfunc_a", "libfunc_a", "retval_a"
};

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


unsigned totalInstructions = 0;
unsigned int currInstruction = 0;
instruction* instructions = (instruction*) 0;


#define EXPAND_INSTRUCTION_SIZE 1024
#define CURR_INSTRUCTION_SIZE   (totalInstructions*sizeof(instruction))
#define NEW_INSTRUCTION_SIZE    (EXPAND_INSTRUCTION_SIZE*sizeof(instruction) + CURR_INSTRUCTION_SIZE)



typedef struct userfunc{
    unsigned address;
    unsigned localSize;
    char* id;
} userfunc;


//from lectures
typedef struct incomplete_jump{
    unsigned instrNo;
    unsigned iaddress;
    struct incomplete_jump* next;
} incomplete_jump;


//from lectures
incomplete_jump* ij_head = (incomplete_jump*) 0;
unsigned ij_total = 0;

//Global vars from lectures
double* numConsts;
unsigned totalNumConsts = 0;
char** stringConsts;
unsigned totalStringConsts = 0;
char** namedLibFuncs;
unsigned totalNamedLibFuncs = 0;
userfunc* userFuncs;
unsigned totalUserFuncs = 0;


//from lectures. Prodhlwsh
unsigned consts_newstring(char* s);
unsigned consts_newnumber(double n);
unsigned libfuncs_newused(char* s);
unsigned userfuncs_newfunc(symrec* sym);
void make_numberoperand(vmarg* arg, double val);
void make_booloperand(vmarg* arg, unsigned val);
void make_retvaloperand(vmarg* arg);
void make_operand(expr* e, vmarg* arg);
void resetoperand(vmarg *a);
unsigned nextinstructionlabel();
void extend_instructions_array();
void emit_instruction(instruction *t);
void add_incomplete_jump(unsigned instrNo, unsigned iaddress);
void patch_incomplete_jumps();

void generate(vmopcode op, quad* q);
void generate_all();

void patchlabel_instuctions(unsigned quadNo, unsigned label);

extern void generate_ADD(quad *q);
extern void generate_SUB(quad *q);
extern void generate_MUL(quad *q);
extern void generate_DIV(quad *q);
extern void generate_MOD(quad *q);
extern void generate_NEWTABLE(quad *q);
extern void generate_TABLEGETELEM(quad *q);
extern void generate_TABLESETELEM(quad *q);
extern void generate_ASSIGN(quad *q);
extern void generate_NOP(quad *q);
extern void generate_relational (vmopcode op, quad *q);
extern void generate_JUMP (quad *q);
extern void generate_IF_EQ (quad *q);
extern void generate_IF_NOTEQ(quad *q);
extern void generate_IF_GREATER (quad *q);
extern void generate_IF_GREATEREQ(quad *q);
extern void generate_IF_LESS (quad *q);
extern void generate_IF_LESSEQ (quad *q);
extern void generate_NOT (quad *q);
extern void generate_OR (quad *q);
extern void generate_AND (quad *q);
extern void generate_PARAM(quad *q);
extern void generate_CALL(quad *q);
extern void generate_GETRETVAL(quad *q);
extern void generate_FUNCSTART(quad *q);
extern void generate_RETURN(quad *q);
extern void generate_FUNCEND(quad *q);

void print_instructions();
void print_binary_instructions();

void pop_functionstack();
void push_functionstack(symrec *s);
symrec* top_functionstack();

//functions for returnList at function symbols
void append_returnlist(symrec* f,unsigned label){
    assert(f);
    returnList *list = malloc(sizeof(struct returnList));
    list->label = label;
    
    if(f->returnlist == NULL){
        f->returnlist = list;
        f->returnlist->next = NULL;
        return;
    }
    
    list->next = f->returnlist;
    f->returnlist = list;
    
}

void backpatch_returnlist(symrec *f, unsigned label){
    assert(f);
    returnList *tmp = f->returnlist;
    while(tmp != NULL){
        patchlabel_instuctions(tmp->label,label);
        tmp = tmp->next;
    }
}

//Functions for functionstack
void push_functionstack(symrec *s) {
    assert(s);
    struct functionstack* new_node = (struct functionstack*)malloc(sizeof(struct functionstack));
    assert(new_node);
    new_node->top = s; 
    if(funcstack == NULL){
        funcstack = new_node;
        funcstack->next = NULL;
        return;
    }
    new_node->next = funcstack;
    funcstack = new_node;
}

void pop_functionstack() {
    assert(funcstack);
    struct functionstack* temp = funcstack;
    funcstack = funcstack->next;
    free(temp);
}

symrec* top_functionstack(){
    assert(funcstack);
    assert(funcstack->top);
    return funcstack->top;
}


//Functions for global arrays
unsigned consts_newstring(char* s){
    char** temp = realloc(stringConsts, (totalStringConsts + 1) * sizeof(char *));
    assert(temp);
    stringConsts = temp;
    stringConsts[totalStringConsts] = malloc(strlen(s) + 1);
    assert(stringConsts[totalStringConsts]);
    //strcpy(stringConsts[totalStringConsts], s);
    stringConsts[totalStringConsts] = strdup(s);
    return totalStringConsts++;
}

unsigned consts_newnumber(double n){
    double* temp = realloc(numConsts, (totalNumConsts + 1) * sizeof(double));
    assert(temp);
    numConsts = temp;
    numConsts[totalNumConsts] = n;
    return totalNumConsts++;

}
//similar to consts_newstring...
unsigned libfuncs_newused(char* s){
    char** temp = realloc(namedLibFuncs, (totalNamedLibFuncs + 1) * sizeof(char *));
    assert(temp);
    namedLibFuncs = temp;
    namedLibFuncs[totalNamedLibFuncs] = malloc(strlen(s) + 1);
    assert(namedLibFuncs[totalNamedLibFuncs]);
    strcpy(namedLibFuncs[totalNamedLibFuncs], s);
    return totalNamedLibFuncs++;

}

unsigned userfuncs_newfunc(symrec* sym){
    assert(sym);
    userfunc* temp = realloc(userFuncs, (totalUserFuncs + 1) * sizeof(struct userfunc));
    assert(temp);
    userFuncs = temp;
    userFuncs[totalUserFuncs].id = malloc(sizeof(strlen(sym->name) + 1));
    assert(userFuncs[totalUserFuncs].id);
    strcpy(userFuncs[totalUserFuncs].id, sym->name);
    userFuncs[totalUserFuncs].address = sym->taddress;
    userFuncs[totalUserFuncs].localSize = sym->totalLocals;
    return totalUserFuncs++;
}




void resetoperand(vmarg *a){
    a = (vmarg*)0;
}

//from lectures 
void make_operand(expr* e, vmarg* arg){
    if(e == NULL){
        arg->type = nil_a;
        arg->val = 0;
        return;
    }
    switch(e->type){
        case var_e:{
            assert(e->sym);
            arg->val = e->sym->offset;
            switch(e->sym->space){
                case programvar: arg->type = global_a; break;
                case functionlocal: arg->type = local_a; break;
                case formalarg: arg->type = formal_a; break;
                default: assert(0);
            }
            break;
        }
        case tableitem_e:{
            assert(e->sym);
            arg->val = e->sym->offset;
            switch(e->sym->space){
                case programvar: arg->type = global_a; break;
                case functionlocal: arg->type = local_a; break;
                case formalarg: arg->type = formal_a; break;
                default: assert(0);
            }
            break;
        }
        case arithexpr_e:{
           assert(e->sym);
            arg->val = e->sym->offset;
            switch(e->sym->space){
                case programvar: arg->type = global_a; break;
                case functionlocal: arg->type = local_a; break;
                case formalarg: arg->type = formal_a; break;
                default: assert(0);
            }
            break;
        }
        case assignexpr_e:{
            assert(e->sym);
            arg->val = e->sym->offset;
            switch(e->sym->space){
                case programvar: arg->type = global_a; break;
                case functionlocal: arg->type = local_a; break;
                case formalarg: arg->type = formal_a; break;
                default: assert(0);
            }
            break;
        }
        case boolexpr_e:{
            assert(e->sym);
            arg->val = e->sym->offset;
            switch(e->sym->space){
                case programvar: arg->type = global_a; break;
                case functionlocal: arg->type = local_a; break;
                case formalarg: arg->type = formal_a; break;
                default: assert(0);
            }
            break;
        }
        case newtable_e:{
            assert(e->sym);
            arg->val = e->sym->offset;
            switch(e->sym->space){
                case programvar: arg->type = global_a; break;
                case functionlocal: arg->type = local_a; break;
                case formalarg: arg->type = formal_a; break;
                default: assert(0);
            }
            break;
        }
        case constbool_e:{
            arg->val = e->boolConst;
            arg->type = bool_a; break;
        }
        case conststring_e:{
            arg->val = consts_newstring(e->strConst);
            arg->type = string_a; break;
        }
        case constnum_e:{
            arg->val = consts_newnumber(e->numConst);
            arg->type = number_a; break;
        }
        case nil_e: {arg->type = nil_a; break;}
        case programfunc_e:{
            arg->type = userfunc_a;
            arg->val = userfuncs_newfunc(e->sym);
            break;
        }
        case libraryfunc_e:{
            arg->type = libfunc_a;
            arg->val = libfuncs_newused(e->sym->name);
            break;
        }
        default: assert(0);
    }
}

//from lectures
void make_numberoperand(vmarg* arg, double val){
    arg->val = consts_newnumber(val);
    arg->type = number_a;
}

void make_booloperand(vmarg* arg, unsigned val){
    arg->val = val;
    arg->type = bool_a;
}

void make_retvaloperand(vmarg* arg){
    arg->type = retval_a;
}


unsigned nextinstructionlabel(){
    return currInstruction;
}


void extend_instructions_array(){
    assert(currInstruction == totalInstructions);
    instruction *i = malloc(NEW_INSTRUCTION_SIZE);
    if(instructions){
        memcpy(i, instructions, CURR_INSTRUCTION_SIZE);
        free(instructions);
    }
    instructions = i;
    totalInstructions = totalInstructions + EXPAND_INSTRUCTION_SIZE;
}
//Similar to emit with instructions
void emit_instruction(instruction *t){
    if(currInstruction == totalInstructions){
        extend_instructions_array();
    }
    instruction* i = instructions + currInstruction++;
    i->arg1 = t->arg1;
    i->arg2 = t->arg2;
    i->opcode = t->opcode;
    i->result = t->result;
    i->srcLine = t->srcLine;
}

//from lectures
void add_incomplete_jump(unsigned instrNo, unsigned iaddress){
    incomplete_jump* j = (struct incomplete_jump*)malloc(sizeof(struct incomplete_jump));
    j->instrNo = instrNo;
    j->iaddress = iaddress;
    if(ij_head == (incomplete_jump*) 0){
        ij_head = j;
        ij_head->next = NULL;
        return;
    }
    j->next = ij_head;
    ij_head = j;
    ij_total++;
}

//from lectures
void patch_incomplete_jumps(){
    incomplete_jump* tmp = ij_head;
    while(tmp != NULL){
        if(tmp->iaddress == currQuad){
            instructions[tmp->instrNo].result.val = nextinstructionlabel();
        }
        else{
            instructions[tmp->instrNo].result.val = quads[tmp->iaddress].taddress;
        }
        tmp = tmp->next;
    }
}

//similar to patchlabel for quads
void patchlabel_instuctions(unsigned instrNo, unsigned label){
    assert(instrNo < currInstruction ); 
    instructions[instrNo].result.val = label;
}

//from lectures
void generate(vmopcode op, quad* q){
    instruction *t = (struct instruction*)malloc(sizeof(struct instruction));
    t->opcode = op;
    t->srcLine = q->line;
    make_operand(q->arg1, &(t->arg1));
    make_operand(q->arg2, &(t->arg2));
    make_operand(q->result, &(t->result));
    q->taddress = nextinstructionlabel();
    emit_instruction(t);
}

void generate_ADD(quad *q){ generate(add_v, q); }
void generate_SUB(quad *q){ generate(sub_v, q); }
void generate_MUL(quad *q){ generate(mul_v, q); }
void generate_DIV(quad *q){ generate(div_v, q); }
void generate_MOD(quad *q){ generate(mod_v, q); }
void generate_NEWTABLE(quad *q){ generate(tablecreate_v, q); }
void generate_TABLEGETELEM(quad *q){ generate(tablegetelem_v, q); }
void generate_TABLESETELEM(quad *q){ generate(tablesetelem_v, q); }
void generate_ASSIGN(quad *q){ generate(assign_v, q); }
void generate_NOP(quad *q){ instruction *t = (instruction*)malloc(sizeof(instruction)); t->opcode = nop_v; t->srcLine = q->line; emit_instruction(t);}

void generate_relational(vmopcode op, quad *q){
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = op;
    t->srcLine = q->line;
    make_operand(q->arg1, &t->arg1);
    make_operand(q->arg2, &t->arg2);
    t->result.type = label_a;
    if(q->label < currQuad){
        t->result.val = quads[q->label].label;
    }
    else{
        add_incomplete_jump(nextinstructionlabel(), q->label);
    }
    q->taddress = nextinstructionlabel();
    emit_instruction(t);
}

void generate_JUMP(quad *q){ 
    generate_relational(jump_v, q);
   /* instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = jump_v;
    t->srcLine = q->line;
    t->result.type = label_a;
    t->result.val = q->label;
    t->arg1.type = t->arg2.type = nil_a;
    t->arg1.val = t->arg2.val = 0;
    emit_instruction(t);*/

}
void generate_IF_EQ(quad *q){ generate_relational(if_eq_v, q);}
void generate_IF_NOTEQ(quad *q){ generate_relational(if_noteq_v, q);}
void generate_IF_GREATER(quad *q){ generate_relational(if_greater_v, q);}
void generate_IF_GREATEREQ(quad *q){ generate_relational(if_greatereq_v, q);}
void generate_IF_LESS(quad *q){ generate_relational(if_less_v, q);}
void generate_IF_LESSEQ(quad *q){ generate_relational(if_lesseq_v, q);}

void generate_NOT(quad *q){
    q->taddress = nextinstructionlabel();
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = if_eq_v;
    t->srcLine = q->line;
    make_operand(q->arg1, &t->arg1);
    make_booloperand(&t->arg2, 0);
    t->result.type = label_a;
    t->result.val = nextinstructionlabel() + 3;
    emit_instruction(t);
    
    t->opcode = assign_v;
    make_booloperand(&t->arg1, 0);
    resetoperand(&t->arg2);
    make_operand(q->result, &t->result);
    emit_instruction(t);

    t->opcode = jump_v;
    resetoperand(&t->arg1);
    resetoperand(&t->arg2);
    t->result.type = label_a;
    t->result.val = nextinstructionlabel() + 2;
    emit_instruction(t);

    t->opcode = assign_v;
    make_booloperand(&t->arg1, 1);
    resetoperand(&t->arg2);
    make_operand(q->result, &t->result);
    emit_instruction(t);

}

void generate_OR(quad *q){
    q->taddress = nextinstructionlabel();
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = if_eq_v;
    t->srcLine = q->line;
    make_operand(q->arg1, &t->arg1);
    make_booloperand(&t->arg2, 1);
    t->result.type = label_a;
    t->result.val = nextinstructionlabel() + 4;
    emit_instruction(t);

    make_operand(q->arg2, &t->arg1);
    t->result.val = nextinstructionlabel() + 3;
    emit_instruction(t);

    t->opcode = assign_v;
    make_booloperand(&t->arg1, 0);
    resetoperand(&t->arg2);
    make_operand(q->result, &t->result);
    emit_instruction(t);

    t->opcode = jump_v;
    resetoperand(&t->arg1);
    resetoperand(&t->arg2);
    t->result.type = label_a;
    t->result.val = nextinstructionlabel() + 2;
    emit_instruction(t);

    t->opcode = assign_v;
    make_booloperand(&t->arg1, 1);
    resetoperand(&t->arg2);
    make_operand(q->result, &t->result);
    emit_instruction(t);
}

void generate_AND(quad *q){
    q->taddress = nextinstructionlabel();
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = if_eq_v;
    t->srcLine = q->line;
    make_operand(q->arg1, &t->arg1);
    make_booloperand(&t->arg2, 0);
    t->result.type = label_a;
    t->result.val = nextinstructionlabel() + 4;
    emit_instruction(t);

    make_operand(q->arg2, &t->arg1);
    t->result.val = nextinstructionlabel() + 3;
    emit_instruction(t);

    t->opcode = assign_v;
    make_booloperand(&t->arg1, 1);
    resetoperand(&t->arg2);
    make_operand(q->result, &t->result);
    emit_instruction(t);

    t->opcode = jump_v;
    resetoperand(&t->arg1);
    resetoperand(&t->arg2);
    t->result.type = label_a;
    t->result.val = nextinstructionlabel() + 2;
    emit_instruction(t);

    t->opcode = assign_v;
    make_booloperand(&t->arg1, 0);
    resetoperand(&t->arg2);
    make_operand(q->result, &t->result);
    emit_instruction(t);
}

void generate_PARAM(quad *q){
    q->taddress = nextinstructionlabel();
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = pusharg_v;
     t->srcLine = q->line;
    t->result.type = nil_a;
    t->result.val = 0;
    make_operand(q->arg1, &t->arg1);
    //What if arg2 is not used??? Make arg2 as nil????????
    t->arg2.type = nil_a;
    t->arg2.val = 0;
    emit_instruction(t);
}

void generate_CALL(quad *q){
    q->taddress = nextinstructionlabel();
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = call_v;
    t->srcLine = q->line;
    t->result.type = nil_a;
    t->result.val = 0;
    make_operand(q->arg1, &t->arg1);
    //What if arg2 is not used??? Make arg2 as nil????????
    t->arg2.type = nil_a;
    t->arg2.val = 0;
    emit_instruction(t);
}

void generate_GETRETVAL(quad *q){
    q->taddress = nextinstructionlabel();
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = assign_v;
    t->srcLine = q->line;
    make_operand(q->result, &t->result);
    make_retvaloperand(&t->arg1);
    //What if arg2 is not used??? Make arg2 as nil????????
    t->arg2.type = nil_a;
    t->arg2.val = 0;
    emit_instruction(t);
}

void generate_FUNCSTART(quad *q){
    symrec *f = q->result->sym;
    f->taddress = nextinstructionlabel(); 
    q->taddress = nextinstructionlabel();
    
    push_functionstack(f);
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = funcstart_v;
    t->srcLine = q->line;
    make_operand(q->result, &t->result);
    //What if arg2, arg1 is not used??? Make arg2 as nil????????
    t->arg1.type = nil_a;
    t->arg1.val = 0;
    t->arg2.type = nil_a;
    t->arg2.val = 0;
    emit_instruction(t);

}

void generate_RETURN(quad *q){
    q->taddress = nextinstructionlabel();
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = assign_v;
    t->srcLine = q->line;
    make_retvaloperand(&t->result);
    make_operand(q->result,&t->arg1); //?????q->arg1 is NULL at return stmts. Return value is at result field at quad. NOT at result
    //What if arg2 is not used??? Make arg2 as nil????????
    t->arg2.type = nil_a;
    t->arg2.val = 0;
    emit_instruction(t);

    symrec *f = top_functionstack();
    append_returnlist(f, nextinstructionlabel());
   
    t->opcode = jump_v;
    resetoperand(&t->arg1);
    resetoperand(&t->arg2);
    t->result.type = label_a;
    emit_instruction(t);


}

void generate_FUNCEND(quad *q){
   
    symrec *f = top_functionstack();
    pop_functionstack();
    backpatch_returnlist(f, nextinstructionlabel());

    q->taddress = nextinstructionlabel();
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = funcend_v;
    t->srcLine = q->line;
    make_operand(q->result, &t->result);
    //What if arg2,arg1 is not used??? Make arg2 as nil????????
    t->arg1.type = nil_a;
    t->arg1.val = 0;
    t->arg2.type = nil_a;
    t->arg2.val = 0;
    emit_instruction(t);

}


typedef void (*generator_func_t) (quad*);

generator_func_t generators[] = {
    generate_ADD,
    generate_SUB,
    generate_MUL,
    generate_DIV,
    generate_MOD,
    generate_NEWTABLE,
    generate_TABLEGETELEM,
    generate_TABLESETELEM,
    generate_ASSIGN,
    generate_NOP,
    generate_JUMP,
    generate_IF_EQ,
    generate_IF_NOTEQ,
    generate_IF_GREATER,
    generate_IF_GREATEREQ,
    generate_IF_LESS,
    generate_IF_LESSEQ,
    generate_NOT,
    generate_OR,
    generate_AND,
    generate_PARAM,
    generate_CALL,
    generate_GETRETVAL,
    generate_FUNCSTART,
    generate_RETURN,
    generate_FUNCEND
};

void generate_all(){
    for(unsigned i = 0; i<currQuad; ++i)
        (*generators[quads[i].op])(quads + i);
    patch_incomplete_jumps();
}


void print_instructions() {
    printf("Instructions:\n");
    printf("%-10s%-15s%-20s%-20s%-20s\n", "Index", "Opcode", "Result", "Arg1", "Arg2");
    printf("------------------------------------------------------------------------\n");

    for (unsigned int i = 0; i < currInstruction; i++) {
        instruction inst = instructions[i];
        printf("%-10d%-15s", i, vmopcode_to_string[inst.opcode]);
        if(inst.result.type != nil_a)
            printf("%-20s", vmarg_type_to_string[inst.result.type]);
        if(inst.arg1.type != nil_a)
            printf("%-20s", vmarg_type_to_string[inst.arg1.type]);
        if(inst.arg2.type != nil_a)
            printf("%-20s", vmarg_type_to_string[inst.arg2.type]);

        printf("\n");
    }
    
}


void print_instructions_vals() {
    printf("Instructions:\n");
    printf("%-10s%-15s%-20s%-20s%-20s\n", "Index", "Opcode", "Result", "Arg1", "Arg2");
    printf("------------------------------------------------------------------------\n");

    for (unsigned int i = 0; i < currInstruction; i++) {
        instruction inst = instructions[i];
        printf("%-10d%-15s", i, vmopcode_to_string[inst.opcode]);
        if(inst.result.type != nil_a)
            printf("%-20u", inst.result.val);
        if(inst.arg1.type != nil_a)
            printf("%-20u", inst.arg1.val);
        if(inst.arg2.type != nil_a)
            printf("%-20u", inst.arg2.val);

        printf("\n");
    }
}



//Binary output
void print_binary_instructions(FILE* file) {
    for (unsigned int i = 0; i < currInstruction; i++) {
        instruction inst = instructions[i];

        // Write the command code
        fwrite(&inst.opcode, sizeof(vmopcode), 1, file);

        // write vmarg
        fwrite(&inst.result.type, sizeof(vmarg_t), 1, file);
        fwrite(&inst.result.val, sizeof(unsigned), 1, file);

        fwrite(&inst.arg1.type, sizeof(vmarg_t), 1, file);
        fwrite(&inst.arg1.val, sizeof(unsigned), 1, file);

        fwrite(&inst.arg2.type, sizeof(vmarg_t), 1, file);
        fwrite(&inst.arg2.val, sizeof(unsigned), 1, file);

        // Write the source code line number if necessary
        fwrite(&inst.srcLine, sizeof(unsigned), 1, file);
    }
}