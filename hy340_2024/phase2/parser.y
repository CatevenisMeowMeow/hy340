
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
%token ASSIGNMENT
%token ADDITION
%token SUBTRACTION
%token MULTIPLICATION
%token DIVISION
%token MODULO
%token EQUAL
%token NOT_EQUAL
%token INCREMENT
%token DECREMENT
%token GREATER
%token LESSER
%token GREATER_EQUAL
%token LESSER_EQUAL
%token LEFT_CURLY // '{'
%token RIGHT_CURLY // '}'
%token LEFT_SQUARE// '['
%token RIGHT_SQUARE // ']' 
%token LEFT_PARENTHESIS
%token RIGHT_PARENTHESIS
%token SEMICOLON
%token COMMA
%token MEMBER_INITIALISER_LIST  // :
%token SCOPE_RESOLUTION         // ::
%token DOT      // .
%token DOUBLE_DOT         // ..
%token <str_val> ID
%token <int_val> INTCONST
%token <double_val> REALCONST
%token <str_val> STRING
%token NESTED_COMMENT
%token BLOCK_COMMENT
%token LINE_COMMENT

//Priority rules

%left REALCONST INTCONST STRING NIL TRUE FALSE
%left ASSIGNMENT
%left OR
%left AND
%nonassoc EQUAL NOT_EQUAL
%nonassoc GREATER GREATER_EQUAL LESSER LESSER_EQUAL
%left ADDITION SUBTRACTION
%left MULTIPLICATION DIVISION MODULO
%left NOT INCREMENT DECREMENT
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

op: ADDITION 
    | SUBTRACTION 
    | MULTIPLICATION
    | DIVISION 
    | MODULO 
    | GREATER 
    | GREATER_EQUAL 
    | LESSER 
    | LESSER_EQUAL 
    | EQUAL 
    | NOT_EQUAL 
    | AND 
    | OR
    ;

term:   LEFT_PARENTHESIS expr RIGHT_PARENTHESIS
        | SUBTRACTION expr %prec UMINUS  
        | NOT expr
        | INCREMENT lvalue //{incrementing_lvalue_action($2.is_declared_local, $2.token_val);}
        | DECREMENT lvalue //{decrementing_lvalue_action($2.is_declared_local, $2.token_val);}
        | lvalue INCREMENT //{incrementing_lvalue_action($1.is_declared_local, $1.token_val);}
        | lvalue DECREMENT //{decrementing_lvalue_action($1.is_declared_local, $1.token_val);}
        | primary
        ;

assignexpr: lvalue ASSIGNMENT expr {assignexpr_action($1.is_declared_local, $1.token_val);}
            ;

primary:    lvalue {lvalue_action($1.is_declared_local, $1.token_val);}
            | call
            | objectdef
            | LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS
            | const
            ;

lvalue: ID                            {$$.token_val = &tokenList.back(); $$.is_declared_local = false;} 
        | LOCAL ID              {$$.token_val = &tokenList.back(); $$.is_declared_local = true;}
        | SCOPE_RESOLUTION ID   {$$.token_val = symbol_table.search_global_id(tokenList.back().getLexem()); $$.is_declared_local = false;}
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

indexedelem:    LEFT_BRACKET expr MEMBER_INITIALISER_LIST expr RIGHT_BRACKET
                ;

indexedelemlisterror:  indexedelemlist LEFT_BRACKET expr expr RIGHT_BRACKET{cerr<<"Error: missing MEMBER_INITIALISER_LIST";} 
                   | indexedelemlist LEFT_BRACKET  MEMBER_INITIALISER_LIST expr RIGHT_BRACKET {cerr<<"Error: left expression missing";}
                   | indexedelemlist LEFT_BRACKET expr MEMBER_INITIALISER_LIST expr stmt {cerr<<"Error: RIGHT_BRACKET missing";}
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
             |  {add_unnamed_function(); symbol_table.define_function(&unnamed_functions.back());} // An meinei xronos, tha ftiaksw nea methodo xwris elegxous.
             ;

funcdefopenscope: LEFT_PARENTHESIS { increment_scope=false; symbol_table.initialise_new_scope(); }
                  ;

functionerror:FUNCTION functioname idlist RIGHT_PARENTHESIS block {cerr<<"Error: LEFT_PARENTHESIS missing!";}
              | FUNCTION functioname LEFT_PARENTHESIS idlist block {cerr<<"Error: RIGHT_PARENTHESIS missing!"<<endl;}
              | FUNCTION functioname LEFT_PARENTHESIS idlist RIGHT_PARENTHESIS {cerr<<"Error: black is missing!"<<endl;}
              | FUNCTION functioname LEFT_PARENTHESIS op RIGHT_PARENTHESIS {cerr<<"Error: false expression in parenthesis and missing block of stmts";}
              | FUNCTION functioname LEFT_PARENTHESIS op RIGHT_PARENTHESIS block {cerr<<"Error: false expression in parenthesis";}
;

const:  REALCONST | INTCONST | STR | NIL | TRUE | FALSE
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


ignore: BLOCK_COMMENT | LINE_COMMENT | NESTED_COMMENT ;

//End of grammar rules


%%

int yyerror(char* message){
        fprintf(stderr,"%s at line &d, before token: %s\n",message,yylineno,yytext);
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
    return 0;


}
