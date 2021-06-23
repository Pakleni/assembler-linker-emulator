%{
    /* C */
    #include "../out/parser.hpp"
    #include <string>

    using namespace std;

    int lexerror (string msg);
    int yyerror(const char*);
%}

%option outfile="out/lexer.cpp" header-file="out/lexer.hpp"

BLANK               [ \r\n\t]*
COMMENT             "#"[^\n\r]*
HEX                 0[Xx][0-9A-Fa-f]+
DEC                 [0-9]+
IDENT               [A-Za-z_][A-Za-z0-9_]*

/* rules */
%%
{COMMENT}           {  }
{HEX}               {
                        sscanf(yytext, "%x", &yylval.num);
                        return TOKEN_NUM;
                    }
{DEC}               {
                        sscanf(yytext, "%d", &yylval.num);
                        return TOKEN_NUM;
                    }
","                 { return TOKEN_COMMA; }
":"                 { return TOKEN_COLON; }
"$"                 { return TOKEN_DOLLAR; }
"%"                 { return TOKEN_PERCENT; }
"["                 { return TOKEN_OBRACKET; }
"]"                 { return TOKEN_CBRACKET; }
"+"                 { return TOKEN_PLUS; }
"*"                 { return TOKEN_TIMES; }
".global"           { return TOKEN_GLOBAL; }
".extern"           { return TOKEN_EXTERN; }
".section"          { return TOKEN_SECTION; }
".word"             { return TOKEN_WORD; }
".skip"             { return TOKEN_SKIP; }
".equ"              { return TOKEN_EQU; }
".end"              { return TOKEN_END; }
"."{IDENT}          {
                        lexerror(string("no such directive: ") + yytext);
                    }
"halt"              { return I_HALT; }
"int"               { return I_INT; }
"iret"              { return I_IRET; }
"call"              { return I_CALL; }
"ret"               { return I_RET; }
"jmp"               { return I_JMP; }
"jeq"               { return I_JEQ; }
"jne"               { return I_JNE; }
"jgt"               { return I_JGT; }
"push"              { return I_PUSH; }
"pop"               { return I_POP; }
"xchg"              { return I_XCHG; }
"add"               { return I_ADD; }
"sub"               { return I_SUB; }
"mul"               { return I_MUL; }
"div"               { return I_DIV; }
"cmp"               { return I_CMP; }
"not"               { return I_NOT; }
"and"               { return I_AND; }
"or"                { return I_OR; }
"xor"               { return I_XOR; }
"test"              { return I_TEST; }
"shl"               { return I_SHL; }
"shr"               { return I_SHR; }
"ldr"               { return I_LDR; }
"str"               { return I_STR; }
"r"[0-7]            { yylval.reg = strdup(yytext); return REG; }
"sp"                { yylval.reg = strdup(yytext); return REG; }
"pc"                { yylval.reg = strdup(yytext); return REG; }
"psw"               { yylval.reg = strdup(yytext); return REG; }
{IDENT}             {
                        yylval.ident = strdup(yytext);
                        return TOKEN_IDENT;
                    }
{BLANK}             {  }
.                   {
                        lexerror(string("syntax error: ") + yytext);
                    }
%%

int yywrap() { return 0; }

int lexerror (string msg) {
    yyerror(msg.c_str());
    exit(1);
}
int yyerror(const char * msg)
{
    fprintf(stderr, "lexer error: %s\n", msg);
    return 0;
}