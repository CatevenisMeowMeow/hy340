#include"sym_table.h"



typedef enum iopcode{
    assign,         add,            sub,
    mul,            divide,/*cant use div*/           mod,
    uminus,         and,            or,
    not,            if_eq,          if_noteq,
    if_lesseq,      if_greatereq,   if_less,
    if_greater,     call,           param,
    ret,            getretval,      funcstart,
    funcend,        tablecreate,    jump,
    tablegetelem,   tablesetelem
} iopcode;


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


typedef struct quad{
    iopcode op;
    expr * result;
    expr * arg1;
    expr * arg2;
    unsigned label;
    unsigned line;
} quad;


unsigned total = 0;
unsigned int currQuad = 0;
quad* quads = (quad*) 0;




#define EXPAND_SIZE 1024
#define CURR_SIZE   (total*sizeof(quad))
#define NEW_SIZE    (EXPAND_SIZE*sizeof(quad) + CURR_SIZE)



