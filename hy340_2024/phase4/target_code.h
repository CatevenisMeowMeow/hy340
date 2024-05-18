#include "quads.h"


//This one stores functions and symbols. Requested from lectures for FUNCSTART
struct functionstack {
    struct functionstack* next;
    symrec* top;
};

struct functionstack *funcstack = NULL;


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


unsigned totalInstructions = 0;
unsigned int currInstruction = 0;
instruction* instructions = (instruction*) 0;


#define EXPAND_INSRUCTION_SIZE 1024
#define CURR_INSTRUCTION_SIZE   (totalInstructions*sizeof(instruction))
#define NEW_INSTRUCTION_SIZE    (EXPAND_INSRUCTION_SIZE*sizeof(instruction) + CURR_INSTRUCTION_SIZE)



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
void generate_ADD(quad *q);
void generate_SUB(quad *q);
void generate_MUL(quad *q);
void generate_DIV(quad *q);
void generate_MOD(quad *q);
void generate_NEWTABLE(quad *q);
void generate_TABLEGETELEM(quad *q);
void generate_TABLESETELEM(quad *q);
void generate_ASSIGN(quad *q);
void generate_NOP(quad *q);
void generate_relational (vmopcode op, quad *q);
void generate_JUMP (quad *q);
void generate_IF_EQ (quad *q);
void generate_IF_NOTEQ(quad *q);
void generate_IF_GREATER (quad *q);
void generate_IF_GREATEREQ(quad *q);
void generate_IF_LESS (quad *q);
void generate_IF_LESSEQ (quad *q);
void generate_NOT (quad *q);
void generate_OR (quad *q);
void generate_PARAM(quad *q);
void generate_CALL(quad *q);
void generate_GETRETVAL(quad *q);
void generate_FUNCSTART(quad *q);
void generate_RETURN(quad *q);
void generate_FUNCEND(quad *q);

void pop_functionstack(struct functionstack *stack);
void push_functionstack(struct functionstack *stack, symrec *s);
symrec* top_functionstack(struct functionstack* stack);



//Functions for functionstack
void push_functionstack(struct functionstack *stack, symrec *s) {
    assert(s);
    struct functionstack* new_node = (struct functionstack*)malloc(sizeof(struct functionstack));
    assert(new_node);
    new_node->top = s; 
    if(stack == NULL){
        stack = new_node;
        stack->next = NULL;
        return;
    }
    new_node->next = stack;
    stack = new_node;

    
}

void pop_functionstack(struct functionstack *stack) {
    assert(stack);
    struct functionstack* temp = stack;
    stack = stack->next;
    free(temp);
}

symrec* top_functionstack(struct functionstack* stack){
    assert(stack);
    assert(stack->top);
    return stack->top;
}




void resetoperand(vmarg *a){
    a = (vmarg*)0;
}

//from lectures //TODO complete cases
void make_operand(expr* e, vmarg* arg){
    if(e == NULL){
        arg->type = nil_a;
        return;
    }
    switch(e->type){
        case var_e:{
            assert(e->sym);
            arg->val = e->sym->offset;
            if(e->sym->type == USERFUNCTION)
                arg->type = userfunc_a;
            else if(e->sym->type == LIBFUNCTION)
                arg->type = libfunc_a;
            else if(e->sym->type == GLOBALVAR)
                arg->type = global_a;
            else if(e->sym->type == LOCALVAR)
                arg->type = local_a;
            else if(e->sym->type == FORMAL)
                arg->type = formal_a;
            else
                assert(0);
            break;
        }
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
            arg->val = e->sym->taddress;
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

void generate_relational(vmopcode op, quad *q){
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = op;
    make_operand(q->arg1, &t->arg1);
    make_operand(q->arg2, &t->arg2);
    t->result.type = label_a;
    if(q->label < currQuad){
        t->result.val = quads[q->label].taddress;
    }
    else{
        add_incomplete_jump(nextinstructionlabel(), q->label);
    }
    q->taddress = nextinstructionlabel();
    emit_instruction(t);
}
void generate_JUMP(quad *q){ generate_relational(jump_v, q); }
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

void generate_PARAM(quad *q){
    q->taddress = nextinstructionlabel();
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = pusharg_v;
    make_operand(q->arg1, &t->arg1);
    emit_instruction(t);
}

void generate_CALL(quad *q){
    q->taddress = nextinstructionlabel();
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = call_v;
    make_operand(q->arg1, &t->arg1);
    emit_instruction(t);
}

void generate_GETRETVAL(quad *q){
    q->taddress = nextinstructionlabel();
    instruction *t = (instruction*)malloc(sizeof(instruction));
    t->opcode = assign_v;
    make_operand(q->result, &t->result);
    make_retvaloperand(&t->arg1);
    emit_instruction(t);
}

void generate_FUNCSTART(quad *q){
    symrec *f = q->result->sym;
    f->taddress = nextinstructionlabel(); // Symbols have iaddress field NOT taddress
    q->taddress = nextinstructionlabel();
    //TODO 


}