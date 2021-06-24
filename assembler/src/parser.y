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
	int             reg;
  struct IdentList *identList;
  class Operand *     op;
  int             instr;
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

%type <op>     jmpop
%type <op>     dataop



%type <instr>     noadr
%type <instr>     jmp
%type <instr>     singlereg
%type <instr>     tworeg
%type <instr>     regop
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
  : noadr
  { Assembler::getInstance().noaddr($1); }
  | jmp jmpop
  { Assembler::getInstance().jmp($1, $2); }
  | singlereg REG
  { Assembler::getInstance().tworeg($1, $2, 0 ); }
  | tworeg REG TOKEN_COMMA REG
  { Assembler::getInstance().tworeg($1, $2, $4); }
  | regop REG TOKEN_COMMA dataop
  { Assembler::getInstance().regop($1, $2, $4); }
  | I_PUSH REG
  { Assembler::getInstance().push($2); }
  | I_POP REG
  { Assembler::getInstance().pop($2); }
  ;

noadr
  : I_HALT
  { $$ = 0b00000000; }
  | I_IRET
  { $$ = 0b00100000; }
  | I_RET
  { $$ = 0b01000000; }
  ;
jmp
  : I_CALL
  { $$ = 0b00110000; }
  | I_JMP
  { $$ = 0b01010000; }
  | I_JEQ
  { $$ = 0b01010001; }
  | I_JNE
  { $$ = 0b01010010; }
  | I_JGT
  { $$ = 0b01010011; }
  ;

singlereg
  : I_INT
  { $$ = 0b00010000; }
  | I_NOT
  { $$ = 0b10000000; }
  ;

tworeg
  : I_XCHG
  { $$ = 0b01100000; }
  | I_ADD
  { $$ = 0b01110000; }
  | I_SUB
  { $$ = 0b01110001; }
  | I_MUL
  { $$ = 0b01110010; }
  | I_DIV
  { $$ = 0b01110011; }
  | I_CMP
  { $$ = 0b01110100; }
  | I_AND
  { $$ = 0b10000001; }
  | I_OR
  { $$ = 0b10000010; }
  | I_XOR
  { $$ = 0b10000011; }
  | I_TEST
  { $$ = 0b10000100; }
  | I_SHL
  { $$ = 0b10010000; }
  | I_SHR
  { $$ = 0b10010001; }
  ;

regop
  : I_LDR
  { $$ = 0b10100000; }
  | I_STR
  { $$ = 0b10110000; }
  ;

dataop
  : TOKEN_DOLLAR TOKEN_NUM
  { $$ = new LitOp($2, LitOp::Mode::DATA_DOLLAR); }
  | TOKEN_DOLLAR TOKEN_IDENT
  { $$ = new SymOp($2, SymOp::Mode::DATA_DOLLAR); }
  | TOKEN_NUM
  { $$ = new LitOp($1, LitOp::Mode::DATA_NULL); }
  | TOKEN_IDENT
  { $$ = new SymOp($1, SymOp::Mode::DATA_NULL); }
  | TOKEN_PERCENT TOKEN_IDENT
  { $$ = new SymOp($2, SymOp::Mode::DATA_PERCENT); }
  | REG
  { $$ = new RegOp($1, RegOp::Mode::NONE); }
  | TOKEN_OBRACKET REG TOKEN_CBRACKET
  { $$ = new RegOp($2, RegOp::Mode::BRACKET); }
  | TOKEN_OBRACKET REG TOKEN_PLUS TOKEN_NUM TOKEN_CBRACKET
  { $$ = new RegLitOp($2, $4); }
  | TOKEN_OBRACKET REG TOKEN_PLUS TOKEN_IDENT TOKEN_CBRACKET
  { $$ = new RegSymOp($2, $4); }
  ;

jmpop
  : TOKEN_NUM
  { $$ = new LitOp($1, LitOp::Mode::JMP_NULL); }
  | TOKEN_IDENT
  { $$ = new SymOp($1, SymOp::Mode::JMP_NULL); }
  | TOKEN_PERCENT TOKEN_IDENT
  { $$ = new SymOp($2, SymOp::Mode::JMP_PERCENT); }
  | TOKEN_TIMES TOKEN_NUM
  { $$ = new LitOp($2, LitOp::Mode::JMP_TIMES); }
  | TOKEN_TIMES TOKEN_IDENT
  { $$ = new SymOp($2, SymOp::Mode::JMP_TIMES); }
  | TOKEN_TIMES REG
  { $$ = new RegOp($2, RegOp::Mode::NONE); }
  | TOKEN_TIMES TOKEN_OBRACKET REG TOKEN_CBRACKET
  { $$ = new RegOp($3, RegOp::Mode::BRACKET); }
  | TOKEN_TIMES TOKEN_OBRACKET REG TOKEN_PLUS TOKEN_NUM TOKEN_CBRACKET
  { $$ = new RegLitOp($3, $5); }
  | TOKEN_TIMES TOKEN_OBRACKET REG TOKEN_PLUS TOKEN_IDENT TOKEN_CBRACKET
  { $$ = new RegSymOp($3, $5); }
  ;
%%