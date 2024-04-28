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
typedef struct stmt_t {
    int breakList, contList;
} stmt_t;


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
    struct expr* truelist; //gia thn meriki apotimisi
    struct expr* falselist; //gia thn meriki apotimisi
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
//expr* elist = NULL;



//definitions from lectures
#define EXPAND_SIZE 1024
#define CURR_SIZE   (total*sizeof(quad))
#define NEW_SIZE    (EXPAND_SIZE*sizeof(quad) + CURR_SIZE)


//ENDS OF DEFINITIONS

//FUNCTIONS DEFINITIONS(HELPS TO REMOVE WARNINGS)

// FUNCTIONS FOR STMTS
void make_stmt(stmt_t* s);
int newlist(int i);
int mergelist(int l1, int l2);
void patchlist(int list, int label);

//FUNCTIONS FOR CALLS
//Call* new_call();


void comperror (char* format);

// FUNCTIONS FOR EXPRESSIONS

void check_arith (expr* e, const char* context);
expr* emit_iftableitem(expr* e);
expr* member_item(expr* lv, char* name);
expr* make_call(expr* lv, expr* reversed_elist, int line);
void add_to_expr_list(expr* elist,expr *e);
void clear_expr_list(expr* elist);
void patchlabel(unsigned quadNo, unsigned label);
unsigned nextquad();
scopespace_t currscopespace();
unsigned currscopeoffset();
void incurrscopeoffset();
void entersscopespace();
void exitscopespace();
void resetformalargsoffset();
void resetfunctionlocalsoffset();
void restorecurrscopeoffset(unsigned offset);
int currscope();
void updateQuadLabel(unsigned quadIndex, unsigned newLabel);
void validateUnaryMinus(expr * e);
unsigned int istempname(char * s);
unsigned int istempexpr(expr * e);
void extraSets(symrec * sym);
char* newtempname();
void resettemp();
symrec* newtemp();
symrec * returnTempName(expr * expr1, expr * expr3);
int is_temp_val(char* name);

expr* newexpr_type(expr_t type);
expr* newexpr_numConst(double num);
expr* newexpr_strConst(char* str);
expr* newexpr_boolConst(unsigned int c);
expr* lvalue_expr(symrec *sym);

// FUNCTIONS FOR QUADS
void extend_quads_array();
void emit(iopcode op, expr* result, expr* arg1, expr* arg2, unsigned label, unsigned line);
void emit_function_params(expr* elist);
int type_matching(expr* arg1, expr* arg2);
void print_quads();

//END OF FUNCTION DEFINITIONS




//FUNCTIONS...

//FUNCTIONS FOR STMTS

//from lectures
void make_stmt (stmt_t* s)
{ s->breakList = s->contList = 0; }

//from lectures
int newlist (int i)
{ quads[i].label = 0; return i; }

//from lectures
int mergelist (int l1, int l2) {
    if (!l1)
        return l2;
    else
        if (!l2)
            return l1;
    else {
        int i = l1;
        while (quads[i].label)
            i = quads[i].label;
        quads[i].label = l2;
    return l1;
    }
}

//from lectures
void patchlist (int list, int label) {
    while (list) {
        int next = quads[list].label;
        quads[list].label = label;
        list = next;
    }
}


void comperror (char* format){
    fprintf(stderr,"%s\n",format);
}


//FUNCTIONS FOR EXPRESSIONS

//From lectures
// Use this function to check correct use of
// of expression in arithmetic
void check_arith (expr* e, const char* context) {
    char err[] = "Illegal expr used in ";
    char error[strlen(err) + strlen(context) +1];
    strcpy(error,"");
    strcat(error,err);
    strcat(error,context); 
    if( e->type == constbool_e || e->type == conststring_e || e->type == nil_e || e->type == newtable_e|| e->type == programfunc_e || e->type == libraryfunc_e || e->type == boolexpr_e )
        comperror(error);
}


//from lectures
expr* member_item(expr* lv, char* name) {
    lv = emit_iftableitem(lv); // Emit code if r-value use of table item
    expr* ti = newexpr_type(tableitem_e); // Make a new expression
    ti->sym = lv->sym;
    ti->index = newexpr_strConst(name); // Const string index
    return ti;
}


