%{
  #include <cstdio>
  int yylex(void);
  void yyerror(const char*);
%}


%output   "out/parser.cpp"
%defines  "out/parser.hpp"


%union {
	int         num;
	char       *directive;
	char       *label;
}

%token <num>        TOKEN_NUM
%token <directive>  TOKEN_DIRECTIVE
%token <label>      TOKEN_LABEL

%%

prog
  :
  | instr prog
  ;

instr
  : TOKEN_LABEL
    { printf("LABEL\n"); }
  | TOKEN_NUM
    { printf("NUMBER: %d\n", yylval.num); }
  | TOKEN_DIRECTIVE
    { printf("DIRECTIVE\n"); }
  ;

%%