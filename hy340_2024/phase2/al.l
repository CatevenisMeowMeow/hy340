%{
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parser.h"


typedef struct alpha_token_t{
	int numlineStart;
	int numlineEnd;
	int numToken;
	char* content;
	char* type;
    char* description;
	struct alpha_token_t* next;
}Token;

/*global*/
Token *head = NULL;
int token_counter = 0;

int comment_token_counter = 0;
int nested_comment_start_line = 0; 
int nested_comment_end_line = 0;




void insert_token(int numlineStart,int numlineEnd,int numToken,char* content,char* description,char* type){
/*make the new token */
	Token *newToken;
   assert((newToken = (Token*)malloc(sizeof(Token)))!=NULL);
   newToken->numlineStart = numlineStart;
   newToken->numlineEnd = numlineEnd;
   newToken->numToken= numToken;

   assert((newToken -> content = (char*)malloc(strlen(content)))!=NULL);
   assert((newToken -> description = (char*)malloc(strlen(description)))!=NULL);
   assert((newToken -> type = (char*)malloc(strlen(type)))!=NULL);

   assert(strcpy(newToken->content,content));
   assert(strcpy(newToken->type,type));
   assert(strcpy(newToken->description,description));


/*insert to token list*/
    if(head == NULL){
        head = newToken;
        newToken->next = NULL;
    }
    else{ /* put the new token at the end of the list */
        Token* tmp = head;
        while(tmp->next != NULL){
            tmp = tmp->next;
        }
        tmp->next = newToken;
        newToken->next = NULL;
    }
}

void print_tokens(){
	Token *tmp = head;
	while(tmp!=NULL){
		if(strcmp(tmp->type, "COMMENT") == 0) {
			printf("%d: #%d  \"%d - %d\" %s  %s\n", tmp->numlineStart, tmp->numToken, tmp->numlineStart, tmp->numlineEnd, tmp->type, tmp->description);
		} 
		else {
			printf("%d: #%d  \"%s\" %s  %s\n",tmp->numlineStart,tmp->numToken,tmp->content,tmp->type,tmp->description);
		}
		tmp = tmp->next;
	}
}



%}

%option noyywrap
%option yylineno


IF                      "if"
ELSE                    "else"
WHILE                   "while"
FOR                     "for"
FUNCTION                "function"
RETURN                  "return"
BREAK                   "break"
CONTINUE                "continue"
AND                     "and"
NOT                     "not"
OR                      "or"
LOCAL                   "local"
TRUE                    "true"
FALSE                   "false"
NIL                     "nil"

EQUAL                   "=="
ASSIGN                  "="
PLUS_PLUS               "++"
PLUS                    "+"
MINUS_MINUS             "--"
MINUS                   "-"
MODULO                  "%"
NOT_EQUAL               "!="
GREATER_EQUAL           ">="
GREATER            ">"
LESS_EQUAL              "<="
LESS              "<"
MULTIPLICATION                   "*"
DIVISION                     "/"

CONST_INT               [0-9]+
CONST_REAL              [0-9]+\.[0-9]+

LEFT_CURLY              "{"
RIGHT_CURLY             "}"
LEFT_SQUARE             "["
RIGHT_SQUARE            "]"
LEFT_PARENTHESIS        "("
RIGHT_PARENTHESIS       ")"
SEMICOLON               ";"
COMMA                   ","
DOUBLE_COLON            "::"
COLON                   ":"
DOUBLE_DOT              ".."
DOT                     "."

STRING                  \"([^"\\]|\\.)*\"
ERROR_STRING            \"[^\n"]*

ID                      [a-zA-Z][a-zA-Z_0-9]*
SPACE                   [\r\n\t\v ]*

COMMENT_LINE                "//".*
COMMENT_NESTED           "/*"

%x COMMENT

%%

