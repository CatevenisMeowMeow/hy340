#include"sym_table.h"


//from lectures
typedef enum iopcode{
    assign, add, sub,
    mul, divide,/*cant use div*/ mod,
    uminus, and, or,
    not, if_eq, if_noteq,
    if_lesseq, if_greatereq, if_less,
    if_greater, call, param,
    ret, getretval,  funcstart,
    funcend, tablecreate, jump,
    tablegetelem, tablesetelem
} iopcode;


//Will be used to make print easier(gia na glytwsoume ta polla if else...arkei na kanoyme swsto indexing)
char *iopcode_to_string[] = {
    "assign", "add", "sub",
    "mul", "div", "mod",
    "uminus", "and", "or",
    "not", "if_eq", "if_noteq",
    "if_lesseq", "if_greatereq", "if_less",
    "if_greater", "call", "param",
    "ret", "getretval",  "funcstart",
    "funcend", "tablecreate", "jump",
    "tablegetelem", "tablesetelem"
};

//from lectures
typedef enum expr_t{
    var_e,
    tableitem_e,
    programfunc_e,
    libraryfunc_e,
    arithexpr_e,
    boolexpr_e,
    assignexpr_e,
    newtable_e,
    constnum_e,
    constbool_e,
    conststring_e,
    nil_e
} expr_t;

//from lectures
typedef struct expr{
    expr_t type;
    symrec* sym;
    struct expr * index;
    double numConst;
    char * strConst;
    unsigned char boolConst;
    unsigned label;
    unsigned isNot;
    struct expr* truelist;
    struct expr* falselist;
    struct expr* next;
} expr;

//from lectures
typedef struct quad{
    iopcode op;
    expr * result;
    expr * arg1;
    expr * arg2;
    unsigned label;
    unsigned line;
} quad;

//from lectures
unsigned total = 0;
unsigned int currQuad = 0;
quad* quads = (quad*) 0;



//definitions from lectures
#define EXPAND_SIZE 1024
#define CURR_SIZE   (total*sizeof(quad))
#define NEW_SIZE    (EXPAND_SIZE*sizeof(quad) + CURR_SIZE)

//to extend the dynamic quads array
void extend_quads_array(){
    quad* new_quads = malloc(NEW_SIZE); 
    if(total = 0){
        quads = new_quads;
        return;
    }
    memcpy(new_quads, quads, CURR_SIZE);//found that memcpy can do the job to copy all the array to the new_quads
    free(quads);
    quads = new_quads;
    total = total + EXPAND_SIZE;
}


void emit(iopcode op, expr* result, expr* arg1, expr* arg2, unsigned label, unsigned line){
    if(currQuad == total){
        extend_quads_array();
    }
    quad* quad = quads + currQuad;
    currQuad++;
    quad->arg1 = arg1;
    quad->arg2 = arg2;
    quad->op = op;
    quad->result = result;
    quad->label = label;
    quad->line = line;
    
}

