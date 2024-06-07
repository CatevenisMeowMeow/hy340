
%{
        #include<stdio.h>
        #include<stdlib.h>
        #include<string.h>
        #include "avm.h"
        #define YY_DECL int alpha_yylex(void* ylval)
       
        int yyerror(char* message);
        int yylex(void);
        extern int yylineno;
        extern char* yytext;
        extern FILE* yyin;
        extern FILE* yyout;
        int cont = 0;
        int infunction = 0;

        Stack *scopeoffsetStack;
        Stack *breakstack;
        Stack *contstack;
        int anonymous_function_counter = 0;
        char buf[32];
        symrec* tmp = NULL;
        
%}

%start program

%union{
        int int_val;
        double double_val;
        unsigned unsigned_val;
        char *str_val;
        struct sym_table *sym_node;
        struct expr *node;

        struct stmt_t{
                int breaklist, contlist;
        } stmt_t_node;

        struct call{
                struct expr* elist;
                unsigned char method;
                char* name;
        } call_node;

        struct forPrefix{
                unsigned test, enter;
        } forPrefix_node;
}

%type <node> lvalue
%type <node> expr
%type <node> term
%type <node> assignexpr
%type <node> primary
%type <node> const
%type <node> member
%type <node> elist
%type <node> call
%type <node> expressionlist
%type <node> indexedelemlist
%type <node> indexedelem
%type <node> indexed
%type <node> tablemake

%type <stmt_t_node> stmts
%type <stmt_t_node> stmt
%type <stmt_t_node> loopstmt

%type <stmt_t_node> if
%type <stmt_t_node> while
%type <stmt_t_node> for
%type <stmt_t_node> returnstmt

%type <str_val> funcname
%type <unsigned_val> funcbody
%type <sym_node> funcprefix
%type <sym_node> funcdef

%type <call_node> callsufix
%type <call_node> methodcall
%type <call_node> normcall

%type <unsigned_val> ifprefix
%type <unsigned_val> elseprefix

%type <unsigned_val> whilestart
%type <unsigned_val> whilecond

%type <forPrefix_node> forprefix
%type <unsigned_val> N
%type <unsigned_val> M

%token IF 
%token ELSE
%token WHILE
%token FOR
%token FUNCTION
%token RETURN
%token BREAK
%token CONTINUE
%token AND
%token NOT
%token OR
%token LOCAL
%token TRUE
%token FALSE
%token NIL
%token ASSIGN
%token PLUS                    
%token MINUS
%token MULTIPLICATION
%token DIVISION
%token MODULO
%token EQUAL
%token NOT_EQUAL
%token PLUS_PLUS               
%token MINUS_MINUS             
%token GREATER
%token LESS
%token GREATER_EQUAL
%token LESS_EQUAL
%token LEFT_CURLY // '{'
%token RIGHT_CURLY // '}'
%token LEFT_SQUARE// '['
%token RIGHT_SQUARE // ']' 
%token LEFT_PARENTHESIS
%token RIGHT_PARENTHESIS
%token SEMICOLON
%token COMMA
%token COLON  // :
%token DOUBLE_COLON         // ::
%token DOT      // .
%token DOUBLE_DOT         // ..
%token <str_val> ID
%token <int_val> CONST_INT
%token <double_val> CONST_REAL
%token <str_val> STRING
%token COMMENT_NESTED
%token BLOCK_COMMENT
%token COMMENT_LINE


//Priority rules

%left CONST_REAL CONST_INT STRING NIL TRUE FALSE
%left ASSIGN
%left OR
%left AND
%nonassoc EQUAL NOT_EQUAL
%nonassoc GREATER GREATER_EQUAL LESS LESS_EQUAL
%left PLUS MINUS
%left MULTIPLICATION DIVISION MODULO
%left NOT PLUS_PLUS MINUS_MINUS
%left DOT DOUBLE_DOT
%left LEFT_SQUARE RIGHT_SQUARE
%right LEFT_PARENTHESIS RIGHT_PARENTHESIS
%right UMINUS                           // -lvalue error.
%left ELSE                        // Prioritize reduction ELSE.



