
%{
        #include<stdio.h>
        #include<stdlib.h>
        #include "sym_table.h"
        #define YY_DECL int alpha_yylex(void* ylval)
       
        int yyerror(char* message);
        int yylex(void);
        extern int yylineno;
        extern char* yytext;
        extern FILE* yyin;

        int scope = 0;
        int anonymous_function_counter = 0;
        char* buf[32];
        


%}

%start program

%union{
        int int_val;
        double double_val;
        char *str_val;
        symrec *node;

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
%token <str_val> CONST_STRING
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


program:   stmts   {}
            ;


stmts : stmt stmtreduce
        ;


stmtreduce: stmt stmtreduce
            |
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
        | ignore
       // | error {cerr<<"Error: invalid stmt";}
        ;

expr:   assignexpr
        | expr op expr %prec EXPRESSION_READ_NEXT_CHARACTER
        | term
       // | error SEMICOLON {cerr<<"Error: expression expected!";}
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
        | PLUS_PLUS lvalue //{incrementing_lvalue_action($2.is_declared_local, $2.token_val);}
        | MINUS_MINUS lvalue //{decrementing_lvalue_action($2.is_declared_local, $2.token_val);}
        | lvalue PLUS_PLUS //{incrementing_lvalue_action($1.is_declared_local, $1.token_val);}
        | lvalue MINUS_MINUS //{decrementing_lvalue_action($1.is_declared_local, $1.token_val);}
        | primary
        ;

assignexpr: lvalue ASSIGN expr {assignexpr_action($1.is_declared_local, $1.token_val);}
            ;

primary:    lvalue {lvalue_action($1.is_declared_local, $1.token_val);}
            | call
            | objectdef
            | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS
            | const
            ;

lvalue: ID                            {$$.token_val = &tokenList.back(); $$.is_declared_local = false;} 
        | LOCAL ID              {$$.token_val = &tokenList.back(); $$.is_declared_local = true;}
        | DOUBLE_COLON ID   {$$.token_val = symbol_table.search_global_id(tokenList.back().getLexem()); $$.is_declared_local = false;}
        | member                            {$$.token_val = nullptr; $$.is_declared_local = false;}
        ;

member: lvalue DOT ID                   {}
        | lvalue LEFT_SUBSCRIPT expr RIGHT_SUBSCRIPT    {}
        | call DOT ID                   {}
        | call LEFT_SUBSCRIPT expr RIGHT_SUBSCRIPT      {}
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

elist:  LEFT_SUBSCRIPT expressionlist RIGHT_SUBSCRIPT
        ;

expressionlist: expr 
                | expr COMMA expressionlist
                |expressionlisterror
                ;

expressionlisterror: expressionlist COMMA RIGHT_PARENTHESIS {cerr<<"Error: invalid RIGHT_PARENTHESIS";} 
                   | expressionlist COMMA RIGHT_SUBSCRIPT {cerr<<"Error: invalid RIGHT_SUBSCRIPT";}
                   | expressionlist COMMA LEFT_SUBSCRIPT {cerr<<"Error: invalid operator and RIGHT_SUBSCRIPT";}
                   | expressionlist op {cerr<<"Error: invalid operator ";} 
                   | expressionlist COMMA RIGHT_BRACKET {cerr<<"Error: invalid operator and RIGHT_BRACKET";}
                   |expressionlist COMMA LEFT_BRACKET {cerr<<"Error: invalid LEFT_BRACKET";}

objectdef:  LEFT_SUBSCRIPT objectdeflist RIGHT_SUBSCRIPT   
            ;

objectdeflist: elist | indexed 
;

indexed:    LEFT_SUBSCRIPT indexedelemlist RIGHT_SUBSCRIPT 
            ;

indexedelemlist: indexedelem
                 | indexedelem COMMA indexedelemlist
                 ;

indexedelem:    LEFT_BRACKET expr COLON expr RIGHT_BRACKET
                ;

indexedelemlisterror:  indexedelemlist LEFT_BRACKET expr expr RIGHT_BRACKET{cerr<<"Error: missing COLON";} 
                   | indexedelemlist LEFT_BRACKET  COLON expr RIGHT_BRACKET {cerr<<"Error: left expression missing";}
                   | indexedelemlist LEFT_BRACKET expr COLON expr stmt {cerr<<"Error: RIGHT_BRACKET missing";}
;


block:  blockopenscope stmt blockclosescope
        |blockerror
        ;

blockopenscope: LEFT_BRACKET { if(increment_scope) symbol_table.initialise_new_scope(); }
                ;

blockclosescope: RIGHT_BRACKET { increment_scope=true; symbol_table.conclude_scope(); }
                 ;

blockerror: blockopenscope stmt {cerr<<"Error: block has not closed! RIGHT_BRACKET expected!";}
            |blockclosescope {cerr<<"Error: You have more RIGHT_BRACKET than LEFT_BRACKET!";}
            ;


funcdef:    FUNCTION functioname funcdefopenscope idlist RIGHT_PARENTHESIS block
            |functionerror
            ;

functioname: ID {symbol_table.define_function(&tokenList.back());}
             |  {
                sprintf(buf,"$%d",anonymous_function_counter);
                anonymous_function_counter++; 
                insert(buf,FUNCTION,yylineno,scope);
                }
             ;

funcdefopenscope: LEFT_PARENTHESIS { increment_scope=false; symbol_table.initialise_new_scope(); }
                  ;

functionerror:FUNCTION functioname idlist RIGHT_PARENTHESIS block {cerr<<"Error: LEFT_PARENTHESIS missing!";}
              | FUNCTION functioname LEFT_PARENTHESIS idlist block {cerr<<"Error: RIGHT_PARENTHESIS missing!"<<endl;}
              | FUNCTION functioname LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS {cerr<<"Error: black is missing!"<<endl;}
              | FUNCTION functioname LEFT_PARENTHESIS op RIGHT_PARENTHESIS {cerr<<"Error: false expression in parenthesis and missing block of stmts";}
              | FUNCTION functioname LEFT_PARENTHESIS op RIGHT_PARENTHESIS block {cerr<<"Error: false expression in parenthesis";}
;

const:  CONST_REAL | CONST_INT | CONST_STRING | NIL | TRUE | FALSE
        ;

idlist: ID                 {symbol_table.search_formal_identifier(&tokenList.back());}
        | ID COMMA idlist  {symbol_table.search_formal_identifier(&tokenList.back());}
        | idlisterror
        ;

idlisterror: ID COMMA LEFT_PARENTHESIS {cerr<<"Error: ID expected";}
            ;

ifstmt: IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt 
        | IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt ELSE stmt
        | IF specialparenthesiserror 
        ;


whilestmt:  WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS stmt
            | WHILE specialparenthesiserror
            ;

forstmt:    FOR LEFT_PARENTHESIS elist SEMICOLON expr SEMICOLON elist RIGHT_PARENTHESIS stmt
            | FOR parenthesiserror 
            | forstmterror 
            ;

forstmterror:   FOR LEFT_PARENTHESIS elist expr SEMICOLON elist RIGHT_PARENTHESIS stmt {cerr<<"Error: missing the first SEMICOLON";}
                |FOR LEFT_PARENTHESIS elist expr SEMICOLON elist RIGHT_PARENTHESIS {cerr<<"Error: missing the first SEMICOLON";}
                |FOR elist SEMICOLON expr elist RIGHT_PARENTHESIS stmt {cerr<<"Error: missing the second SEMICOLON";}
                |FOR RIGHT_PARENTHESIS elist LEFT_PARENTHESIS {cerr<<"Error: missing the SEMICOLONs";}
                |FOR RIGHT_PARENTHESIS LEFT_PARENTHESIS {cerr<< "Error: missing the SEMICOLONs";}
;

parenthesiserror: RIGHT_PARENTHESIS {cerr<<"Error: more RIGHT_PARENTHESISs than LEFT_PARENTHESIS";}
                 |LEFT_PARENTHESIS expr RIGHT_BRACKET {cerr<<"Error: invalid RIGHT_BRACKET";}
                 |LEFT_PARENTHESIS expr LEFT_BRACKET {cerr<<"Error: invalid LEFT_BRACKET";}
;
specialparenthesiserror: LEFT_PARENTHESIS stmt {cerr<<"Error: invalid position for stmt";}
                        | parenthesiserror
;

returnstmt: RETURN returnstmts  {}

returnstmts: expr SEMICOLON {}
             | SEMICOLON
             | op SEMICOLON {cerr<<"Error: missing the SEMICOLONs";}
             ;


ignore: BLOCK_COMMENT | COMMENT_LINE | COMMENT_NESTED ;

//End of grammar rules


%%

int yyerror(char* message){
        fprintf(stderr,"%s at line &d, before token: %s\n",message,yylineno,yytext);
        return 1;
}

/*
int yylex(){
    return alpha_yylex(nullptr);
}
*/

//int yywrap(){return 1;}

int main( int argc, char** argv) {

    if(argc > 1){
        if(!(yyin = fopen(argv[1],"r"))){
                fprintf(stderr,"Cannot open file: %s\n",argv[1]);
                return 1;
        }
    }
    else yyin = stdin;
    insert_library_functions();
    yyparse();
    print_symbol_table();
    return 0;


}
