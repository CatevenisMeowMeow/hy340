#include"sym_table.h"

#define MAX_TEMP_NAME 8

//Global vars
int temp_values_counter = 0;

//Global vars from lectures
unsigned programVarOffset = 0;
unsigned functionLocalOffset = 0;
unsigned formalArgOffset = 0;
unsigned scopeSpaceCounter = 1;



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


//Will be used to make print easier(gia na glytwsoume ta polla if else sthn print_quads...arkei na kanoyme swsto indexing)
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
    struct expr* index;
    double numConst;
    char* strConst;
    unsigned char boolConst;
    unsigned label;
    unsigned isNot;
    struct expr* truelist;
    struct expr* falselist;
    struct expr* next; //will be usefull for function parameters
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

//global vars from lectures
unsigned total = 0;
unsigned int currQuad = 0;
quad* quads = (quad*) 0;

//global var for the elist for a function call or def
expr* function_params_head = NULL;



//definitions from lectures
#define EXPAND_SIZE 1024
#define CURR_SIZE   (total*sizeof(quad))
#define NEW_SIZE    (EXPAND_SIZE*sizeof(quad) + CURR_SIZE)


//ENDS OF DEFINITIONS

//FUNCTIONS...

//FUNCTIONS FOR EXPRESSION LIST

//function to add to the elist a new expr
void add_to_expr_list(expr *e){
    assert(e);
    if(function_params_head == NULL){
        function_params_head = e;
        return;
    }
    expr* tmp = function_params_head;
    expr* prev = NULL;
    while(tmp!=NULL){
        prev = tmp;
        tmp = tmp->next;
    }
    prev->next = e;
}

//function to clear elist
void clear_expr_list(){
    if(function_params_head == NULL)
        return;
    expr* tmp = function_params_head;
    tmp = tmp->next;
    expr* prev = tmp;
    while(tmp!=NULL){
        prev = tmp;
        tmp = tmp->next;
        free(prev);
    }
    function_params_head = NULL;
}





//FUNCTIONS FOR SCOPESPACE
//Function from lectures
scopespace_t currscopespace(){
    if(scopeSpaceCounter == 1)
        return programvar;
    else if(scopeSpaceCounter % 2 == 0)
        return formalarg;
    else
        return functionlocal;
}

//Function from lectures
unsigned currscopeoffset(){
    switch(currscopespace()){
        case programvar : return programVarOffset;
        case functionlocal : return functionLocalOffset;
        case formalarg : return formalArgOffset;
        default : assert(0);
    }
}

//Function from lectures
void incurrscopeoffset(){
    switch(currscopespace()){
        case programvar : ++programVarOffset; break;
        case functionlocal : ++functionLocalOffset; break;
        case formalarg : ++formalArgOffset; break;
        default : assert(0);
    }
}

//Function from lectures
void entersscopespace(){
    ++scopeSpaceCounter;
}

//Function from lectures
void exitscopespace(){
    assert(scopeSpaceCounter > 1);
    --scopeSpaceCounter;
}

int currscope(){
    return scope;
}

/*Vardis*/
void updateQuadLabel(unsigned quadIndex, unsigned newLabel) {
    assert(quadIndex < currQuad); // We check that the quad index is within the limits.
    quads[quadIndex].label = newLabel; // We renew the label of the quads.
}

/*Vardis*/
void validateUnaryMinus(expr * e){
    if(e->type == constbool_e || e->type == conststring_e || e->type == nil_e || e->type == newtable_e || e->type == programfunc_e || e->type == libraryfunc_e || e->type == boolexpr_e){
        perror("Type error: Unary minus operator is not applicable to this expression.");
    }
}

/*Vardis*///Yparxei hdh h is_temp_val poy kanei akrivws to idio
unsigned int istempname(char * s){
    return *s == '_';
}

/*Vardis*/
unsigned int istempexpr(expr * e){
    return e->sym && (e->sym->type == LOCALVAR ||  e->sym->type == GLOBALVAR) && istempname(e->sym->name);
}



/*Vardis*/
void extraSets(symrec * sym){
    sym->space = currscopespace();
    sym->offset = currscopeoffset();
    incurrscopeoffset();
}

//FUNCTIONS FOR TEMP VALS
//Function implement from lectures for temporary values
char* newtempname(){
    char* temp = (char*)malloc(MAX_TEMP_NAME*sizeof(char));
    sprintf(temp,"_t%d",temp_values_counter);
    temp_values_counter++;
    return temp;
}

