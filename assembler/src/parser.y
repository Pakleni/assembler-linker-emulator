%{
  #include "lexer.h"
  #include <stdio.h>
  int yylex(void);
%}


%output "out/parser.c"
%defines "out/parser.h"


%union {
	int         num;
	char       *directive;
	char       *label;
}

%token <num>   TOKEN_NUM
%token <directive> TOKEN_DIRECTIVE
%token <label> TOKEN_LABEL

%%

prog
  :
  | instr prog
  ;

instr
  : TOKEN_LABEL
  | TOKEN_NUM
  | TOKEN_DIRECTIVE
  ;

%%

int main(int argc, char* argv[])
{
	yyparse();
}