//from lectures
expr* emit_iftableitem(expr* e){
    if(e->type != tableitem_e)
        return e;
    else{
        expr* result = newexpr_type(var_e);
        result->sym = newtemp();
        emit(tablegetelem, result,e, e->index, currQuad, e->sym->line);
        return result;
    }
}

//from lectures(added line because it is required for emit and quads)
expr* make_call(expr* lv, expr* reversed_elist, int line) {
    expr* func = emit_iftableitem(lv);
    while (reversed_elist) {
        emit(param, NULL, reversed_elist, NULL,currQuad,line);
        reversed_elist = reversed_elist->next;
    }
    emit(call, func,NULL,NULL,currQuad,line);
    expr* result = newexpr_type(var_e);
    result->sym = newtemp();
    emit(getretval,result,NULL,NULL,currQuad,line);
    return result;
}


//function to add to the elist a new expr(usefull for calls or for stmts)
void add_to_expr_list(expr* elist,expr *e){
    assert(e);
    if(elist == NULL){
        elist = e;
        return;
    }
    expr* tmp = elist;
    expr* prev = NULL;
    while(tmp!=NULL){
        prev = tmp;
        tmp = tmp->next;
    }
    prev->next = e;
}

//function to clear elist(usefull for calls or for stmts)
//DO NOT USE IT BEFORE YOU PRINT QUADS. IT WILL THROW SEGMENTATION
void clear_expr_list(expr* elist){
    if(elist == NULL)
        return;
    expr* tmp = elist;
    tmp = tmp->next;
    expr* prev = NULL;
    while(tmp!=NULL){
        prev = tmp;
        tmp = tmp->next;
        free(prev);
    }
    elist = NULL;
}




//function from lectures
void patchlabel(unsigned quadNo, unsigned label){
    assert(quadNo < currQuad && !quads[quadNo].label);
    quads[quadNo].label = label;
}