{IF}                    {insert_token(yylineno, yylineno, ++token_counter, "if", "IF","KEYWORD"); return IF;}
{ELSE}                  {insert_token(yylineno, yylineno, ++token_counter, "else","ELSE","KEYWORD"); return ELSE;}
{WHILE}                 {insert_token(yylineno, yylineno, ++token_counter, "while","WHILE","KEYWORD"); return WHILE;}
{FOR}                   {insert_token(yylineno, yylineno, ++token_counter, "for","FOR","KEYWORD"); return FOR;}
{FUNCTION}              {insert_token(yylineno, yylineno, ++token_counter, "function","FUNCTION" ,"KEYWORD"); return FUNCTION;}
{RETURN}                {insert_token(yylineno, yylineno, ++token_counter, "return","RETURN", "KEYWORD"); return RETURN;}
{BREAK}                 {insert_token(yylineno, yylineno, ++token_counter, "break", "BREAK","KEYWORD"); return BREAK;}
{CONTINUE}              {insert_token(yylineno, yylineno, ++token_counter, "continue","CONTINUE", "KEYWORD"); return CONTINUE;}
{AND}                   {insert_token(yylineno, yylineno, ++token_counter, "and","AND" ,"KEYWORD"); return AND;}
{NOT}                   {insert_token(yylineno, yylineno, ++token_counter, "not", "NOT","KEYWORD"); return NOT;}
{OR}                    {insert_token(yylineno, yylineno, ++token_counter, "or","OR" ,"KEYWORD"); return OR;}
{LOCAL}                 {insert_token(yylineno, yylineno, ++token_counter, "local","LOCAL" ,"KEYWORD"); return LOCAL;}
{TRUE}                  {insert_token(yylineno, yylineno, ++token_counter, "true", "TRUE","KEYWORD"); return TRUE;}
{FALSE}                 {insert_token(yylineno, yylineno, ++token_counter, "false","FALSE" ,"KEYWORD"); return FALSE;}
{NIL}                   {insert_token(yylineno, yylineno, ++token_counter, "nil", "NIL","KEYWORD"); return NIL;}

{EQUAL}					{insert_token(yylineno, yylineno, ++token_counter, "==","EQUAL" ,"OPERATOR"); return EQUAL;}
{ASSIGN}				{insert_token(yylineno, yylineno, ++token_counter, "=", "ASSIGN","OPERATOR"); return ASSIGN;}
{PLUS_PLUS}				{insert_token(yylineno, yylineno, ++token_counter, "++","PLUS_PLUS" ,"OPERATOR"); return PLUS_PLUS;}
{PLUS}					{insert_token(yylineno, yylineno, ++token_counter, "+","PLUS" ,"OPERATOR"); return PLUS;}
{MINUS_MINUS}			{insert_token(yylineno, yylineno, ++token_counter, "--", "MINUS_MINUS","OPERATOR"); return MINUS_MINUS;}
{MINUS}					{insert_token(yylineno, yylineno, ++token_counter, "-","MINUS" ,"OPERATOR"); return MINUS;}
{MODULO}				{insert_token(yylineno, yylineno, ++token_counter, "%", "MODULO","OPERATOR"); return MODULO;}
{NOT_EQUAL}				{insert_token(yylineno, yylineno, ++token_counter, "!=", "NOT_EQUAL","OPERATOR"); return NOT_EQUAL;}
{GREATER_EQUAL}	        {insert_token(yylineno, yylineno, ++token_counter, ">=","GREATER_EQUAL" ,"OPERATOR"); return GREATER_EQUAL;}
{GREATER}			    {insert_token(yylineno, yylineno, ++token_counter, ">", "GREATER","OPERATOR"); return GREATER;}
{LESS_EQUAL}		    {insert_token(yylineno, yylineno, ++token_counter, "<=", "LESS_EQUAL","OPERATOR"); return LESS_EQUAL;}
{LESS}				    {insert_token(yylineno, yylineno, ++token_counter, "<", "LESS","OPERATOR"); return LESS;}
{MULTIPLICATION}		{insert_token(yylineno, yylineno, ++token_counter, "*","MULTIPLICATION" ,"OPERATOR"); return MULTIPLICATION;}
{DIVISION}				{insert_token(yylineno, yylineno, ++token_counter, "/", "DIVISION","OPERATOR"); return DIVISION;}

