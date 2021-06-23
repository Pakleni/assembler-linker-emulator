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
	char *          label;
	char *          ident;
	char *          reg;
  struct IdentList *identList;
}

%token              TOKEN_COMMA
%token              TOKEN_COLON
%token              TOKEN_DOLLAR
%token              TOKEN_PERCENT
%token              TOKEN_OBRACKET
%token              TOKEN_CBRACKET
%token              TOKEN_PLUS
%token              TOKEN_TIMES
%token              I_HALT
%token              I_INT
%token              I_IRET
%token              I_CALL
%token              I_RET
%token              I_JMP
%token              I_JEQ
%token              I_JNE
%token              I_JGT
%token              I_PUSH
%token              I_POP
%token              I_XCHG
%token              I_ADD
%token              I_SUB
%token              I_MUL
%token              I_DIV
%token              I_CMP
%token              I_NOT
%token              I_AND
%token              I_OR
%token              I_XOR
%token              I_TEST
%token              I_SHL
%token              I_SHR
%token              I_LDR
%token              I_STR
%token              TOKEN_GLOBAL
%token              TOKEN_EXTERN
%token              TOKEN_SECTION
%token              TOKEN_WORD
%token              TOKEN_SKIP
%token              TOKEN_EQU
%token              TOKEN_END
%token<num>         TOKEN_NUM
%token<ident>       TOKEN_IDENT
%token<reg>         REG

%type <identList> identList;

%%

prog
  :
  | instr prog
  ;

instr
  : label
  | directive
  | instruction
  ;

label
  : TOKEN_IDENT TOKEN_COLON
  { Assembler::getInstance().parseLabel($1); }
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

instruction
  : I_HALT
  | I_IRET
  | I_RET
  | jmp jmpop
  | singlereg REG
  | tworeg REG TOKEN_COMMA REG
  | regop REG TOKEN_COMMA dataop
  ;

jmp
  : I_CALL
  | I_JMP
  | I_JEQ
  | I_JNE
  | I_JGT
  ;

singlereg
  : I_INT
  | I_PUSH
  | I_POP
  | I_NOT

tworeg
  : I_XCHG
  | I_ADD
  | I_SUB
  | I_MUL
  | I_DIV
  | I_CMP
  | I_AND
  | I_OR
  | I_XOR
  | I_TEST
  | I_SHL
  | I_SHR
  ;

regop
  : I_LDR
  | I_STR
  ;

dataop
  : TOKEN_DOLLAR TOKEN_NUM
  | TOKEN_DOLLAR TOKEN_IDENT
  | TOKEN_NUM
  | TOKEN_IDENT
  | TOKEN_PERCENT TOKEN_IDENT
  | REG
  | TOKEN_OBRACKET REG TOKEN_CBRACKET
  | TOKEN_OBRACKET REG TOKEN_PLUS TOKEN_NUM TOKEN_CBRACKET
  | TOKEN_OBRACKET REG TOKEN_PLUS TOKEN_IDENT TOKEN_CBRACKET
  ;

jmpop
  : TOKEN_NUM
  | TOKEN_IDENT
  | TOKEN_PERCENT TOKEN_IDENT
  | TOKEN_TIMES TOKEN_NUM
  | TOKEN_TIMES TOKEN_IDENT
  | TOKEN_TIMES REG
  | TOKEN_TIMES TOKEN_OBRACKET REG TOKEN_CBRACKET
  | TOKEN_TIMES TOKEN_OBRACKET REG TOKEN_PLUS TOKEN_NUM TOKEN_CBRACKET
  | TOKEN_TIMES TOKEN_OBRACKET REG TOKEN_PLUS TOKEN_IDENT TOKEN_CBRACKET
  ;
%%