//End of priority rules

%%


program:  stmts 
            ;

stmt:   expr SEMICOLON {$$.breaklist = $$.contlist = 0; resettemp(); }
        | if  {$$.breaklist = $$.contlist = 0; resettemp();}
        | while  {$$.breaklist = $$.contlist = 0; resettemp();}
        | for  {$$.breaklist = $$.contlist = 0; resettemp();}
        | returnstmt  {$$.breaklist = $$.contlist = 0; resettemp();}
        | BREAK SEMICOLON
        {
                if(loopcounter == 0){
                        comperror("Error: Cannot use break outside of loop", yylineno);
                }
                $$.contlist = 0;
                $$.breaklist = newlist(nextquad()); 
                push(breakstack,$$.breaklist);
                emit(jump, NULL, NULL, NULL, 0, yylineno);

        }
        | CONTINUE SEMICOLON
        {
                if(loopcounter == 0){
                        comperror("Error: Cannot use continue outside of loop", yylineno);
                }
                $$.breaklist = 0;
                $$.contlist = newlist(nextquad()); 
                push(contstack,$$.contlist);
                emit(jump, NULL, NULL, NULL, 0, yylineno);
        }
        | block {$$.breaklist = $$.contlist = 0; resettemp();}
        | funcdef {$$.breaklist = $$.contlist = 0; resettemp();}
        | SEMICOLON  {$$.breaklist = $$.contlist = 0; resettemp();}
        ;

stmts: stmt {$stmts = $stmt;}
| stmts stmt
{           

     //   $$.breaklist = mergelist($1.breaklist, $2.breaklist);
     //   $$.contlist = mergelist($1.contlist, $2.contlist);
        
}
;

expr:   assignexpr{$$ = $1;}
        | expr PLUS expr {check_arith($1,"expr1",yylineno); check_arith($3,"expr2",yylineno); $$ = newexpr_type(arithexpr_e); $$->sym = newtemp(); emit(add,$$,$1,$3,0,yylineno);  }
        | expr MINUS expr {check_arith($1,"expr1",yylineno); check_arith($3,"expr2",yylineno); $$ = newexpr_type(arithexpr_e); $$->sym = newtemp(); emit(sub,$$,$1,$3,0,yylineno);  }
        | expr MULTIPLICATION expr {check_arith($1,"expr1",yylineno); check_arith($3,"expr2",yylineno); $$ = newexpr_type(arithexpr_e); $$->sym = newtemp(); emit(mul,$$,$1,$3,0,yylineno);  }
        | expr DIVISION expr {check_arith($1,"expr1",yylineno); check_arith($3,"expr2",yylineno); $$ = newexpr_type(arithexpr_e); $$->sym = newtemp(); emit(divide,$$,$1,$3,0,yylineno);  }
        | expr MODULO expr {check_arith($1,"expr1",yylineno); check_arith($3,"expr2",yylineno); $$ = newexpr_type(arithexpr_e); $$->sym = newtemp(); emit(mod,$$,$1,$3,0,yylineno);  }
        | expr GREATER expr { $$ = newexpr_type(boolexpr_e); $$->sym = newtemp(); emit(if_greater,$$,$1,$3,nextquad()+3,yylineno); emit(assign,newexpr_boolConst(0),$$,NULL,0,yylineno); emit(jump,NULL,NULL,NULL,nextquad()+2,yylineno); emit(assign,newexpr_boolConst(1),NULL,$$,0,yylineno);}
        | expr GREATER_EQUAL expr { $$ = newexpr_type(boolexpr_e); $$->sym = newtemp(); emit(if_greatereq,$$,$1,$3,nextquad()+3,yylineno); emit(assign,newexpr_boolConst(0),NULL,$$,0,yylineno); emit(jump,NULL,NULL,NULL,nextquad()+2,yylineno); emit(assign,newexpr_boolConst(1),NULL,$$,0,yylineno);}
        | expr LESS expr { $$ = newexpr_type(boolexpr_e); $$->sym = newtemp(); emit(if_less,$$,$1,$3,nextquad()+3,yylineno); emit(assign,newexpr_boolConst(0),$$,NULL,0,yylineno); emit(jump,NULL,NULL,NULL,nextquad()+2,yylineno); emit(assign,newexpr_boolConst(1),$$,NULL,0,yylineno);}
        | expr LESS_EQUAL expr { $$ = newexpr_type(boolexpr_e); $$->sym = newtemp(); emit(if_lesseq,$$,$1,$3,nextquad()+3,yylineno); emit(assign,newexpr_boolConst(0),$$,NULL,0,yylineno); emit(jump,NULL,NULL,NULL,nextquad()+2,yylineno); emit(assign,newexpr_boolConst(1),$$,NULL,0,yylineno);}
        | expr EQUAL expr { $$ = newexpr_type(boolexpr_e); $$->sym = newtemp(); emit(if_eq,$$,$1,$3,nextquad()+3,yylineno); emit(assign,newexpr_boolConst(0),$$,NULL,0,yylineno); emit(jump,NULL,NULL,NULL,nextquad()+2,yylineno); emit(assign,newexpr_boolConst(1),$$,NULL,0,yylineno);}
        | expr NOT_EQUAL expr { $$ = newexpr_type(boolexpr_e); $$->sym = newtemp(); emit(if_noteq,$$,$1,$3,nextquad()+3,yylineno); emit(assign,newexpr_boolConst(0),$$,NULL,0,yylineno); emit(jump,NULL,NULL,NULL,nextquad()+2,yylineno); emit(assign,newexpr_boolConst(1),$$,NULL,0,yylineno);}
        | expr AND expr {  $$ = newexpr_type(boolexpr_e); $$->sym = newtemp(); emit(and,$$,$1,$3,0,yylineno);  }
        | expr OR expr {  $$ = newexpr_type(boolexpr_e); $$->sym = newtemp(); emit(or,$$,$1,$3,0,yylineno);  }
        | term {$$ = $1;}
        ;

