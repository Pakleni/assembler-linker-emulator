%{
    /* C */
    #include "../out/parser.hpp"
    #include <string>

    using namespace std;

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
".global"           { return TOKEN_GLOBAL; }
".extern"           { return TOKEN_EXTERN; }
".section"          { return TOKEN_SECTION; }
".word"             { return TOKEN_WORD; }
".skip"             { return TOKEN_SKIP; }
".equ"              { return TOKEN_EQU; }
".end"              { return TOKEN_END; }
{IDENT}             {
                        yylval.ident = strdup(yytext);
                        return TOKEN_IDENT;
                    }
{BLANK}             {  }
.                   {
                        return yyerror("syntax error");;
                    }
%%

int yywrap() { return 0; }

int yyerror(const char* msg)
{
    fprintf(stderr, "lexer error: %s\n", msg);
    return 0;
}