//function from lectures
unsigned nextquad(){
    return currQuad;
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
//Function from lectures
void resetformalargsoffset(){
    formalArgOffset = 0;
}

//Function from lectures
void resetfunctionlocalsoffset(){
    functionLocalOffset = 0;
}

//Function from lectures
void restorecurrscopeoffset(unsigned offset){
    switch(currscopespace()){
        case programvar : programVarOffset = offset; break;
        case functionlocal : functionLocalOffset = offset; break;
        case formalarg : formalArgOffset = offset; break;
        default : assert(0);
    }
}


int currscope(){
    return scope;
}

/*Vardis*/
void updateQuadLabel(unsigned quadIndex, unsigned newLabel) {
    assert(quadIndex < currQuad); // We check that the quad index is within the limits.
    quads[quadIndex].label = newLabel; // We renew the label of the quads.
}

expr_t returnType(expr * temp, expr * a, expr * b){

    if(a->type == constnum_e && b->type == constnum_e){
        return constnum_e;
    }
    return temp->type;
}

/*Vardis*/
void validateUnaryMinus(expr * e){
    if(e->type == constbool_e || e->type == conststring_e || e->type == nil_e || e->type == newtable_e || e->type == programfunc_e || e->type == libraryfunc_e || e->type == boolexpr_e){
        perror("Type error: Unary minus operator is not applicable to this expression.");
    }
}

/*Vardis*///
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

//New expr with symbol. Requested from lectures...
expr* lvalue_expr(symrec *sym){
    assert(sym);
    expr* ex = (expr*)malloc(sizeof(expr));
    memset(ex,0,sizeof(expr));
    ex->next = (expr*)0;
    ex->sym = sym;
    if(sym == NULL)
        ex->type = nil_e;
    else if((sym->type == GLOBALVAR) || (sym->type == LOCALVAR) || (sym->type == FORMAL))
        ex->type = var_e;
    else if(sym->type == USERFUNCTION)
        ex->type = programfunc_e;
    else if(sym->type == LIBFUNCTION)
        ex->type = libraryfunc_e;
    else{
        assert(0);
    }
    return ex;
}


//New expr with an expr_t
expr* newexpr_type(expr_t type){
    expr* ex = (expr*)malloc(sizeof(expr));
    memset(ex,0,sizeof(expr));
    ex->type = type;
    return ex;
}


//New numconst expr
expr* newexpr_numConst(double num){
    expr* ex = newexpr_type(constnum_e);
    ex->numConst = num;
    return ex;
}

//New strconst expr
expr* newexpr_strConst(char* str){
    assert(str);
    expr* ex = newexpr_type(conststring_e);
    ex->strConst = malloc(strlen(str)*sizeof(char) + 1);
    strcpy(ex->strConst,str);
    return ex;
}

//New boolconst expr
expr* newexpr_boolConst(unsigned int c){
    expr* ex = newexpr_type(constbool_e);
    ex->boolConst = !!c;
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
    memcpy(new_quads, quads, CURR_SIZE);
    free(quads);
    quads = new_quads;
    total = total + EXPAND_SIZE;
}

//function to add a quad to quads array
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

//function to make function def using elist
//Should make the elist first then call this function and then clear_expr_elist
//Should put the correct type to make multiple quads with this type
void emit_function_params(expr* elist){
    expr* e = elist;
    assert(e);
    while(e != NULL){
        assert(e->sym != NULL);
        emit(param,NULL,e,NULL,0,e->sym->line);
        e = e->next;
    }

}

//function that compares the types between two expressions. Returns 1 at success and 0 at fail.
int type_matching(expr* arg1, expr* arg2){
    assert(arg1);
    assert(arg2);
    if(arg1->type == arg2->type)
        return 1;
    return 0;
}



//Function to print quads array. Does not check for errors assuming quads are correctly inserted
void print_quads() {
    int i;
    printf("%-10s%-15s%-20s%-20s%-10s%-10s\n", "quad#", "opcode", "result", "arg1", "arg2", "       label");
    printf("---------------------------------------------------------------------------------------\n");
    for (i = 0; i < currQuad; i++) {
        // quad#
        printf("%-10d", i);

        // opcode
        printf("%-15s", iopcode_to_string[quads[i].op]);

        // result
        if (quads[i].result == (expr*)0)
            printf("%-20s", " ");
        else {
            if (quads[i].result->type == constnum_e) {
                printf("%-20.2f", quads[i].result->numConst);
            }
            else if (quads[i].result->type == conststring_e) {
                printf("%-20s", quads[i].result->strConst);
            }
            else if (quads[i].result->type == constbool_e) {
                printf("%-20s", quads[i].result->boolConst ? "true" : "false");
            }
            //otherwise it prints the symbol name(ident)
            else {
                assert(quads[i].result->sym);
                printf("%-20s", quads[i].result->sym->name);
            }
        }

        //for arg1
        //if it is a constnum, conststring or constbool just prints the value. If null just prints whitespace
        if (quads[i].arg1 == (expr*)0)
            printf("%-20s", " ");
        else {
            if (quads[i].arg1->type == constnum_e) {
                printf("%-20.2f", quads[i].arg1->numConst);
            }
            else if (quads[i].arg1->type == conststring_e) {
                printf("%-20s", quads[i].arg1->strConst);
            }
            else if (quads[i].arg1->type == constbool_e) {
                printf("%-20s", quads[i].arg1->boolConst ? "true" : "false");
            }
            else {
                //assert(quads[i].arg1->sym);
                if(quads[i].arg1->sym == NULL)
                    printf("(NULL SYMBOL)       ");
                else
                    printf("%-20s", quads[i].arg1->sym->name);
            }
        }

        //for arg2
        //Same with arg1
        if (quads[i].arg2 == (expr*)0)
            printf("%-20s", " ");
        else {
            if (quads[i].arg2->type == constnum_e) {
                printf("%-20.2f", quads[i].arg2->numConst);
            }
            else if (quads[i].arg2->type == conststring_e) {
                printf("%-20s", quads[i].arg2->strConst);
            }
            else if (quads[i].arg2->type == constbool_e) {
                printf("%-20s", quads[i].arg2->boolConst ? "true" : "false");
            }
            else {
                 //assert(quads[i].arg1->sym);
                if(quads[i].arg2->sym == NULL)
                    printf("(NULL SYMBOL)       ");
                else
                    printf("%-20s", quads[i].arg2->sym->name);
            }
        }

        // label
        printf("%-10d\n", quads[i].label);
    }
    printf("\n---------------------------------------------------------------------------------------\n");
}