term:   LEFT_PARENTHESIS expr RIGHT_PARENTHESIS{$$ = $2;}
        | MINUS expr {check_arith($2,"unary minus",yylineno); $$ = newexpr_type(arithexpr_e); $$->sym = istempexpr($2) ? $2->sym : newtemp();  emit(uminus,$$,$2,NULL,0,yylineno);}
        | NOT expr {$$ = newexpr_type(boolexpr_e); $$->sym = newtemp();  emit(not,$$,$2,NULL,0,yylineno);}
        | PLUS_PLUS lvalue {  check_arith($2,"++lvalue",yylineno);  
                                if($2->type == tableitem_e){
                                        $$ = emit_iftableitem($2);
                                        emit(add, $$, $$, newexpr_numConst(1), 0, yylineno);
                                        emit(tablesetelem, $$, $2, $2->index, 0, yylineno);
                                }
                                else{
                                        emit(add, $2, $2, newexpr_numConst(1), 0, yylineno);
                                        $$ = newexpr_type(arithexpr_e);
                                        $$->sym = newtemp();
                                        emit(assign, $$, $2, NULL, 0, yylineno);       
                                }   
                        }
        | MINUS_MINUS lvalue {
                                check_arith($2,"--lvalue",yylineno);  
                                if($2->type == tableitem_e){
                                        $$ = emit_iftableitem($2);
                                        emit(sub, $$, $$, newexpr_numConst(1), 0, yylineno);
                                        emit(tablesetelem, $$, $2, $2->index, 0, yylineno);
                                }
                                else{
                                        emit(sub, $2, $2, newexpr_numConst(1), 0, yylineno);
                                        $$ = newexpr_type(arithexpr_e);
                                        $$->sym = newtemp();
                                        emit(assign, $$, $2, NULL, 0, yylineno);       
                                } 

                        }
        | lvalue PLUS_PLUS {    check_arith($1,"lvalue++",yylineno); 
                                $$ = newexpr_type(var_e); 
                                $$->sym = newtemp();  
                                if($1->type == tableitem_e){
                                        expr* val = emit_iftableitem($1);
                                        emit(assign, $$, val, NULL, 0, yylineno);
                                        emit(add, val, val, newexpr_numConst(1), 0, yylineno);
                                        emit(tablesetelem, val, $1, $1->index, 0, yylineno);
                                }
                                else{
                                        emit(assign, $$, $1, NULL, 0, yylineno);
                                        emit(add, $1, $1, newexpr_numConst(1), 0, yylineno);
                                }   
                        }
        | lvalue MINUS_MINUS {
                                check_arith($1,"lvalue--",yylineno); 
                                $$ = newexpr_type(var_e); 
                                $$->sym = newtemp();  
                                if($1->type == tableitem_e){
                                        expr* val = emit_iftableitem($1);
                                        emit(assign, $$, val, NULL, 0, yylineno);
                                        emit(sub, val, val, newexpr_numConst(1), 0, yylineno);
                                        emit(tablesetelem, val, $1, $1->index,0, yylineno);
                                }
                                else{
                                        emit(assign, $$, $1, NULL, 0, yylineno);
                                        emit(sub, $1, $1, newexpr_numConst(1), 0, yylineno);
                                } 

                        }
        | primary {$$ = $1; }
        ;