//Function implement from lectures for temporary values
void resettemp(){
    temp_values_counter = 0;
}

//Function implement from lectures for temporary values
symrec* newtemp(){
    char* new_temp_name = newtempname();
    int currentscope = currscope();
    symrec* rec = lookup_scope(new_temp_name,currentscope);
    if(rec == NULL){
        insert(new_temp_name, LOCALVAR, 0, currentscope);
        rec = lookup_scope(new_temp_name,currentscope);
        return rec;
    }
    return rec;
}

/*Vardis*/
symrec * returnTempName(expr * expr1, expr * expr3){
    if(expr1 != NULL && istempexpr(expr1)){
		return expr1->sym;
	}else if(expr3 != NULL && istempexpr(expr3)){
		return expr3->sym;
	}else{
		return newtemp();
	}
}


//to check if a var is temp or not by name
int is_temp_val(char* name){
    if(strlen(name) < 3)
        return 0;
    if(*name == '_' & *(name+1) == 't')
        return 1;
    return 0;
}





//FUNCTIONS FOR EXPRESSIONS

//New expr with an expr_t
expr* newexpr_type(expr_t type){
    expr* ex = (expr*)malloc(sizeof(expr));
    ex->type = type;
    return ex;
}


//New numconst expr
expr* newexpr_numConst(double num){
    expr* ex = (expr*)malloc(sizeof(expr));
    ex->type = constnum_e;
    ex->numConst = num;
    return ex;
}

//New strconst expr
expr* newexpr_strConst(char* str){
    assert(str);
    expr* ex = (expr*)malloc(sizeof(expr));
    ex->strConst = malloc(strlen(str)*sizeof(char) + 1);
    ex->type = conststring_e;
    strcpy(ex->strConst,str);
    return ex;
}

//New boolconst expr
expr* newexpr_boolConst(unsigned char c){
    expr* ex = (expr*)malloc(sizeof(expr));
    ex->boolConst = c; // 0 false and 1 true
    ex->type = constbool_e;
    return ex;
}





//FUNCTIONS FOR QUADS
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
    quad* q = quads + currQuad;
    currQuad++;
    q->arg1 = arg1;
    q->arg2 = arg2;
    q->op = op;
    q->result = result;
    q->label = label;
    q->line = line;
    
}



//Function to print quads array. Does not check for errors assuming quads are correctly inserted
void print_quads(){
    int i;
    printf("quad#   opcode      result      arg1        arg2        label\n");
    printf("-------------------------------------------------------------\n");
    for(i=0;i<currQuad;i++){
        //quad#
        printf("%d         ",i);

        //opcode
        printf("%s      ",iopcode_to_string[quads[i].op]);

        //result
        if(quads[i].result == (expr*)0)
            printf("              ");
        else
            printf("%s      ",quads[i].result->sym->name);

        //for arg1
        //if it is a constnum, conststring or constbool just prints the value
        if(quads[i].arg1 == (expr*)0)
            printf("           ");
        else{
            if(quads[i].arg1->type == constnum_e){
                    printf("'%f'      ",quads[i].arg1->numConst);
                }
                else if(quads[i].arg1->type == conststring_e){
                    printf("'%s'      ",quads[i].arg1->strConst);
            }
            else if(quads[i].arg1->type == constbool_e){
                if(quads[i].arg1->boolConst == 0)
                    printf("'false'      ");
                else
                    printf("'true'      ");
            }
                //otherwise it prints the symbol name(ex. x)
            else{
                printf("%s      ",quads[i].arg1->sym->name);
            }
        }

        //for arg2

        if(quads[i].arg1 == (expr*)0)
            printf("              ");
        else{
            if(quads[i].arg2->type == constnum_e){
                printf("'%f'      ",quads[i].arg2->numConst);
            }
            else if(quads[i].arg2->type == conststring_e){
                printf("'%s'      ",quads[i].arg2->strConst);
            }
            else if(quads[i].arg2->type == constbool_e){
                if(quads[i].arg2->boolConst == 0)
                    printf("'false'      ");
                else
                    printf("'true'      ");
            }
            else{
                printf("%s      ",quads[i].arg2->sym->name);
            }
        }
        //for label
        printf("%d\n",quads[i].label);
    }

}

