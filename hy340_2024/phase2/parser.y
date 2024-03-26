
%{
        #include<stdio.h>
        #include<stdlib.h>
        #include<string.h>
        #include "sym_table.h"
        #define YY_DECL int alpha_yylex(void* ylval)
       
        int yyerror(char* message);
        int yylex(void);
        extern int yylineno;
        extern char* yytext;
        extern FILE* yyin;
        extern FILE* yyout;

        int scope = 0;
        int anonymous_function_counter = 0;
        char buf[32];
        symrec* tmp = NULL;
        
%}

%start program

%union{
        int int_val;
        double double_val;
        char *str_val;
        struct sym_table *node;
}

%type <node> lvalue


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
%right EXPRESSION_READ_NEXT_CHARACTER   // opeartions.
%left ELSE                        // Prioritize reduction ELSE.



//End of priority rules

%%


program: {printf("Started compiling...\n");}  stmts 
            ;





stmt:   expr SEMICOLON
        | ifstmt
        | whilestmt
        | forstmt
        | returnstmt
        | BREAK SEMICOLON
        | CONTINUE SEMICOLON
        | block
        | funcdef
        | SEMICOLON 
      //  | ignore
        ;

stmts: stmt stmts
        |
        ;

expr:   assignexpr
        | expr op expr %prec EXPRESSION_READ_NEXT_CHARACTER
        | term
        ;

op: PLUS 
    | MINUS 
    | MULTIPLICATION
    | DIVISION 
    | MODULO 
    | GREATER 
    | GREATER_EQUAL 
    | LESS 
    | LESS_EQUAL 
    | EQUAL 
    | NOT_EQUAL 
    | AND 
    | OR
    ;

term:   LEFT_PARENTHESIS expr RIGHT_PARENTHESIS
        | MINUS expr %prec UMINUS  
        | NOT expr
        | PLUS_PLUS lvalue {if($2 != NULL && ($2->type == USERFUNCTION || $2->type == LIBFUNCTION)) yyerror("Variable is function");}
        | MINUS_MINUS lvalue {if($2 != NULL && ($2->type == USERFUNCTION || $2->type == LIBFUNCTION)) yyerror("Variable is function");}
        | lvalue PLUS_PLUS {if($1 != NULL && ($1->type == USERFUNCTION || $1->type == LIBFUNCTION)) yyerror("Variable is function");}
        | lvalue MINUS_MINUS {if($1 != NULL && ($1->type == USERFUNCTION || $1->type == LIBFUNCTION)) yyerror("Variable is function");}
        | primary
        ;

assignexpr: lvalue ASSIGN expr {if($1 != NULL && ($1->type == USERFUNCTION || $1->type == LIBFUNCTION))
                                        yyerror("Variable is function");
                                }
            ;

primary:    lvalue {}
            | call
            | objectdef
            | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS
            | const
            ;

lvalue: ID      {       
                        tmp = lookup_scope(yylval.str_val, 0);
                        if(tmp != NULL && tmp->type == LIBFUNCTION){
                                yyerror("Trying to shadow libfunction");
                                $$ = tmp;
                        }
                        else if((tmp = lookup_scope(yylval.str_val,scope)) != NULL){
                                $$ = tmp;
                        }
                        else{
                                if(scope == 0){
                                        insert(yylval.str_val, GLOBALVAR, yylineno, scope);
                                        $$ = tmp;
                                }
                                else{
                                        insert(yylval.str_val, LOCALVAR, yylineno, scope);
                                        $$ = tmp;
                                }
                        }
                        

                } 
        | LOCAL ID              {
                                        tmp = lookup_scope(yylval.str_val,scope);
                                        if(tmp == NULL){
                                                if(scope == 0){
                                                        insert(yylval.str_val, GLOBALVAR, yylineno, scope);
                                                        $$ = tmp;
                                                }
                                                else{
                                                        insert(yylval.str_val, LOCALVAR, yylineno, scope);
                                                        $$ = tmp;
                                                }
                                        }
                                        else if(tmp->type == LIBFUNCTION){
                                                        yyerror("Trying to shadow libfunction");
                                                        $$ = tmp;
                                        }
                                        else{
                                                $$ = tmp;
                                        }
                                }
        | DOUBLE_COLON ID{
                                tmp = lookup_scope(yylval.str_val,0);
                                if(tmp == NULL)
                                        yyerror("Global variable not found");
                                $$ = tmp;

                                }
        | member        {}                
        ;