assignexpr: lvalue{    
                       
                     /*  if($1 == NULL){
                                if(scope == 0)
                                        insert(yylval.str_val, GLOBALVAR, yylineno, scope);
                                else
                                        insert(yylval.str_val, LOCALVAR, yylineno, scope);
                                
                                tmp = lookup_scope(yylval.str_val, scope);
                                tmp->space = currscopespace();
                                tmp->offset = currscopeoffset();
                                incurrscopeoffset();
                        }*/
                } 
                ASSIGN expr{  
                                
                                if($lvalue != NULL && $lvalue->type == tableitem_e){
                                        emit(tablesetelem,$expr,$lvalue,$lvalue->index,0,yylineno);
                                        $$ = emit_iftableitem($lvalue);
                                        $$->type = assignexpr_e;
                                }
                                else{
                                        emit(assign,$lvalue,$expr,NULL,0,yylineno);
                                        $$ = newexpr_type(assignexpr_e);
                                        $$->sym = newtemp();
                                        emit(assign,$$,$lvalue,NULL,0,yylineno);
                                }
                }
            ;

primary:    lvalue {
                      /*  if($1 == NULL){
                                if(scope == 0)
                                        insert(yylval.str_val, GLOBALVAR, yylineno, scope);
                                else
                                        insert(yylval.str_val, LOCALVAR, yylineno, scope);
                                
                                tmp = lookup_scope(yylval.str_val, scope);
                                tmp->space = currscopespace();
                                tmp->offset = currscopeoffset();
                                incurrscopeoffset();
                        }*/
                        $$ = emit_iftableitem($1);
        }
            | call {$$ = $1;}
            | tablemake {$$ = $1;} 
            | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS {
                                                                $$ = newexpr_type(programfunc_e);
                                                                $$->sym = $2;
                                                }
            | const {$$ = $1;}
            ;

