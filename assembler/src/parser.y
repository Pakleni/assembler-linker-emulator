%{
  #include <cstdio>
  #include "../inc/assembler.hpp"

  using namespace std;

  int yylex(void);
  void yyerror(const char*);
%}


%output   "out/parser.cpp"
%defines  "out/parser.hpp"


%union {
	int             num;
  char *          directive;
	char *          label;
	char *          ident;
  struct IdentList *identList;
}

%token              TOKEN_COMMA
%token              TOKEN_COLON
%token              TOKEN_GLOBAL
%token              TOKEN_EXTERN
%token              TOKEN_SECTION
%token              TOKEN_WORD
%token              TOKEN_SKIP
%token              TOKEN_EQU
%token              TOKEN_END
%token<num>         TOKEN_NUM
%token<ident>       TOKEN_IDENT

%type <identList> identList;

%%

prog
  :
  | instr prog
  ;

instr
  : TOKEN_IDENT TOKEN_COLON
  { Assembler::getInstance().parseLabel($1); }
  | directive
  ;

directive
  : TOKEN_GLOBAL identList
  { Assembler::getInstance().parseGlobal($2); }
  | TOKEN_EXTERN identList
  { Assembler::getInstance().parseExtern($2); }
  | TOKEN_SECTION TOKEN_IDENT
  { Assembler::getInstance().parseSection(string($2)); }
  | TOKEN_WORD identList
  { Assembler::getInstance().parseWord($2); }
  | TOKEN_WORD TOKEN_NUM
  { Assembler::getInstance().parseWord($2); }
  | TOKEN_SKIP TOKEN_NUM
  { Assembler::getInstance().parseSkip($2); }
  | TOKEN_EQU TOKEN_IDENT TOKEN_COMMA TOKEN_NUM
  { Assembler::getInstance().parseEqu(string($2), $4); }
  | TOKEN_END
  { Assembler::getInstance().parseEnd(); YYACCEPT; }
  ;

identList
  : TOKEN_IDENT
  { $$ = new IdentList(string($1)); }
  | TOKEN_IDENT TOKEN_COMMA identList
  { $$ = new IdentList(string($1), $3); }
  ;
%%