{CONST_INT}				{
							insert_token(yylineno, yylineno, ++token_counter, yytext, "","CONST_INT");
							yylval.int_val = atoi(yytext);
							return CONST_INT;
						}
{CONST_REAL}			{
							insert_token(yylineno, yylineno, ++token_counter, yytext, "","CONST_REAL");
							yylval.double_val = atof(yytext);
							return CONST_REAL;
						}


{LEFT_CURLY}			{insert_token(yylineno, yylineno, ++token_counter, "{", "LEFT_CURLY", "PUNCTUATION"); return LEFT_CURLY;}
{RIGHT_CURLY}			{insert_token(yylineno, yylineno, ++token_counter, "}", "RIGHT_CURLY", "PUNCTUATION"); return RIGHT_CURLY;}
{LEFT_SQUARE}			{insert_token(yylineno, yylineno, ++token_counter, "[", "LEFT_SQUARE", "PUNCTUATION"); return LEFT_SQUARE;}
{RIGHT_SQUARE}			{insert_token(yylineno, yylineno, ++token_counter, "]", "RIGHT_SQUARE", "PUNCTUATION"); return RIGHT_SQUARE;}
{LEFT_PARENTHESIS}		{insert_token(yylineno, yylineno, ++token_counter, "(", "LEFT_PARENTHESIS", "PUNCTUATION"); return LEFT_PARENTHESIS;}
{RIGHT_PARENTHESIS}		{insert_token(yylineno, yylineno, ++token_counter, ")", "RIGHT_PARENTHESIS", "PUNCTUATION"); return RIGHT_PARENTHESIS;}
{SEMICOLON}				{insert_token(yylineno, yylineno, ++token_counter, ";", "SEMICOLON", "PUNCTUATION"); return SEMICOLON;}
{COMMA}					{insert_token(yylineno, yylineno, ++token_counter, ",", "COMMA", "PUNCTUATION"); return COMMA;}
{DOUBLE_COLON}			{insert_token(yylineno, yylineno, ++token_counter, "::", "DOUBLE_COLON", "PUNCTUATION"); return DOUBLE_COLON;}
{COLON}					{insert_token(yylineno, yylineno, ++token_counter, ":", "COLON", "PUNCTUATION"); return COLON;}
{DOUBLE_DOT}			{insert_token(yylineno, yylineno, ++token_counter, "..", "DOUBLE_DOT","PUNCTUATION"); return DOUBLE_DOT;}
{DOT}					{insert_token(yylineno, yylineno, ++token_counter, ".", "DOT", "PUNCTUATION"); return DOT;}

{STRING}				{
							insert_token(yylineno, yylineno, ++token_counter, yytext,"","STRING");
							yylval.str_val = strdup(yytext);
							return STRING;	
						}
{ERROR_STRING}			{
							fprintf(stderr, "String beggining at line %d does not close\n", yylineno);
							return 1;
						}
{ID}					{
							insert_token(yylineno, yylineno, ++token_counter, yytext,"","ID");
							yylval.str_val = strdup(yytext);
							return ID;
						}
{SPACE}					{}
{COMMENT_LINE}			{insert_token(yylineno, yylineno, ++token_counter,"//","", "COMMENT LINE_COMMENT");}

.						{
							printf("Undefined input %s in line %d\n", yytext, yylineno);
							return 0;
						}

"/*"					{
							if(comment_token_counter == 0){
								nested_comment_start_line = yylineno;
							}
							comment_token_counter++;
							BEGIN(COMMENT);
						}

<COMMENT>"*/" 			{
							comment_token_counter--;
							if(comment_token_counter == 0){
								nested_comment_end_line = yylineno;  
    							insert_token(nested_comment_start_line, nested_comment_end_line, ++token_counter, "comment content", "BLOCK_COMMENT", "COMMENT");
    							BEGIN(INITIAL);
							}
						}

<COMMENT>"/*" 			{
							comment_token_counter++;
						}

<COMMENT>.|\n 			{}

<<EOF>> return EOF;

%%