lvalue: ID      {       
                        tmp = lookup_scope(yylval.str_val, 0);
                        if(tmp != NULL){
                                $$ = lvalue_expr(tmp);
                        }
                        else if((tmp = lookup_scope(yylval.str_val,scope)) != NULL && tmp->active == 1){
                                $$ = lvalue_expr(tmp);
                        }
                        else{
                                if(scope == 0){
                                        insert(yylval.str_val, GLOBALVAR, yylineno, scope);
                                        tmp = lookup_scope(yylval.str_val,scope);
                                        tmp->space = currscopespace();
                                        tmp->offset = currscopeoffset();
                                        incurrscopeoffset();
                                        $$ = lvalue_expr(tmp);
                                }
                                else{
                                        insert(yylval.str_val, LOCALVAR, yylineno, scope);
                                        tmp = lookup_scope(yylval.str_val,scope);
                                        tmp->space = currscopespace();
                                        tmp->offset = currscopeoffset();
                                        incurrscopeoffset();
                                        $$ = lvalue_expr(tmp);
                                }
                        }
                        

                } 
        | LOCAL ID              {
                                        tmp = lookup_scope(yylval.str_val,scope);
                                        if(tmp == NULL){
                                                if(scope == 0){
                                                        insert(yylval.str_val, GLOBALVAR, yylineno, scope);
                                                        tmp = lookup_scope(yylval.str_val,scope);
                                                        tmp->space = currscopespace();
                                                        tmp->offset = currscopeoffset();
                                                        incurrscopeoffset();
                                                        $$ = lvalue_expr(tmp);
                                                }
                                                else{
                                                        insert(yylval.str_val, LOCALVAR, yylineno, scope);
                                                        tmp = lookup_scope(yylval.str_val,scope);
                                                        tmp->space = currscopespace();
                                                        tmp->offset = currscopeoffset();
                                                        incurrscopeoffset();
                                                        $$ = lvalue_expr(tmp);
                                                }
                                        }
                                        else if(tmp->type == LIBFUNCTION){
                                                        yyerror("Trying to shadow libfunction");
                                                        $$ = lvalue_expr(tmp);
                                        }
                                        else{
                                                if(tmp->active == 1)
                                                        $$ = lvalue_expr(tmp);
                                                else{
                                                        if(scope == 0){
                                                                insert(yylval.str_val, GLOBALVAR, yylineno, scope);
                                                                tmp = lookup_scope(yylval.str_val,scope);
                                                                tmp->offset = currscopeoffset();
                                                                tmp->space = currscopespace();
                                                                incurrscopeoffset();
                                                                $$ = lvalue_expr(tmp);
                                                        }
                                                        else{
                                                                insert(yylval.str_val, LOCALVAR, yylineno, scope);
                                                                tmp = lookup_scope(yylval.str_val,scope);
                                                                tmp->offset = currscopeoffset();
                                                                tmp->space = currscopespace();
                                                                incurrscopeoffset();
                                                                $$ = lvalue_expr(tmp);
                                                        }
                                                }
                                        }
                                }
        | DOUBLE_COLON ID{
                                tmp = lookup_scope(yylval.str_val,0);
                                if(tmp == NULL)
                                        yyerror("Global variable not found");
                                $$ = lvalue_expr(tmp);
                                }
        | member   {$$ = $1;}                
        ;

member: lvalue DOT ID {$$ = member_item($1,yylval.str_val);}                  
        | lvalue LEFT_SQUARE expr RIGHT_SQUARE {$1 = emit_iftableitem($1);
                                                $$ = newexpr_type(tableitem_e);
                                                $$->sym = $1->sym;
                                                $$->index = $3;
                                        }  
        | call DOT ID   // { $$ = NULL;}             
        | call LEFT_SQUARE expr RIGHT_SQUARE    //{ $$ = NULL;}  
        ;

call:   call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS { $$ = make_call($1,$elist,yylineno);}
        | lvalue callsufix {    
                                $lvalue = emit_iftableitem($lvalue);
                                if($callsufix.method){
                                        expr* tmp_ex = $callsufix.elist;
                                        //if elist is empty
                                        if(tmp_ex == NULL){
                                                tmp_ex = $lvalue;
                                                tmp_ex->next = NULL;
                                        }
                                        else{ //get to the last
                                                while(tmp_ex->next != NULL) tmp_ex = tmp_ex->next; 
                                                tmp_ex->next = $lvalue;
                                        }
                                        $lvalue = emit_iftableitem(member_item($lvalue,$callsufix.name));
                                }
                                $call = make_call($lvalue,$callsufix.elist,yylineno);
        }
        | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS { expr* ex = newexpr_type(programfunc_e);
                                                                                                ex->sym = $funcdef;
                                                                                                $call = make_call(ex,$elist,yylineno);
                                                                                        }
        ;

callsufix:  normcall {$callsufix = $normcall;}
            | methodcall {$callsufix = $methodcall;}
            ;

normcall:   LEFT_PARENTHESIS elist RIGHT_PARENTHESIS {
                                                        $normcall.elist = $elist;
                                                        $normcall.method = 0;
                                                        $normcall.name = NULL;
                                                }
            ;