member: lvalue DOT ID                   
        | lvalue LEFT_SQUARE expr RIGHT_SQUARE    
        | call DOT ID                   
        | call LEFT_SQUARE expr RIGHT_SQUARE      
        ;

call:   call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
        | lvalue callsufix
        | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
        ;

callsufix:  normcall
            | methodcall
            ;

normcall:   LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
            ;

methodcall: DOUBLE_DOT ID LEFT_PARENTHESIS elist RIGHT_PARENTHESIS
            ;

elist:  expr expressionlist
        |
        ;

expressionlist: COMMA expr expressionlist
                |
                ;


objectdef:  LEFT_SQUARE objectdeflist RIGHT_SQUARE  
            ;

objectdeflist: elist | indexed 
;

indexed:    LEFT_SQUARE indexedelemlist RIGHT_SQUARE 
            ;

indexedelemlist: indexedelem
                 | indexedelem COMMA indexedelemlist
                 ;

indexedelem:    LEFT_CURLY expr COLON expr RIGHT_CURLY
                ;

block:  LEFT_CURLY{scope++;} stmts RIGHT_CURLY{hide_scope(scope); scope--;}
        ;


funcdef:    FUNCTION functioname LEFT_PARENTHESIS{scope++;} idlist RIGHT_PARENTHESIS{scope--;} block
            ;

functioname: ID {
                        if(is_library_function(yylval.str_val)){
                                yyerror("Trying to shadow libfunction\n");
                        }
                        else{
                                tmp = lookup_scope(yylval.str_val,scope);
                                if(tmp != NULL){
                                        yyerror("Symbol name already exists");
                                }
                                else{
                                        insert(yylval.str_val, USERFUNCTION, yylineno, scope);
                                }
                        }
                }
             |  {
                        sprintf(buf,"$%d",anonymous_function_counter);
                        anonymous_function_counter++; 
                        insert(buf,USERFUNCTION,yylineno,scope);
                        
                }
             ;


const:  CONST_REAL | CONST_INT | STRING | NIL | TRUE | FALSE
        ;

idlist: ID                 {
                                        if(is_library_function(yylval.str_val))
                                                yyerror("Trying to shadow libfunction");
                                        else{
                                                tmp = lookup_scope(yylval.str_val,scope);
                                                if(tmp != NULL)
                                                        yyerror("Multiple formal args detected");
                                                else{
                                                        insert(yylval.str_val,FORMAL,yylineno,scope);
                                                }
                                        }     
                                }
        | ID {
                                        if(is_library_function(yylval.str_val))
                                                yyerror("Trying to shadow libfunction");
                                        else{
                                                tmp = lookup_scope(yylval.str_val,scope);
                                                if(tmp != NULL)
                                                        yyerror("Multiple formal args detected");
                                                else{
                                                        insert(yylval.str_val,FORMAL,yylineno,scope);
                                                }
                                        }     
                                } COMMA idlist
        |
        ;


ifstmt: IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt 
        | IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt ELSE stmt
        ;


whilestmt:  WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt
            ;

forstmt:    FOR LEFT_PARENTHESIS elist SEMICOLON expr SEMICOLON elist RIGHT_PARENTHESIS stmt
            ;


returnstmt: RETURN returnstmts  {}
        ;


returnstmts: expr SEMICOLON {}
             | SEMICOLON
             ;


//ignore: BLOCK_COMMENT | COMMENT_LINE | COMMENT_NESTED ;

//End of grammar rules


%%

int yyerror(char* message){
        fprintf(stderr,"%s at line %d, before token: %s\n",message,yylineno,yytext);
}




int main( int argc, char** argv) {

    if(argc > 1){
        if(!(yyin = fopen(argv[1],"r"))){
                fprintf(stderr,"Cannot open file: %s\n",argv[1]);
                return 1;
        }
    }
    else 
        yyin = stdin;
    insert_library_functions();
    yyparse();
    printf("Finished compiling\n");
    print_symbol_table();
    return 0;
}
