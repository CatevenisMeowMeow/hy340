#include "quads.h"


//Global vars from lectures
double* numConsts;
unsigned totalNumConsts = 0;
char** stringConsts;
unsigned totalStringConsts = 0;
char** namedLibFuncs;
unsigned totalNamedLibFuncs = 0;
//userfunc* userFuncs;
unsigned totalUserFuncs = 0;


//from lectures
unsigned consts_newstring(char* s);
unsigned consts_newnumber(double n);
unsigned libfuncs_newused(char* s);
unsigned userfuncs_newfunc(symrec* sym);




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



void make_operand(expr* e, vmarg* arg);



//from lectures
void make_operand(expr* e, vmarg* arg){
    switch(e->type){
        case var_e:
        case tableitem_e:
        case arithexpr_e:
        case boolexpr_e:
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
        case nil_e: arg->type = nil_a; break;
        case programfunc_e:{
            arg->type = userfunc_a;
            arg->val = e->sym->iaddress;
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

unsigned totalInstructions = 0;
unsigned int currInstruction = 0;
instruction* instructions = (instruction*) 0;


#define EXPAND_INSRUCTION_SIZE 1024
#define CURR_INSTRUCTION_SIZE   (totalInstructions*sizeof(instruction))
#define NEW_INSTRUCTION_SIZE    (EXPAND_INSRUCTION_SIZE*sizeof(instruction) + CURR_INSTRUCTION_SIZE)


unsigned nextinstructionlabel(){
    return currInstruction;
}

void extend_instructions_array(){
    assert(currInstruction == totalInstructions);
    instruction *i = (instruction*)malloc(NEW_INSTRUCTION_SIZE);
    if(instructions){
        memcpy(i, instructions, CURR_INSTRUCTION_SIZE);
        free(instructions);
    }
    instructions = i;
    totalInstructions = totalInstructions + EXPAND_INSRUCTION_SIZE;
}

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
typedef struct incomplete_jump{
    unsigned instrNo;
    unsigned iaddress;
    struct incomplete_jump* next;
} incomplete_jump;

//from lectures
incomplete_jump* ij_head = (incomplete_jump*) 0;
unsigned ij_total = 0;

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
    totalInstructions++;
}

//from lectures
void patch_incomplete_jumps(){
    incomplete_jump* tmp = ij_head;
    while(tmp != NULL){
        if(tmp->iaddress == currQuad){
            instructions[tmp->instrNo].result.val = totalInstructions;
            instructions[tmp->instrNo].result.type = label_a; //Is this the correct type??
        }
        else{
            instructions[tmp->instrNo].result.val = quads[tmp->iaddress].taddress;
            instructions[tmp->instrNo].result.type = label_a; //Is this the correct type??
        }
        tmp = tmp->next;
    }
}

//from lectures
void generate(vmopcode op, quad* q){
    instruction *t = (struct instruction*)malloc(sizeof(struct instruction));
    t->opcode = op;
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
void generate_NOP(quad *q){ instruction *t = (instruction*)malloc(sizeof(instruction)); t->opcode = nop_v;  emit_instruction(t);}
//TODO........