methodcall: DOUBLE_DOT ID LEFT_PARENTHESIS elist RIGHT_PARENTHESIS {
                                                                        
                                                                        $methodcall.elist = $elist;
                                                                        $methodcall.method = 1;
                                                                        $methodcall.name = strdup($2);             
                                                        }
            ;

elist:  expr expressionlist{$elist = $expr; $elist->next = $expressionlist;}
        | {$elist = NULL;}
        ;

expressionlist: COMMA expr expressionlist{$$ = $expr; $$->next = $3;}
                | {$$ = NULL;}
                ;



tablemake: LEFT_SQUARE elist RIGHT_SQUARE
{       
        expr* t = newexpr_type(newtable_e);
        t->sym = newtemp();
        emit(tablecreate, t, NULL, NULL, 0, yylineno);
        int i = 0;
        while($elist != NULL){ 
                emit(tablesetelem, t, newexpr_numConst(i++), $elist, 0, yylineno);
                $elist = $elist->next;
        }
        $tablemake = t;

}
                | LEFT_SQUARE indexed RIGHT_SQUARE
                {
                        expr* t = newexpr_type(newtable_e);
                        t->sym = newtemp();
                        emit(tablecreate, t, NULL, NULL, 0, yylineno);
                        while($indexed != NULL){
                                emit(tablesetelem, t, $indexed, $indexed->index, 0, yylineno);
                                $indexed = $indexed->next;
                        }
                        $tablemake = t;
                }
;



indexed:   indexedelem indexedelemlist {$$ = $1; $$->next = $2;} 
           ;

indexedelemlist: COMMA indexedelem indexedelemlist{$$ = $2; $$->next = $3;}
                | {$$ = NULL;}
                ;

indexedelem:    LEFT_CURLY expr COLON expr RIGHT_CURLY{$$ = $4; $$->index = $2;}
                ;

block:  LEFT_CURLY{scope++;} stmts RIGHT_CURLY{hide_scope(scope); scope--;}
|LEFT_CURLY  RIGHT_CURLY //for empty blocks
        ;

funcargs: LEFT_PARENTHESIS{scope++;  entersscopespace();} idlist RIGHT_PARENTHESIS{
                                                                scope--;
                                                                entersscopespace();
                                                                resetfunctionlocalsoffset();
                                                        }



funcbody: block { 
        $$ = currscopeoffset();
       

}
;


funcblockstart:
{       scope++;
        infunction++;
        push_loopcounter();
}
;

funcblockend:
{
        hide_scope(scope);
        scope--;
        infunction--;
        pop_loopcounter();
        
}
;



funcdef:    funcprefix funcargs funcblockstart funcbody funcblockend { 
                        exitscopespace();
                        exitscopespace();
                        $1->totalLocals = $funcbody;
                        int offset = pop(scopeoffsetStack);
                        restorecurrscopeoffset(offset);
                        $$ = $1;
                        emit(funcend,lvalue_expr($1),NULL,NULL,0,yylineno);
                }  
            ;

funcname: ID {  
                        if(is_library_function(yylval.str_val)){
                                yyerror("Trying to shadow libfunction\n");
                        }
                        else{
                                tmp = lookup_scope(yylval.str_val,scope);
                                if(tmp != NULL){
                                        if(tmp->type == GLOBALVAR || tmp->type == LOCALVAR)
                                                fprintf(stderr,"error at line %d: Function '%s' already defined as variable at line %d\n",yylineno,yylval.str_val,tmp->line);
                                        else
                                                fprintf(stderr,"error at line %d: Function '%s' already exists at line %d\n",yylineno,yylval.str_val,tmp->line);
                                        
                                }
                                else{
                                        insert(yylval.str_val, USERFUNCTION, yylineno, scope);
                                        tmp = lookup_scope(yylval.str_val,scope);
                                        tmp->offset = currscopeoffset();
                                        tmp->space = currscopespace();
                                        incurrscopeoffset();
                                }
                        }
                        resetformalargsoffset();
                        $$ = yylval.str_val;
                }
             |  {
                        sprintf(buf,"$%d",anonymous_function_counter);
                        anonymous_function_counter++; 
                        insert(buf,USERFUNCTION,yylineno,scope);
                        $$ = buf;
                        tmp = lookup_scope(buf,scope);
                        tmp->offset = currscopeoffset();
                        tmp->space = currscopespace();
                        incurrscopeoffset();
                        resetformalargsoffset();
                        
                }
             ;

funcprefix: FUNCTION funcname{   
        $$ = lookup_scope($funcname,scope); //Must never return NULL
        assert($$);
        //$$ = new_symbol($2, USERFUNCTION, yylineno, scope); //It is already created from above
        $$->iaddress = nextquad();
        emit(funcstart,lvalue_expr($$),NULL,NULL,0,yylineno);
        push(scopeoffsetStack, currscopeoffset()); // Save current offset in a stack
        entersscopespace(); //Entering function arguments scope space
        resetformalargsoffset(); //Start formals from zero

}


const:  CONST_REAL{$$ = newexpr_numConst(yylval.double_val);} 
        | CONST_INT {$$ = newexpr_numConst(yylval.int_val);}
        | STRING {$$ = newexpr_strConst(yylval.str_val); } 
        | NIL {$$ = newexpr_type(nil_e);} 
        | TRUE {$$ = newexpr_boolConst(1);} 
        | FALSE {$$ = newexpr_boolConst(0);} 
        ;

idlist: ID                 {
                                        formalArgOffset++;
                                        if(is_library_function(yylval.str_val))
                                                yyerror("Trying to shadow libfunction");
                                        else{
                                                tmp = lookup_scope(yylval.str_val,scope);
                                                if(tmp != NULL && tmp->active == 1){
                                                        fprintf(stderr,"Multiple formal args '%s' detected",yylval.str_val);
                                                        yyerror("");
                                                }
                                                else{
                                                        insert(yylval.str_val,FORMAL,yylineno,scope);
                                                        tmp = lookup_scope(yylval.str_val,scope);
                                                        tmp->offset = currscopeoffset();
                                                        tmp->space = currscopespace();
                                                        incurrscopeoffset();
                                                }
                                                
                                        }     
                                }
        | ID {                          formalArgOffset++;
                                        if(is_library_function(yylval.str_val))
                                                yyerror("Trying to shadow libfunction");
                                        else{
                                                tmp = lookup_scope(yylval.str_val,scope);
                                                if(tmp != NULL && tmp->active == 1){
                                                        fprintf(stderr,"Multiple formal args '%s' detected",yylval.str_val);
                                                        yyerror("");
                                                }      
                                                else{
                                                        insert(yylval.str_val,FORMAL,yylineno,scope);
                                                        tmp = lookup_scope(yylval.str_val,scope);
                                                        tmp->offset = currscopeoffset();
                                                        tmp->space = currscopespace();
                                                        incurrscopeoffset();
                                                }
                                        }     
                                } COMMA idlist
        |
        ;

ifprefix: IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS
{    
        emit(if_eq, NULL, $expr, newexpr_boolConst(1), nextquad()+2, yylineno);
        $ifprefix = nextquad();
        emit(jump, NULL, NULL, NULL, 0, yylineno); 


}
;

elseprefix: ELSE
{
        $elseprefix = nextquad();
        emit(jump, NULL, NULL, NULL, 0, yylineno);
}
;

if: ifprefix stmt
{
        patchlabel($ifprefix, nextquad());
}
| ifprefix stmt elseprefix stmt
        {
                patchlabel($ifprefix, $elseprefix + 1);
                patchlabel($elseprefix, nextquad());
        }
;

whilestart: WHILE
{
        $whilestart = nextquad();

}
;

whilecond: LEFT_PARENTHESIS expr RIGHT_PARENTHESIS
{       
        emit(if_eq, NULL, $expr, newexpr_boolConst(1), nextquad()+2, yylineno);
        $whilecond = nextquad();
        emit(jump, NULL, NULL, NULL, 0, yylineno);
}
;


while: whilestart whilecond loopstmt
{       
        emit(jump, NULL ,NULL , NULL, $whilestart, yylineno);
        patchlabel($whilecond, nextquad());

        //This is new patenda with stack
        if(breakstack->top == -1 && contstack->top == -1){
                patchlist($loopstmt.breaklist, nextquad());
                patchlist($loopstmt.contlist, $whilestart); 
        }
        else{
                while(breakstack->top != -1){
                        patchlist(pop(breakstack), nextquad());
                }
                while(contstack->top != -1){
                        patchlist(pop(contstack), $whilestart); 
                }
        }

        //That was from lectures
       // patchlist($loopstmt.breaklist, nextquad()); 
       // patchlist($loopstmt.contlist, $whilestart); 
     
}
;

N: { $N = nextquad(); emit(jump, NULL, NULL, NULL, 0, yylineno);}
;

M: { $M = nextquad();}
;


forprefix: FOR LEFT_PARENTHESIS elist SEMICOLON M expr SEMICOLON
{
        $forprefix.test = $M;
        $forprefix.enter = nextquad();
        emit(if_eq, NULL, $expr, newexpr_boolConst(1), 0, yylineno);
}
;
for: forprefix N elist RIGHT_PARENTHESIS N loopstmt N
{
    

        patchlabel($forprefix.enter, $5+1); //true jump 
        patchlabel($2, nextquad()); //false jump
        patchlabel($5, $forprefix.test); // loop jump
        patchlabel($7, $2 + 1); // closure jump

        //This is new patenda with stack
        if(breakstack->top == -1 && contstack->top == -1){
                patchlist($loopstmt.breaklist, nextquad());
                patchlist($loopstmt.contlist, $2 + 1); 
        }
        else{
                while(breakstack->top != -1){
                        patchlist(pop(breakstack), nextquad());
                }
                while(contstack->top != -1){
                        patchlist(pop(contstack), $2 + 1); 
                }
        }

        //That was from lectures
       // patchlist($loopstmt.breaklist, nextquad()); 
       // patchlist($loopstmt.contlist, $2 + 1);
}
;

loopstart: {++loopcounter;}
;

loopend:  {--loopcounter;}
;

loopstmt: loopstart stmt loopend {$loopstmt = $stmt;}
;


returnstmt: RETURN expr SEMICOLON
{ 
        if(!infunction) comperror("Error: Cannot use return out of function block",yylineno);
        emit(ret, $expr, NULL, NULL, 0, yylineno);
}
| RETURN SEMICOLON 
{ 
        if(!infunction) comperror("Error: Cannot use return out of function block",yylineno);
        emit(ret, NULL, NULL, NULL, 0, yylineno); 
}
;


//End of grammar rules


%%

int yyerror(char* message){
        fprintf(stderr,"%s at line %d, before token: %s\n",message,yylineno,yytext);
}




int main( int argc, char** argv) {

        FILE *f = fopen("target_code","w");

        if(argc > 1){
                if(!(yyin = fopen(argv[1],"r"))){
                        fprintf(stderr,"Cannot open file: %s\n",argv[1]);
                        return 1;
                }
        }
        else 
                yyin = stdin;
        insert_library_functions();
        
        scopeoffsetStack = newStack();
        initializeStack(scopeoffsetStack);

        breakstack = newStack();
        initializeStack(breakstack);
        //push(breakstack,0);

        contstack = newStack();
        initializeStack(contstack);
       // push(contstack,0);

        push_loopcounter();

        yyparse();
        //2h fash
        //print_symbol_table();

        //3h fash
        print_quads();

        //4h fash
        generate_all();
        print_instructions();
        print_instructions_vals();
        print_binary_instructions(f);

        //5h fash //TODO
        avm_initialize();
        avm_load_instructions("target_code");
        printf("\nStarting avm execution...\n\n");

        unsigned int cycle_count = 0; //for debugging
        while(executionFinished != 1){
                printf("Cycle #%u: ",cycle_count);//for debugging
                print_curr_instr(); //for debugging
                execute_cycle();
                cycle_count++;//for debugging
        }
        printf("\n\nexecution finished!\n");

        return 0;
}
